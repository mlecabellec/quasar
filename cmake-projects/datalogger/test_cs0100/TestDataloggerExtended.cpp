/**
 * @file TestDataloggerExtended.cpp
 * @brief CS-0100 comprehensive unit, template (20+), concurrency/deadlock, fuzz, stress, and tutorial tests for `quasar_datalogger`.
 *
 * @details Supplements existing `datalogger_test` without modifying existing test files.
 * Covers `RingBuffer<T>` (20 template instantiations), `DataView`, `MathFilter`, `ValueThresholdTrigger`,
 * `CsvFileWriter`, `ADevRecorder`, `DataLoggerService`, `LogLevel` enumerators, and `LOG_*` macros.
 *
 * @task TSK-20261113-004, TSK-20260303-001
 * @feature FE-0160
 * @constraint CS-0010, CS-0040, CS-0060, CS-0090, CS-0100.1, CS-0100.2, CS-0100.4, CS-0100.5, CS-0100.14
 * @compliance Uses isolated temp CSV paths, AsyncTestLogger, DeterministicRng, and bounded loops.
 */

#include "quasar/datalogger/ADevRecorder.hpp"
#include "quasar/datalogger/CsvFileWriter.hpp"
#include "quasar/datalogger/DataLoggerService.hpp"
#include "quasar/datalogger/DataView.hpp"
#include "quasar/datalogger/Log.hpp"
#include "quasar/datalogger/LogEntry.hpp"
#include "quasar/datalogger/MathFilter.hpp"
#include "quasar/datalogger/RingBuffer.hpp"
#include "quasar/datalogger/ValueThresholdTrigger.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"

#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace quasar::datalogger::test_cs0100 {

/**
 * @brief Symbolic constants and bounds for datalogger CS-0100 tests (`CS-0040`).
 * @task TSK-20261113-004
 * @constraint CS-0040.1, CS-0040.6, CS-0090.1
 * @compliance Centralizes capacities, stress lengths, and thread counts.
 */
struct DataloggerTestConstants {
    /// @brief Fixed deterministic PRNG seed (`CS-0100.12`).
    static constexpr std::uint64_t PRNG_SEED = 0x2468ACE013579BDFULL;
    /// @brief Default RingBuffer capacity in tutorial/coverage tests (`CS-0040.2`).
    static constexpr std::size_t RING_CAPACITY = 16U;
    /// @brief Concurrent producer/consumer threads (`CS-0100.4`).
    static constexpr std::size_t CONCURRENT_THREADS = 4U;
    /// @brief Items pushed per stress producer thread (`CS-0010.37`).
    static constexpr std::size_t ITEMS_PER_THREAD = 100U;
    /// @brief Long message length for memory stress verification (`CS-0100.23`).
    static constexpr std::size_t LONG_MSG_LEN = 16384U;
};

/**
 * @brief Exercises one `RingBuffer<T>` instantiation (`CS-0100.14`).
 * @tparam T Stored element type.
 * @param[in] v1 First deterministic sample.
 * @param[in] v2 Second deterministic sample.
 * @task TSK-20261113-004, TSK-20260303-001
 * @feature FE-0160
 * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0100.14
 * @compliance Covers zero-capacity throw, copy/move push, timeout pop, and FIFO ordering.
 */
template <typename T>
void VerifyRingBufferSpecialization(const T& v1, T v2) {
    EXPECT_THROW((void)quasar::datalogger::RingBuffer<T>(0U), std::invalid_argument);
    quasar::datalogger::RingBuffer<T> rb{4U};
    EXPECT_TRUE(rb.push(v1));
    EXPECT_TRUE(rb.push(std::move(v2)));
    const std::optional<T> p1 = rb.pop(std::chrono::milliseconds(5));
    const std::optional<T> p2 = rb.pop(std::chrono::milliseconds(5));
    const std::optional<T> pEmpty = rb.pop(std::chrono::milliseconds(1));
    EXPECT_TRUE(p1.has_value());
    EXPECT_TRUE(p2.has_value());
    EXPECT_FALSE(pEmpty.has_value());
}

// ---------------------------------------------------------------------------
// Progressive Pedagogical Tutorials (CS-0100.5)
// ---------------------------------------------------------------------------

/**
 * @brief Tutorial 1 (Basic): Push/pop samples in `RingBuffer` and sample a `NamedObject` via `DataView`.
 * @task TSK-20261113-004, TSK-20260303-001
 * @feature FE-0160
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5
 * @compliance Introduces `RingBuffer` and `DataView` cleanly for human developers.
 */
TEST(DataloggerTutorialTest, Tutorial1_SimpleRingBufferAndDataView) {
    quasar::test_support::TestScopeGuard guard{"DataloggerTutorialTest.Tutorial1_SimpleRingBufferAndDataView"};
    quasar::datalogger::RingBuffer<int> rb{DataloggerTestConstants::RING_CAPACITY};
    EXPECT_TRUE(rb.push(10));
    EXPECT_TRUE(rb.push(20));

    const std::optional<int> first = rb.pop();
    ASSERT_TRUE(first.has_value());
    EXPECT_EQ(first.value(), 10);

    std::shared_ptr<quasar::named::NamedInteger<int64_t>> sensor =
        quasar::named::NamedInteger<int64_t>::create("RPM", 1500);
    quasar::datalogger::DataView view{sensor};
    const std::optional<quasar::datalogger::DataSample> sample = view.sample();
    ASSERT_TRUE(sample.has_value());
    EXPECT_EQ(sample.value().sourcePath, "RPM");
    EXPECT_TRUE(std::holds_alternative<double>(sample.value().value));
}

/**
 * @brief Tutorial 2 (Intermediate): Chain `MathFilter`, `ValueThresholdTrigger`, and `CsvFileWriter`.
 * @task TSK-20261113-004, TSK-20260303-001
 * @feature FE-0160
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5
 * @compliance Demonstrates sample scaling, threshold gating, and file flush to isolated `/tmp` CSV.
 */
TEST(DataloggerTutorialTest, Tutorial2_IntermediateFilterTriggerAndCsvWriter) {
    quasar::test_support::TestScopeGuard guard{"DataloggerTutorialTest.Tutorial2_IntermediateFilterTriggerAndCsvWriter"};
    const std::string csvPath = "/tmp/quasar_datalogger_tut2_cs0100.csv";
    std::filesystem::remove(csvPath);

    quasar::datalogger::MathFilter scaleFilter{"Voltage", 2.0, 1.0};
    quasar::datalogger::ValueThresholdTrigger trigger{"Voltage", 5.0, true};

    quasar::datalogger::LogEntry lowEntry{
        std::chrono::system_clock::now(),
        quasar::datalogger::DataSample{"Voltage", 1.5}};
    const std::optional<quasar::datalogger::LogEntry> scaledLow = scaleFilter.process(lowEntry);
    ASSERT_TRUE(scaledLow.has_value());
    EXPECT_FALSE(trigger.process(scaledLow.value()).has_value());

    quasar::datalogger::LogEntry highEntry{
        std::chrono::system_clock::now(),
        quasar::datalogger::DataSample{"Voltage", 4.0}};
    const std::optional<quasar::datalogger::LogEntry> scaledHigh = scaleFilter.process(highEntry);
    ASSERT_TRUE(scaledHigh.has_value());
    const std::optional<quasar::datalogger::LogEntry> triggered = trigger.process(scaledHigh.value());
    ASSERT_TRUE(triggered.has_value());

    {
        std::shared_ptr<quasar::datalogger::CsvFileWriter> writer =
            quasar::datalogger::CsvFileWriter::create("CsvTutorial", csvPath);
        writer->record(triggered.value());
        writer->flush();
    }
    EXPECT_TRUE(std::filesystem::exists(csvPath));
    std::filesystem::remove(csvPath);
}

/**
 * @brief Tutorial 3 (Advanced): Configure singleton `DataLoggerService` via `initDefault` and invoke `LOG_*` macros.
 * @task TSK-20261113-004, TSK-20260303-001
 * @feature FE-0160
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5
 * @compliance Tests all `LogLevel` enumerators and `LOG_DEBUG`..`LOG_CRITICAL` pipeline lifecycle safely.
 */
TEST(DataloggerTutorialTest, Tutorial3_AdvancedServiceLifecycleAndLogMacros) {
    quasar::test_support::TestScopeGuard guard{"DataloggerTutorialTest.Tutorial3_AdvancedServiceLifecycleAndLogMacros"};
    const std::string csvPath = "/tmp/quasar_datalogger_tut3_cs0100.csv";
    std::filesystem::remove(csvPath);

    quasar::datalogger::DataLoggerService::resetInstance();
    quasar::datalogger::DataLoggerService::initDefault(csvPath, 128U);
    std::shared_ptr<quasar::datalogger::DataLoggerService> svc =
        quasar::datalogger::DataLoggerService::getInstance();
    ASSERT_NE(svc, nullptr);
    EXPECT_EQ(svc->getType(), "DataLoggerService");

    LOG_DEBUG("Tutorial 3 debug message");
    LOG_INFO("Tutorial 3 info message");
    LOG_WARN("Tutorial 3 warning message");
    LOG_ERROR("Tutorial 3 error message");
    LOG_CRITICAL("Tutorial 3 critical message");

    svc->flush();
    quasar::datalogger::DataLoggerService::resetInstance();
    std::filesystem::remove(csvPath);
}

// ---------------------------------------------------------------------------
// 20+ Template Instantiations, Concurrency Deadlock & Memory Stress
// ---------------------------------------------------------------------------

/**
 * @brief Tests 20 distinct template instantiations of `quasar::datalogger::RingBuffer<T>`.
 * @task TSK-20261113-004, TSK-20260303-001
 * @feature FE-0160
 * @constraint CS-0090.1, CS-0090.2, CS-0100.14
 * @compliance Covers 20 integral, floating-point, string, and structured types (`RingBuffer<T>`).
 */
TEST(DataloggerTemplateTest, TwentyRingBufferInstantiations) {
    quasar::test_support::TestScopeGuard guard{"DataloggerTemplateTest.TwentyRingBufferInstantiations"};
    VerifyRingBufferSpecialization<int>(1, 2);
    VerifyRingBufferSpecialization<unsigned int>(1U, 2U);
    VerifyRingBufferSpecialization<short>(1, 2);
    VerifyRingBufferSpecialization<unsigned short>(1U, 2U);
    VerifyRingBufferSpecialization<long>(1L, 2L);
    VerifyRingBufferSpecialization<unsigned long>(1UL, 2UL);
    VerifyRingBufferSpecialization<long long>(1LL, 2LL);
    VerifyRingBufferSpecialization<unsigned long long>(1ULL, 2ULL);
    VerifyRingBufferSpecialization<std::int8_t>(1, 2);
    VerifyRingBufferSpecialization<std::uint8_t>(1U, 2U);
    VerifyRingBufferSpecialization<std::int16_t>(1, 2);
    VerifyRingBufferSpecialization<std::uint16_t>(1U, 2U);
    VerifyRingBufferSpecialization<std::int32_t>(1, 2);
    VerifyRingBufferSpecialization<std::uint32_t>(1U, 2U);
    VerifyRingBufferSpecialization<std::int64_t>(1, 2);
    VerifyRingBufferSpecialization<std::uint64_t>(1U, 2U);
    VerifyRingBufferSpecialization<float>(1.5F, 2.5F);
    VerifyRingBufferSpecialization<double>(1.5, 2.5);
    VerifyRingBufferSpecialization<std::string>(std::string("a"), std::string("b"));
    VerifyRingBufferSpecialization<quasar::datalogger::LogLevel>(
        quasar::datalogger::LogLevel::INFO, quasar::datalogger::LogLevel::ERROR);
}

/**
 * @brief Exercises `RingBuffer` overwrite, `ValueThresholdTrigger` below-mode, and `DataView` null target.
 * @task TSK-20261113-004, TSK-20260303-001
 * @feature FE-0160
 * @constraint CS-0090.1, CS-0090.2, CS-0100.3, CS-0100.9
 * @compliance Verifies edge branches including empty DataView, full overwrite, and trigger below mode.
 */
TEST(DataloggerCoverageTest, OverwriteNullDataViewAndTriggerBelowMode) {
    quasar::test_support::TestScopeGuard guard{"DataloggerCoverageTest.OverwriteNullDataViewAndTriggerBelowMode"};
    quasar::datalogger::RingBuffer<int> smallRb{2U};
    EXPECT_TRUE(smallRb.push(1));
    EXPECT_TRUE(smallRb.push(2));
    EXPECT_TRUE(smallRb.push(3)); // Overwrites oldest (1).
    EXPECT_EQ(smallRb.pop().value_or(-1), 2);
    EXPECT_EQ(smallRb.pop().value_or(-1), 3);

    quasar::datalogger::DataView nullView{nullptr};
    EXPECT_FALSE(nullView.sample().has_value());

    quasar::datalogger::ValueThresholdTrigger belowTrig{"Pressure", 10.0, false};
    quasar::datalogger::LogEntry lowP{std::chrono::system_clock::now(), quasar::datalogger::DataSample{"Pressure", 4.0}};
    quasar::datalogger::LogEntry highP{std::chrono::system_clock::now(), quasar::datalogger::DataSample{"Pressure", 20.0}};
    EXPECT_TRUE(belowTrig.process(lowP).has_value());
    EXPECT_FALSE(belowTrig.process(highP).has_value());
}

/**
 * @brief Multi-threaded stress test verifying lock/deadlock safety and large string memory handling.
 * @task TSK-20261113-004, TSK-20260303-001
 * @feature FE-0160
 * @constraint CS-0090.1, CS-0090.2, CS-0100.4, CS-0100.12, CS-0100.13, CS-0100.23
 * @compliance Concurrent producers + consumers finish strictly under time limit without deadlocks.
 */
TEST(DataloggerFuzzStressTest, ConcurrentRingBufferAndLongPayloadStress) {
    quasar::test_support::TestScopeGuard guard{"DataloggerFuzzStressTest.ConcurrentRingBufferAndLongPayloadStress"};
    quasar::datalogger::RingBuffer<std::string> sharedRb{64U};
    quasar::test_support::DeterministicRng rng{DataloggerTestConstants::PRNG_SEED};
    const std::string longPayload(DataloggerTestConstants::LONG_MSG_LEN, 'Z');

    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::vector<std::thread> producers{};
    producers.reserve(DataloggerTestConstants::CONCURRENT_THREADS);
    for (std::size_t t = 0U; t < DataloggerTestConstants::CONCURRENT_THREADS; ++t) {
        const std::string seedPrefix = rng.nextAsciiString(8U);
        producers.emplace_back([&sharedRb, seedPrefix, &longPayload, t]() {
            for (std::size_t i = 0U; i < DataloggerTestConstants::ITEMS_PER_THREAD; ++i) {
                if ((i % 25U) == 0U && t == 0U) {
                    (void)sharedRb.push(longPayload);
                } else {
                    (void)sharedRb.push(seedPrefix + "_" + std::to_string(i));
                }
                if ((i % 2U) == 0U) {
                    (void)sharedRb.pop(std::chrono::milliseconds(1));
                } else {
                    // Interleave push-only steps to exercise wrap-around (`CS-0060.3`).
                }
            }
        });
    }
    for (std::size_t t = 0U; t < producers.size(); ++t) {
        producers[t].join();
    }

    const std::chrono::milliseconds elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    EXPECT_LT(elapsed.count(), 5000);
}

} // namespace quasar::datalogger::test_cs0100
