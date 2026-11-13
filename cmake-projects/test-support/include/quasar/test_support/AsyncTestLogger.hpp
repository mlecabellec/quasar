/**
 * @file AsyncTestLogger.hpp
 * @brief Non-blocking asynchronous operational test logger and deterministic PRNG utilities.
 *
 * @details Provides a wait-free bounded SPSC/MPSC-safe ring-buffer logger with a background
 * flush thread (`std::jthread`) so verbose test telemetry (inputs, outputs, timestamps,
 * step durations, branch traces, and throttled loop summaries) never blocks or distorts
 * timing-sensitive or concurrent test execution. Also supplies a fixed-seed `<random>`
 * wrapper for deterministic fuzzing and test data generation.
 *
 * @task TSK-20261113-004
 * @feature FE-0010, FE-0030
 * @constraint CS-0010.21, CS-0010.26, CS-0010.34, CS-0010.37, CS-0040.2, CS-0060.3, CS-0060.5, CS-0060.7, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9, CS-0100.12, CS-0100.20, CS-0100.21
 * @compliance Uses explicit types (no auto), bounded ring-buffer storage (no raw new/delete),
 * atomic non-blocking enqueue, timed mutex sink serialization, hard loop bounds, total
 * branching, and noexcept RAII destruction.
 */

#ifndef QUASAR_TEST_SUPPORT_ASYNC_TEST_LOGGER_HPP
#define QUASAR_TEST_SUPPORT_ASYNC_TEST_LOGGER_HPP

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>

/**
 * @brief Namespace hosting Quasar test infrastructure helpers compliant with CS-0100.
 *
 * @details Encapsulates asynchronous operational logging, RAII test scope timing guards,
 * and deterministic seeded pseudo-random generators.
 *
 * @task TSK-20261113-004
 * @constraint CS-0010.33, CS-0090.1, CS-0090.2
 * @compliance Encloses all test support declarations within `quasar::test_support`.
 */
namespace quasar::test_support {

/**
 * @brief Symbolic constants governing bounded async logging and deterministic seeding.
 *
 * @details Eliminates magic numbers across ring-buffer sizing, spin/flush bounds,
 * and default deterministic PRNG seeds.
 *
 * @task TSK-20261113-004
 * @constraint CS-0040.1, CS-0040.2, CS-0040.3, CS-0040.6, CS-0090.1, CS-0090.2
 * @compliance Exposes UPPER_SNAKE_CASE constexpr symbolic constants in header scope.
 */
struct TestSupportConstants {
    /// @brief Fixed ring-buffer capacity for lock-free async test log records.
    static constexpr std::size_t RING_CAPACITY = 4096U;
    /// @brief Maximum text payload length stored in a single fixed-size log record.
    static constexpr std::size_t MAX_TEXT_BYTES = 192U;
    /// @brief Maximum test/step label length stored in a log record.
    static constexpr std::size_t MAX_LABEL_BYTES = 64U;
    /// @brief Hard iteration bound per background flush cycle [CS-0010.37].
    static constexpr std::size_t MAX_FLUSH_BATCH = 8192U;
    /// @brief Hard iteration bound for synchronous flush barrier [CS-0010.37].
    static constexpr std::size_t MAX_WAIT_ITERATIONS = 50000U;
    /// @brief Timed mutex wait budget for sink serialization [CS-0010.26].
    static constexpr std::chrono::milliseconds SINK_LOCK_TIMEOUT{50};
    /// @brief Background consumer polling interval when queue is idle.
    static constexpr std::chrono::microseconds WORKER_IDLE_WAIT{250};
    /// @brief Default deterministic seed for reproducible fuzz and boundary tests [CS-0100.12].
    static constexpr std::uint64_t DEFAULT_TEST_SEED = 0x51A5A100C0DE2026ULL;
    /// @brief Default loop log throttling stride to prevent log flooding [CS-0100.20].
    static constexpr std::size_t DEFAULT_LOOP_STRIDE = 100U;
};

/**
 * @brief Fixed-capacity non-allocating log record stored inside the async ring buffer.
 *
 * @details Avoids heap allocations on the producer test path so timing-sensitive and
 * concurrent tests incur minimal latency perturbation.
 *
 * @task TSK-20261113-004
 * @constraint CS-0010.32, CS-0090.1, CS-0090.2
 * @compliance Uses fixed-size std::array buffers with explicit member initialization.
 */
struct AsyncLogRecord {
    /// @brief Sequence slot readiness flag for multi-producer lock-free ring synchronization.
    std::atomic<std::size_t> sequence{0U};
    /// @brief Monotonic timestamp captured at event emission.
    std::uint64_t timestamp_ns{0ULL};
    /// @brief Measured duration in nanoseconds when logging step or test completion.
    std::uint64_t duration_ns{0ULL};
    /// @brief Null-terminated category label (e.g., START, END, INPUT, OUTPUT, BRANCH).
    std::array<char, TestSupportConstants::MAX_LABEL_BYTES> category{};
    /// @brief Null-terminated test case or step identifier.
    std::array<char, TestSupportConstants::MAX_LABEL_BYTES> tag{};
    /// @brief Null-terminated descriptive payload.
    std::array<char, TestSupportConstants::MAX_TEXT_BYTES> message{};
};

/**
 * @brief Asynchronous non-blocking operational test logger using a bounded MPSC ring buffer.
 *
 * @details Producers enqueue fixed-size `AsyncLogRecord` slots atomically without waiting
 * on file/console I/O. A dedicated background `std::jthread` drains records and formats
 * them deterministically outside the critical execution path.
 *
 * @task TSK-20261113-004
 * @constraint CS-0010.21, CS-0010.26, CS-0010.37, CS-0060.5, CS-0090.1, CS-0090.2, CS-0100.20, CS-0100.21
 * @invariant Producer enqueue never acquires blocking I/O locks; destructor joins worker thread noexcept.
 * @compliance Thread-safe atomic ring buffer with timed-mutex sink protection and noexcept shutdown.
 */
class AsyncTestLogger {
public:
    /**
     * @brief Retrieve the process-wide singleton `AsyncTestLogger` instance.
     *
     * @details Uses Meyers singleton initialization guaranteed thread-safe by C++23.
     *
     * @task TSK-20261113-004
     * @constraint CS-0090.1, CS-0090.2, CS-0090.7
     * @compliance Returns stable reference without raw pointers or manual heap allocation.
     * @return Reference to the singleton `AsyncTestLogger`.
     *   Justification: Centralizes background thread ownership across tests in a binary.
     *   Usage: Callers invoke `AsyncTestLogger::instance().logEvent(...)`.
     *   Consumed: Borrowed reference; caller does not own or delete.
     *   Prerequisites: None.
     *   Limitations: Single background drain queue per test process.
     *   Lifecycle: Lives from first invocation until process exit.
     *   Modification / Deletion: Callers may invoke non-const logging methods; deletion forbidden.
     *   Specific Values: Always returns valid initialized instance reference.
     *   Error Behavior: No-throw after first construction.
     */
    [[nodiscard]] static AsyncTestLogger& instance() {
        static AsyncTestLogger logger_instance{};
        return logger_instance;
    }

    AsyncTestLogger(const AsyncTestLogger&) = delete;
    AsyncTestLogger& operator=(const AsyncTestLogger&) = delete;
    AsyncTestLogger(AsyncTestLogger&&) = delete;
    AsyncTestLogger& operator=(AsyncTestLogger&&) = delete;

    /**
     * @brief Enqueue an operational log event without blocking on I/O.
     *
     * @details Copies bounded substrings into the next ring slot via CAS on `enqueue_pos_`.
     * If the bounded buffer is temporarily full, increments `dropped_count_` instead of
     * stalling the test under test.
     *
     * @task TSK-20261113-004
     * @constraint CS-0010.37, CS-0060.3, CS-0090.1, CS-0090.2, CS-0090.3, CS-0100.20, CS-0100.21
     * @compliance Wait-free bounded retry CAS with explicit fallback counter on full queue.
     * @param[in] category
     *   Justification: Classifies record type (START, END, INPUT, OUTPUT, STEP, BRANCH, LOOP).
     *   Usage: Copied up to `MAX_LABEL_BYTES - 1` into fixed buffer.
     *   Consumed: No (read-only string_view).
     *   Prerequisites: Valid string_view (empty permitted).
     * @param[in] tag
     *   Justification: Identifies test name, step name, or variable key.
     *   Usage: Copied up to `MAX_LABEL_BYTES - 1` into fixed buffer.
     *   Consumed: No (read-only string_view).
     *   Prerequisites: Valid string_view (empty permitted).
     * @param[in] message
     *   Justification: Captures serialized value, branch label, or summary payload.
     *   Usage: Copied up to `MAX_TEXT_BYTES - 1` into fixed buffer.
     *   Consumed: No (read-only string_view).
     *   Prerequisites: Valid string_view (empty permitted).
     * @param[in] duration_ns
     *   Justification: Records measured elapsed nanoseconds for timed steps or test scopes.
     *   Usage: Stored verbatim in `AsyncLogRecord::duration_ns`.
     *   Consumed: No (scalar value).
     *   Prerequisites: Non-negative unsigned duration (0 when not applicable).
     */
    void logEvent(std::string_view category,
                  std::string_view tag,
                  std::string_view message,
                  std::uint64_t duration_ns = 0ULL) noexcept {
        // Capture timestamp prior to slot claim for accurate test timing telemetry.
        const std::uint64_t now_ns = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch())
                .count());

        std::size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
        constexpr std::size_t MAX_CAS_ATTEMPTS = 64U;
        // Bounded CAS loop guarantees producer never spins infinitely [CS-0010.37].
        for (std::size_t attempt = 0U; attempt < MAX_CAS_ATTEMPTS; ++attempt) {
            AsyncLogRecord& slot = ring_[pos % TestSupportConstants::RING_CAPACITY];
            const std::size_t seq = slot.sequence.load(std::memory_order_acquire);
            const std::intptr_t diff = static_cast<std::intptr_t>(seq) - static_cast<std::intptr_t>(pos);
            // Slot is free when sequence matches current enqueue position.
            if (diff == 0) {
                if (enqueue_pos_.compare_exchange_weak(pos, pos + 1U, std::memory_order_relaxed)) {
                    slot.timestamp_ns = now_ns;
                    slot.duration_ns = duration_ns;
                    copyBounded(category, slot.category);
                    copyBounded(tag, slot.tag);
                    copyBounded(message, slot.message);
                    slot.sequence.store(pos + 1U, std::memory_order_release);
                    return;
                } else {
                    // CAS contended with another producer thread; retry with updated pos [CS-0060.3].
                }
            } else if (diff < 0) {
                // Ring buffer is full; drop without blocking to preserve test determinism [CS-0100.21].
                dropped_count_.fetch_add(1U, std::memory_order_relaxed);
                return;
            } else {
                // Slot is still advancing by another thread; reload enqueue_pos_ [CS-0060.3].
                pos = enqueue_pos_.load(std::memory_order_relaxed);
            }
        }
        dropped_count_.fetch_add(1U, std::memory_order_relaxed);
    }

    /**
     * @brief Log a typed test input value asynchronously.
     *
     * @task TSK-20261113-004
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0100.20
     * @compliance Formats scalar/string input via std::ostringstream and delegates to non-blocking `logEvent`.
     * @param[in] key
     *   Justification: Names the logged input parameter.
     *   Usage: Passed as record tag.
     *   Consumed: No.
     *   Prerequisites: Non-empty parameter identifier recommended.
     * @param[in] value
     *   Justification: Captures exact input value fed to code under test.
     *   Usage: Streamed to string payload.
     *   Consumed: No (const reference).
     *   Prerequisites: Type `T` supports `operator<<`.
     */
    template <typename T>
    void logInput(std::string_view key, const T& value) noexcept {
        std::ostringstream oss{};
        oss << value;
        logEvent("INPUT", key, oss.str(), 0ULL);
    }

    /**
     * @brief Log a typed test output or return value asynchronously.
     *
     * @task TSK-20261113-004
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0100.20
     * @compliance Formats scalar/string output and delegates to non-blocking `logEvent`.
     * @param[in] key
     *   Justification: Names the logged output or return quantity.
     *   Usage: Passed as record tag.
     *   Consumed: No.
     *   Prerequisites: Non-empty identifier recommended.
     * @param[in] value
     *   Justification: Captures observed result produced by code under test.
     *   Usage: Streamed to string payload.
     *   Consumed: No (const reference).
     *   Prerequisites: Type `T` supports `operator<<`.
     */
    template <typename T>
    void logOutput(std::string_view key, const T& value) noexcept {
        std::ostringstream oss{};
        oss << value;
        logEvent("OUTPUT", key, oss.str(), 0ULL);
    }

    /**
     * @brief Log an executed branch inside a test control structure.
     *
     * @task TSK-20261113-004
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0100.20
     * @compliance Records exact test control-flow path taken via wait-free logEvent.
     * @param[in] branch_name
     *   Justification: Identifies the branch predicate or switch case taken in the test.
     *   Usage: Stored as message under category BRANCH.
     *   Consumed: No.
     *   Prerequisites: Valid string_view.
     */
    void logBranch(std::string_view branch_name) noexcept {
        logEvent("BRANCH", "flow", branch_name, 0ULL);
    }

    /**
     * @brief Log measured duration of a test operational step.
     *
     * @task TSK-20261113-004
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0100.20
     * @compliance Computes elapsed nanoseconds from steady_clock endpoints without blocking.
     * @param[in] step_name
     *   Justification: Identifies the measured step.
     *   Usage: Passed as record tag.
     *   Consumed: No.
     *   Prerequisites: Valid string_view.
     * @param[in] start_tp
     *   Justification: Step start steady_clock timestamp.
     *   Usage: Subtracted from `end_tp`.
     *   Consumed: No.
     *   Prerequisites: Captured prior to or equal to `end_tp`.
     * @param[in] end_tp
     *   Justification: Step end steady_clock timestamp.
     *   Usage: Upper endpoint for elapsed duration calculation.
     *   Consumed: No.
     *   Prerequisites: Captured at or after `start_tp`.
     */
    void logStepDuration(std::string_view step_name,
                         std::chrono::steady_clock::time_point start_tp,
                         std::chrono::steady_clock::time_point end_tp) noexcept {
        const std::int64_t delta_ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end_tp - start_tp).count();
        // Clamp negative clock deltas defensively before unsigned storage.
        const std::uint64_t safe_ns = (delta_ns >= 0) ? static_cast<std::uint64_t>(delta_ns) : 0ULL;
        logEvent("STEP", step_name, "elapsed", safe_ns);
    }

    /**
     * @brief Throttled loop progress logger preventing log flooding inside tight loops.
     *
     * @task TSK-20261113-004
     * @constraint CS-0060.3, CS-0090.1, CS-0090.2, CS-0090.3, CS-0100.20
     * @compliance Logs only first, last, and stride-aligned iterations to bound log volume.
     * @param[in] loop_name
     *   Justification: Identifies the active loop.
     *   Usage: Stored as record tag.
     *   Consumed: No.
     *   Prerequisites: Valid string_view.
     * @param[in] index
     *   Justification: Current zero-based loop iteration.
     *   Usage: Checked against stride and terminal index.
     *   Consumed: No.
     *   Prerequisites: `index < total`.
     * @param[in] total
     *   Justification: Total planned iterations.
     *   Usage: Determines terminal log emission (`index + 1 == total`).
     *   Consumed: No.
     *   Prerequisites: `total > 0`.
     * @param[in] stride
     *   Justification: Sampling period between intermediate loop logs.
     *   Usage: Modulo interval check (falls back to 1 if 0 passed).
     *   Consumed: No.
     *   Prerequisites: Positive stride recommended.
     */
    void logLoopProgress(std::string_view loop_name,
                         std::size_t index,
                         std::size_t total,
                         std::size_t stride = TestSupportConstants::DEFAULT_LOOP_STRIDE) noexcept {
        const std::size_t effective_stride = (stride > 0U) ? stride : 1U;
        // Emit only boundary or stride-aligned samples to suppress loop log spam [CS-0100.20].
        if ((index == 0U) || ((index + 1U) >= total) || ((index % effective_stride) == 0U)) {
            std::ostringstream oss{};
            oss << "iter=" << index << "/" << total;
            logEvent("LOOP", loop_name, oss.str(), 0ULL);
        } else {
            // Intermediate iteration intentionally throttled [CS-0060.3].
        }
    }

    /**
     * @brief Flush queued log records synchronously outside timed assertion paths.
     *
     * @details Intended for post-test or fixture teardown synchronization so all emitted
     * telemetry is persisted without interfering with timed execution measurements.
     *
     * @task TSK-20261113-004
     * @constraint CS-0010.26, CS-0010.37, CS-0060.3, CS-0090.1, CS-0090.2, CS-0100.21
     * @compliance Enforces bounded wait iterations (`MAX_WAIT_ITERATIONS`) and timed drain.
     */
    void flushOutsideTimedPath() noexcept {
        // Bound wait iterations so flush cannot hang if background thread exits early [CS-0010.37].
        for (std::size_t iter = 0U; iter < TestSupportConstants::MAX_WAIT_ITERATIONS; ++iter) {
            const std::size_t enq = enqueue_pos_.load(std::memory_order_acquire);
            const std::size_t deq = dequeue_pos_.load(std::memory_order_acquire);
            // Exit early once all enqueued slots have been drained by worker.
            if (deq >= enq) {
                return;
            } else {
                // Yield briefly while background thread drains pending records [CS-0060.3].
                std::this_thread::yield();
            }
        }
        drainBatch();
    }

    /**
     * @brief Return count of records dropped due to full ring buffer.
     *
     * @task TSK-20261113-004
     * @constraint CS-0060.1, CS-0090.1, CS-0090.2, CS-0090.7
     * @compliance Lock-free atomic read annotated `[[nodiscard]]`.
     * @return Total dropped log record count since process start.
     *   Justification: Allows verification of buffer headroom during heavy stress tests.
     *   Usage: Inspected in diagnostics after test flush.
     *   Consumed: Scalar value copied on return.
     *   Prerequisites: None.
     *   Limitations: Cumulative over process lifetime.
     *   Lifecycle: Independent prvalue scalar.
     *   Modification / Deletion: Not applicable to scalar return.
     *   Specific Values: 0 indicates zero backpressure overflow.
     *   Error Behavior: No-throw atomic load.
     */
    [[nodiscard]] std::size_t droppedCount() const noexcept {
        return dropped_count_.load(std::memory_order_relaxed);
    }

    /**
     * @brief Enable or disable stdout mirroring from the background drain thread.
     *
     * @task TSK-20261113-004
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3
     * @compliance Atomic state toggle without locking.
     * @param[in] enabled
     *   Justification: Allows quiet mode or verbose console output on demand.
     *   Usage: Stored atomically for background writer checks.
     *   Consumed: Scalar bool.
     *   Prerequisites: None.
     */
    void setConsoleOutput(bool enabled) noexcept {
        console_enabled_.store(enabled, std::memory_order_relaxed);
    }

    /**
     * @brief Destructor cleanly stopping the background logger thread and draining remaining records.
     *
     * @task TSK-20261113-004
     * @constraint CS-0060.5, CS-0090.1, CS-0090.2
     * @compliance Explicitly `noexcept`; requests stop and joins `std::jthread` cleanly.
     */
    ~AsyncTestLogger() noexcept {
        running_.store(false, std::memory_order_release);
        if (worker_.joinable()) {
            worker_.request_stop();
            worker_.join();
        } else {
            // Worker already joined or never started [CS-0060.3].
        }
        drainBatch();
    }

private:
    /**
     * @brief Private constructor initializing ring sequences and launching background worker thread.
     *
     * @task TSK-20261113-004
     * @constraint CS-0010.37, CS-0090.1, CS-0090.2
     * @compliance Initializes all ring slot sequences and spawns bounded `std::jthread` worker.
     */
    AsyncTestLogger() {
        // Initialize every slot sequence to its array index for lock-free availability checks.
        for (std::size_t idx = 0U; idx < TestSupportConstants::RING_CAPACITY; ++idx) {
            ring_[idx].sequence.store(idx, std::memory_order_relaxed);
        }
        worker_ = std::jthread([this](std::stop_token stop_tok) {
            workerLoop(stop_tok);
        });
    }

    /**
     * @brief Copy bounded string_view content safely into fixed null-terminated char array.
     *
     * @task TSK-20261113-004
     * @constraint CS-0010.14, CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.3
     * @compliance Enforces null termination and strict bounds against `BufferSize - 1`.
     * @param[in] src
     *   Justification: Source text view.
     *   Usage: Copied character-by-character up to capacity.
     *   Consumed: No.
     *   Prerequisites: Valid string_view.
     * @param[out] dst
     *   Justification: Destination fixed array inside ring slot.
     *   Usage: Populated and null-terminated.
     *   Consumed: Replaced in-place.
     *   Prerequisites: Non-empty `std::array` (`BufferSize >= 1`).
     */
    template <std::size_t BufferSize>
    static void copyBounded(std::string_view src, std::array<char, BufferSize>& dst) noexcept {
        const std::size_t max_copy = (BufferSize > 0U) ? (BufferSize - 1U) : 0U;
        const std::size_t copy_len = (src.size() < max_copy) ? src.size() : max_copy;
        // Bounded copy loop prevents buffer overflow [CS-0010.14, CS-0010.37].
        for (std::size_t idx = 0U; idx < copy_len; ++idx) {
            dst[idx] = src[idx];
        }
        if constexpr (BufferSize > 0U) {
            dst[copy_len] = '\0';
        }
    }

    /**
     * @brief Drain up to `MAX_FLUSH_BATCH` ready records from ring buffer to sink.
     *
     * @task TSK-20261113-004
     * @constraint CS-0010.26, CS-0010.37, CS-0060.3, CS-0090.1, CS-0090.2
     * @compliance Bounded batch drain loop using timed mutex for console formatting.
     * @return Number of records drained in this batch.
     *   Justification: Enables idle-backoff decisions in background worker.
     *   Usage: Checked against 0 to decide short timed wait.
     *   Consumed: Scalar value.
     *   Prerequisites: Single consumer thread or synchronized call.
     *   Limitations: Capped at `MAX_FLUSH_BATCH` per call.
     *   Lifecycle: Independent prvalue scalar.
     *   Modification / Deletion: Scalar value.
     *   Specific Values: `[0, MAX_FLUSH_BATCH]`.
     *   Error Behavior: No-throw.
     */
    std::size_t drainBatch() noexcept {
        std::size_t drained = 0U;
        // Hard iteration bound ensures bounded worker drain cycle [CS-0010.37].
        for (std::size_t step = 0U; step < TestSupportConstants::MAX_FLUSH_BATCH; ++step) {
            const std::size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
            AsyncLogRecord& slot = ring_[pos % TestSupportConstants::RING_CAPACITY];
            const std::size_t seq = slot.sequence.load(std::memory_order_acquire);
            const std::intptr_t diff =
                static_cast<std::intptr_t>(seq) - static_cast<std::intptr_t>(pos + 1U);
            // Stop draining when next slot has not been committed by a producer.
            if (diff < 0) {
                break;
            } else {
                // Slot is committed; advance dequeue cursor and emit record [CS-0060.3].
                dequeue_pos_.store(pos + 1U, std::memory_order_relaxed);
                if (console_enabled_.load(std::memory_order_relaxed)) {
                    std::unique_lock<std::timed_mutex> lock{
                        sink_mutex_, TestSupportConstants::SINK_LOCK_TIMEOUT};
                    // Emit formatted log line only when timed mutex acquired within timeout [CS-0010.26].
                    if (lock.owns_lock()) {
                        std::printf("[ASYNC-TEST][ns=%llu][%s][%s][dt_ns=%llu] %s\n",
                                    static_cast<unsigned long long>(slot.timestamp_ns),
                                    slot.category.data(),
                                    slot.tag.data(),
                                    static_cast<unsigned long long>(slot.duration_ns),
                                    slot.message.data());
                    } else {
                        // Skip formatted output if sink lock contended past timeout [CS-0060.3].
                    }
                } else {
                    // Console output disabled; record still tracked in drained_count_ [CS-0060.3].
                }
                slot.sequence.store(pos + TestSupportConstants::RING_CAPACITY,
                                    std::memory_order_release);
                drained_count_.fetch_add(1U, std::memory_order_relaxed);
                ++drained;
            }
        }
        return drained;
    }

    /**
     * @brief Background consumer loop draining queued records until stop requested.
     *
     * @task TSK-20261113-004
     * @constraint CS-0010.27, CS-0010.37, CS-0060.3, CS-0090.1, CS-0090.2, CS-0090.3
     * @compliance Bounded shutdown-aware loop using short idle wait.
     * @param[in] stop_tok
     *   Justification: Cooperative cancellation token from `std::jthread`.
     *   Usage: Polled on every outer cycle alongside `running_`.
     *   Consumed: No.
     *   Prerequisites: Bound to `worker_` thread.
     */
    void workerLoop(std::stop_token stop_tok) noexcept {
        constexpr std::size_t MAX_WORKER_IDLE_CYCLES = 100000000U;
        // Bounded outer service loop with explicit cooperative termination check [CS-0010.37].
        for (std::size_t cycle = 0U; cycle < MAX_WORKER_IDLE_CYCLES; ++cycle) {
            const std::size_t count = drainBatch();
            // Exit immediately once stop is requested and queue is empty.
            if (!running_.load(std::memory_order_acquire) || stop_tok.stop_requested()) {
                if (count == 0U) {
                    return;
                } else {
                    // Drain remaining queued items before exiting [CS-0060.3].
                }
            } else if (count == 0U) {
                std::this_thread::sleep_for(TestSupportConstants::WORKER_IDLE_WAIT);
            } else {
                // Active batch drained; continue immediately [CS-0060.3].
            }
        }
    }

    /// @brief Fixed-capacity array storing asynchronous log records.
    std::array<AsyncLogRecord, TestSupportConstants::RING_CAPACITY> ring_{};
    /// @brief Monotonically advancing producer claim index.
    std::atomic<std::size_t> enqueue_pos_{0U};
    /// @brief Monotonically advancing consumer drain index.
    std::atomic<std::size_t> dequeue_pos_{0U};
    /// @brief Total records dropped on buffer saturation.
    std::atomic<std::size_t> dropped_count_{0U};
    /// @brief Total records drained successfully.
    std::atomic<std::size_t> drained_count_{0U};
    /// @brief Worker run state flag.
    std::atomic<bool> running_{true};
    /// @brief Toggle controlling formatted stdout emission from worker thread.
    std::atomic<bool> console_enabled_{false};
    /// @brief Timed mutex serializing formatted sink output [CS-0010.46].
    std::timed_mutex sink_mutex_{};
    /// @brief Background worker thread draining records asynchronously.
    std::jthread worker_{};
};

/**
 * @brief RAII guard automatically logging test START, END, and total duration.
 *
 * @details Records `START` upon construction and `END` + elapsed duration upon scope exit
 * without throwing exceptions, then flushes async records cleanly outside assertions.
 *
 * @task TSK-20261113-004
 * @constraint CS-0010.21, CS-0060.5, CS-0090.1, CS-0090.2, CS-0100.20, CS-0100.21
 * @compliance RAII scope object with noexcept destructor ensuring deterministic test telemetry.
 */
class TestScopeGuard {
public:
    /**
     * @brief Begin a timed test telemetry scope.
     *
     * @task TSK-20261113-004
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3
     * @compliance Records `START` event asynchronously with zero blocking I/O.
     * @param[in] test_name
     *   Justification: Identifies the active test case in operational logs.
     *   Usage: Stored as `std::string` and emitted on START and END events.
     *   Consumed: Copied into member `test_name_`.
     *   Prerequisites: Non-empty test identifier recommended.
     */
    explicit TestScopeGuard(std::string_view test_name) noexcept
        : test_name_(test_name), start_tp_(std::chrono::steady_clock::now()) {
        AsyncTestLogger::instance().logEvent("START", test_name_, "begin", 0ULL);
    }

    TestScopeGuard(const TestScopeGuard&) = delete;
    TestScopeGuard& operator=(const TestScopeGuard&) = delete;

    /**
     * @brief End the timed test telemetry scope and flush buffered records.
     *
     * @task TSK-20261113-004
     * @constraint CS-0060.5, CS-0090.1, CS-0090.2
     * @compliance Strictly `noexcept` destructor computing duration and flushing queue.
     */
    ~TestScopeGuard() noexcept {
        const std::chrono::steady_clock::time_point end_tp = std::chrono::steady_clock::now();
        const std::int64_t delta_ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end_tp - start_tp_).count();
        const std::uint64_t safe_ns = (delta_ns >= 0) ? static_cast<std::uint64_t>(delta_ns) : 0ULL;
        AsyncTestLogger::instance().logEvent("END", test_name_, "complete", safe_ns);
        AsyncTestLogger::instance().flushOutsideTimedPath();
    }

private:
    /// @brief Identifier of the active test case.
    std::string test_name_{};
    /// @brief Steady clock timestamp captured at test entry.
    std::chrono::steady_clock::time_point start_tp_{};
};

/**
 * @brief Deterministic fixed-seed PRNG wrapper compliant with CS-0060.7 and CS-0100.12.
 *
 * @details Wraps `std::mt19937_64` and logs the active seed via `AsyncTestLogger` at
 * construction so any fuzzing or randomized schedule run is 100% reproducible.
 *
 * @task TSK-20261113-004
 * @constraint CS-0060.7, CS-0090.1, CS-0090.2, CS-0100.10, CS-0100.12
 * @compliance Uses `<random>` (`std::mt19937_64`) instead of `std::rand` and logs seed.
 */
class DeterministicRng {
public:
    /**
     * @brief Construct deterministic generator with explicit seed and log it.
     *
     * @task TSK-20261113-004
     * @constraint CS-0090.1, CS-0090.2, CS-0090.3
     * @compliance Logs seed value via `AsyncTestLogger::logInput` for reproducibility.
     * @param[in] seed
     *   Justification: Guarantees identical pseudo-random sequences across runs.
     *   Usage: Seeds internal `std::mt19937_64` engine.
     *   Consumed: Scalar uint64_t.
     *   Prerequisites: Any 64-bit unsigned seed value.
     */
    explicit DeterministicRng(
        std::uint64_t seed = TestSupportConstants::DEFAULT_TEST_SEED) noexcept
        : engine_(seed) {
        AsyncTestLogger::instance().logInput("rng_seed", seed);
    }

    /**
     * @brief Generate deterministic integer in inclusive range `[min_val, max_val]`.
     *
     * @task TSK-20261113-004
     * @constraint CS-0060.3, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7
     * @compliance Normalizes inverted bounds deterministically before sampling.
     * @param[in] min_val
     *   Justification: Lower inclusive bound.
     *   Usage: Passed to `std::uniform_int_distribution`.
     *   Consumed: Scalar int64_t.
     *   Prerequisites: `min_val <= max_val` (swapped safely if inverted).
     * @param[in] max_val
     *   Justification: Upper inclusive bound.
     *   Usage: Passed to `std::uniform_int_distribution`.
     *   Consumed: Scalar int64_t.
     *   Prerequisites: `max_val >= min_val`.
     * @return Deterministic int64_t within `[min_val, max_val]`.
     *   Justification: Supplies fuzzed bounded integers.
     *   Usage: Passed to tested API boundaries.
     *   Consumed: Scalar value returned by copy.
     *   Prerequisites: Valid generator state.
     *   Limitations: Period bounded by MT19937-64.
     *   Lifecycle: Independent scalar prvalue.
     *   Modification / Deletion: Not applicable.
     *   Specific Values: Always within `[min_val, max_val]`.
     *   Error Behavior: No-throw.
     */
    [[nodiscard]] std::int64_t nextInt(std::int64_t min_val, std::int64_t max_val) noexcept {
        std::int64_t low = min_val;
        std::int64_t high = max_val;
        // Normalize bounds if caller inverted min/max to maintain distribution precondition.
        if (low > high) {
            low = max_val;
            high = min_val;
        } else {
            // Bounds are ordered [CS-0060.3].
        }
        std::uniform_int_distribution<std::int64_t> dist{low, high};
        return dist(engine_);
    }

    /**
     * @brief Generate deterministic double in range `[min_val, max_val]`.
     *
     * @task TSK-20261113-004
     * @constraint CS-0060.3, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7
     * @compliance Uses `std::uniform_real_distribution` over ordered finite bounds.
     * @param[in] min_val
     *   Justification: Lower floating-point bound.
     *   Usage: Distribution minimum.
     *   Consumed: Scalar double.
     *   Prerequisites: Finite double value.
     * @param[in] max_val
     *   Justification: Upper floating-point bound.
     *   Usage: Distribution maximum.
     *   Consumed: Scalar double.
     *   Prerequisites: Finite double value.
     * @return Deterministic double in `[min_val, max_val]`.
     *   Justification: Supplies fuzzed floating-point payloads.
     *   Usage: Passed to numeric/codec/calibration APIs.
     *   Consumed: Scalar value.
     *   Prerequisites: Valid engine state.
     *   Limitations: Uniform floating-point sampling.
     *   Lifecycle: Independent scalar prvalue.
     *   Modification / Deletion: Not applicable.
     *   Specific Values: Bounded finite double.
     *   Error Behavior: No-throw.
     */
    [[nodiscard]] double nextDouble(double min_val, double max_val) noexcept {
        double low = min_val;
        double high = max_val;
        // Normalize inverted bounds safely before uniform_real_distribution construction.
        if (low > high) {
            low = max_val;
            high = min_val;
        } else {
            // Bounds are ordered [CS-0060.3].
        }
        std::uniform_real_distribution<double> dist{low, high};
        return dist(engine_);
    }

    /**
     * @brief Generate deterministic printable ASCII string of requested length.
     *
     * @task TSK-20261113-004
     * @constraint CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7
     * @compliance Bounded generation loop capped by `MAX_GENERATED_LEN`.
     * @param[in] length
     *   Justification: Desired output string length.
     *   Usage: Clamped to `MAX_GENERATED_LEN` to bound allocation and runtime.
     *   Consumed: Scalar size_t.
     *   Prerequisites: Any size_t (0 returns empty string).
     * @return Deterministically generated `std::string`.
     *   Justification: Supplies fuzzed string identifiers/payloads.
     *   Usage: Passed to parser/tree/codec interfaces.
     *   Consumed: Returned by value (NRVO/move).
     *   Prerequisites: Valid engine state.
     *   Limitations: Length capped at 16384 characters per call.
     *   Lifecycle: Caller-owned `std::string`.
     *   Modification / Deletion: Caller may freely mutate or discard.
     *   Specific Values: Strictly printable ASCII characters `['a'..'z', '0'..'9', '_']`.
     *   Error Behavior: Propagates `std::bad_alloc` only on OOM.
     */
    [[nodiscard]] std::string nextAsciiString(std::size_t length) {
        constexpr std::size_t MAX_GENERATED_LEN = 16384U;
        constexpr std::string_view ALPHABET = "abcdefghijklmnopqrstuvwxyz0123456789_";
        const std::size_t capped_len = (length <= MAX_GENERATED_LEN) ? length : MAX_GENERATED_LEN;
        std::string result{};
        result.reserve(capped_len);
        std::uniform_int_distribution<std::size_t> dist{0U, ALPHABET.size() - 1U};
        // Bounded loop generating deterministic ASCII characters [CS-0010.37].
        for (std::size_t idx = 0U; idx < capped_len; ++idx) {
            result.push_back(ALPHABET[dist(engine_)]);
        }
        return result;
    }

private:
    /// @brief Deterministic 64-bit Mersenne Twister engine.
    std::mt19937_64 engine_{};
};

} // namespace quasar::test_support

#endif // QUASAR_TEST_SUPPORT_ASYNC_TEST_LOGGER_HPP
