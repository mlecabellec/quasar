/**
 * @file TestUtils.cpp
 * @brief Comprehensive CS-0100 test suite for the `utils` SMP services module.
 *
 * @details Covers `utils::EventManager`, `utils::Logger`, and `utils::TimeKeeper`
 * across all public methods, enum values, error/exception sources, concurrent access
 * and lifecycle, timed-mutex bounds, seeded fuzzing, stress tests, and 3-tier
 * progressive developer tutorial cases.
 *
 * @task TSK-20261113-004
 * @feature FE-0070, FE-0080
 * @constraint CS-0010.26, CS-0010.34, CS-0010.37, CS-0060.3, CS-0090.1, CS-0090.2, CS-0100.1, CS-0100.2, CS-0100.3, CS-0100.4, CS-0100.6, CS-0100.7, CS-0100.8, CS-0100.9, CS-0100.10, CS-0100.11, CS-0100.12, CS-0100.13, CS-0100.19, CS-0100.20, CS-0100.21
 * @compliance Uses explicit types (no auto), RAII test scope async logging, bounded
 * loops, deterministic PRNG seeding, and full traceability excluding CS-0100 alone.
 */

#include <gtest/gtest.h>

#include "quasar/test_support/AsyncTestLogger.hpp"
#include "utils/EventManager.hpp"
#include "utils/Logger.hpp"
#include "utils/TimeKeeper.hpp"

#include <Smp/IEntryPoint.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ILogger.h>
#include <core/Object.hpp>
#include <core/StandardExceptions.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace utils::test {

/**
 * @brief Symbolic constants bounding utils test iterations and durations [CS-0040.2].
 * @task TSK-20261113-004
 * @feature FE-0070
 * @constraint CS-0040.2, CS-0040.6, CS-0090.1, CS-0090.2
 * @compliance Replaces literal bounds with UPPER_SNAKE_CASE symbolic constants.
 */
struct UtilsTestConstants {
    static constexpr std::size_t MAX_STRESS_ITERATIONS = 500U;
    static constexpr std::size_t MAX_FUZZ_SAMPLES = 100U;
    static constexpr std::size_t CONCURRENT_THREADS = 4U;
    static constexpr std::chrono::milliseconds MAX_TEST_BUDGET{2000};
};

/**
 * @brief Deterministic SMP entry point stub counting callback invocations.
 * @task TSK-20261113-004
 * @feature FE-0070
 * @constraint CS-0090.1, CS-0090.2
 * @compliance Implements `Smp::IEntryPoint` with atomic call counter.
 */
class CountingEntryPoint : public core::Object, public virtual Smp::IEntryPoint {
public:
    CountingEntryPoint(Smp::String8 name, std::atomic<std::size_t>& counter)
        : core::Object(name, "Counting entry point", nullptr), counter_(counter) {}

    void Execute() const override {
        counter_.fetch_add(1U, std::memory_order_relaxed);
    }

private:
    std::atomic<std::size_t>& counter_;
};

// ---------------------------------------------------------------------------
// Tutorial Tests (CS-0100.19): 3 progressive cases for EventManager, Logger, TimeKeeper
// ---------------------------------------------------------------------------

/**
 * @brief Tutorial 1 (Simple): Minimal EventManager subscription and emission workflow.
 * @task TSK-20261113-004
 * @feature FE-0070.4
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.19, CS-0100.20, CS-0100.21
 * @compliance Demonstrates basic QueryEventId -> Subscribe -> Emit -> Unsubscribe flow.
 */
TEST(UtilsTutorialTest, Tutorial1_SimpleEventManagerFlow) {
    quasar::test_support::TestScopeGuard guard{"UtilsTutorialTest.Tutorial1_SimpleEventManagerFlow"};
    utils::EventManager manager{};
    std::atomic<std::size_t> hits{0U};
    CountingEntryPoint ep{"ep_simple", hits};

    // Step 1: Query or register a named event ID.
    const Smp::Services::EventId evt_id = manager.QueryEventId("TutorialEvent");
    quasar::test_support::AsyncTestLogger::instance().logInput("event_name", "TutorialEvent");
    quasar::test_support::AsyncTestLogger::instance().logOutput("evt_id", evt_id);

    // Step 2: Subscribe, emit, and verify callback executed once.
    manager.Subscribe(evt_id, &ep);
    manager.Emit(evt_id, true);
    EXPECT_EQ(hits.load(std::memory_order_relaxed), 1U);
    quasar::test_support::AsyncTestLogger::instance().logOutput("hits", hits.load());

    // Step 3: Unsubscribe and confirm subsequent emit does not trigger callback.
    manager.Unsubscribe(evt_id, &ep);
    manager.Emit(evt_id, true);
    EXPECT_EQ(hits.load(std::memory_order_relaxed), 1U);
}

/**
 * @brief Tutorial 2 (Intermediate): Coupled TimeKeeper and EventManager notifications.
 * @task TSK-20261113-004
 * @feature FE-0070.2, FE-0070.4
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.19, CS-0100.20
 * @compliance Shows wiring `TimeKeeper::SetEventManager` to emit epoch/mission time events.
 */
TEST(UtilsTutorialTest, Tutorial2_IntermediateTimeKeeperWithEvents) {
    quasar::test_support::TestScopeGuard guard{"UtilsTutorialTest.Tutorial2_IntermediateTimeKeeperWithEvents"};
    utils::EventManager manager{};
    utils::TimeKeeper tk{};
    std::atomic<std::size_t> epoch_hits{0U};
    std::atomic<std::size_t> mission_hits{0U};
    CountingEntryPoint ep_epoch{"ep_epoch", epoch_hits};
    CountingEntryPoint ep_mission{"ep_mission", mission_hits};

    const Smp::Services::EventId epoch_id =
        manager.QueryEventId(Smp::Services::IEventManager::SMP_EpochTimeChanged);
    const Smp::Services::EventId mission_id =
        manager.QueryEventId(Smp::Services::IEventManager::SMP_MissionTimeChanged);

    manager.Subscribe(epoch_id, &ep_epoch);
    manager.Subscribe(mission_id, &ep_mission);
    tk.SetEventManager(&manager);

    tk.SetSimulationTime(1000LL);
    tk.SetEpochTime(5000LL);
    tk.SetMissionStartTime(2000LL);

    EXPECT_EQ(tk.GetSimulationTime(), 1000LL);
    EXPECT_EQ(tk.GetEpochTime(), 5000LL);
    EXPECT_EQ(tk.GetMissionStartTime(), 2000LL);
    EXPECT_EQ(epoch_hits.load(std::memory_order_relaxed), 1U);
    EXPECT_EQ(mission_hits.load(std::memory_order_relaxed), 1U);
}

/**
 * @brief Tutorial 3 (Advanced): Multi-service Logger, TimeKeeper, and re-entrant EventManager chain.
 * @task TSK-20261113-004
 * @feature FE-0070.1, FE-0070.2, FE-0070.4
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.19, CS-0100.20
 * @compliance Demonstrates logging across SMP kinds combined with event-triggered updates.
 */
TEST(UtilsTutorialTest, Tutorial3_AdvancedMultiServiceComposition) {
    quasar::test_support::TestScopeGuard guard{"UtilsTutorialTest.Tutorial3_AdvancedMultiServiceComposition"};
    utils::Logger logger{};
    utils::EventManager manager{};
    utils::TimeKeeper tk{};

    const Smp::Services::LogMessageKind info_kind =
        logger.QueryLogMessageKind(Smp::Services::ILogger::LMK_InformationName);
    EXPECT_EQ(info_kind, Smp::Services::ILogger::LMK_Information);

    tk.SetEventManager(&manager);
    tk.SetSimulationTime(2500LL);
    tk.SetMissionTime(500LL);

    const Smp::DateTime zulu_now = tk.GetZuluTime();
    quasar::test_support::AsyncTestLogger::instance().logOutput("zulu_now", zulu_now);
    EXPECT_GT(zulu_now, 0LL);

    logger.Log(&tk, "Tutorial 3 simulation checkpoint", info_kind);
    EXPECT_EQ(logger.GetState(), Smp::ComponentStateKind::CSK_Connected);
}

// ---------------------------------------------------------------------------
// Exhaustive Method, Enum, Boundary & Exception Coverage (CS-0100.1, .3, .4, .6, .9)
// ---------------------------------------------------------------------------

/**
 * @brief Verifies all EventManager, Logger, and TimeKeeper IComponent stubs and metadata.
 * @task TSK-20261113-004
 * @feature FE-0070, FE-0080
 * @constraint CS-0090.1, CS-0090.2, CS-0100.1, CS-0100.2, CS-0100.3
 * @compliance Exercises every `IComponent` virtual override across all three utils services.
 */
TEST(UtilsCoverageTest, AllComponentStubsAndMetadata) {
    quasar::test_support::TestScopeGuard guard{"UtilsCoverageTest.AllComponentStubsAndMetadata"};
    utils::EventManager ev{};
    utils::Logger lg{};
    utils::TimeKeeper tk{};
    std::array<Smp::IComponent*, 3U> services{&ev, &lg, &tk};

    for (std::size_t idx = 0U; idx < services.size(); ++idx) {
        Smp::IComponent* svc = services[idx];
        EXPECT_EQ(svc->GetState(), Smp::ComponentStateKind::CSK_Connected);
        svc->Publish(nullptr);
        svc->Configure(nullptr, nullptr);
        svc->Connect(nullptr);
        svc->Disconnect();
        EXPECT_NE(svc->GetUuid().Data1, 999999U);
        EXPECT_EQ(svc->GetField("missing"), nullptr);
        EXPECT_EQ(svc->GetFields(), nullptr);
        EXPECT_EQ(static_cast<int>(svc->GetSimpleValue("missing").type),
                  static_cast<int>(Smp::PrimitiveTypeKind::PTK_None));
        svc->SetSimpleValue("missing", Smp::AnySimple{});
        svc->GetSimpleArrayValue("missing", 0U, nullptr, 0U);
        svc->SetSimpleArrayValue("missing", 0U, nullptr, 0U);
        EXPECT_FALSE(svc->AddChild(nullptr, nullptr));
        EXPECT_FALSE(svc->RemoveChild(nullptr, nullptr));
        EXPECT_EQ(svc->IsChildInCollection(nullptr, nullptr), nullptr);
        EXPECT_EQ(svc->GetChild(nullptr), nullptr);
    }
}

/**
 * @brief Exhaustively tests all predefined SMP log kinds and unknown/null inputs on Logger.
 * @task TSK-20261113-004
 * @feature FE-0070.1
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.3, CS-0100.4
 * @compliance Covers all 5 predefined SMP log kinds plus unknown sentinel and null pointers.
 */
TEST(UtilsCoverageTest, LoggerAllMessageKindsAndNullGuards) {
    quasar::test_support::TestScopeGuard guard{"UtilsCoverageTest.LoggerAllMessageKindsAndNullGuards"};
    utils::Logger logger{};

    const std::array<const char*, 7U> names{
        Smp::Services::ILogger::LMK_InformationName,
        Smp::Services::ILogger::LMK_EventName,
        Smp::Services::ILogger::LMK_WarningName,
        Smp::Services::ILogger::LMK_ErrorName,
        Smp::Services::ILogger::LMK_DebugName,
        "NonExistentKind",
        nullptr};

    const std::array<Smp::Services::LogMessageKind, 7U> expected{
        Smp::Services::ILogger::LMK_Information,
        Smp::Services::ILogger::LMK_Event,
        Smp::Services::ILogger::LMK_Warning,
        Smp::Services::ILogger::LMK_Error,
        Smp::Services::ILogger::LMK_Debug,
        -1,
        -1};

    for (std::size_t idx = 0U; idx < names.size(); ++idx) {
        const Smp::Services::LogMessageKind actual = logger.QueryLogMessageKind(names[idx]);
        EXPECT_EQ(actual, expected[idx]);
        logger.Log(nullptr, "Direct kind test", actual);
    }

    // Verify null message pointer returns safely without throwing or crashing.
    logger.Log(&logger, nullptr, Smp::Services::ILogger::LMK_Information);
}

/**
 * @brief Verifies all EventManager exception sources, predefined events, and synchronous flags.
 * @task TSK-20261113-004
 * @feature FE-0070.4
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.4, CS-0100.9
 * @compliance Exercises every throw path (`InvalidEventName`, `invalid_argument`, `EntryPointAlreadySubscribed`, `EntryPointNotSubscribed`).
 */
TEST(UtilsCoverageTest, EventManagerAllExceptionSourcesAndBranches) {
    quasar::test_support::TestScopeGuard guard{"UtilsCoverageTest.EventManagerAllExceptionSourcesAndBranches"};
    utils::EventManager manager{};
    std::atomic<std::size_t> calls{0U};
    CountingEntryPoint ep{"ep_exc", calls};

    // Exception 1: null event name.
    EXPECT_THROW((void)manager.QueryEventId(nullptr), core::InvalidEventName);

    // Predefined event ID lookup consistency.
    const Smp::Services::EventId predef_id =
        manager.QueryEventId(Smp::Services::IEventManager::SMP_EnterExecuting);
    EXPECT_EQ(predef_id, Smp::Services::IEventManager::SMP_EnterExecutingId);

    // Dynamic event ID stability on repeated query.
    const Smp::Services::EventId dyn_id1 = manager.QueryEventId("CustomDynEvent");
    const Smp::Services::EventId dyn_id2 = manager.QueryEventId("CustomDynEvent");
    EXPECT_EQ(dyn_id1, dyn_id2);
    EXPECT_GE(dyn_id1, 20);

    // Exception 2: null entry point on Subscribe and Unsubscribe.
    EXPECT_THROW(manager.Subscribe(dyn_id1, nullptr), std::invalid_argument);
    EXPECT_THROW(manager.Unsubscribe(dyn_id1, nullptr), std::invalid_argument);

    // Exception 3: duplicate subscription.
    manager.Subscribe(dyn_id1, &ep);
    EXPECT_THROW(manager.Subscribe(dyn_id1, &ep), core::EntryPointAlreadySubscribed);

    // Emit with synchronous = false and synchronous = true.
    manager.Emit(dyn_id1, false);
    manager.Emit(dyn_id1, true);
    EXPECT_EQ(calls.load(std::memory_order_relaxed), 2U);

    // Emit on unknown event ID safely no-ops.
    manager.Emit(999999, true);

    // Exception 4: unsubscribing when not subscribed (both valid and unknown event IDs).
    manager.Unsubscribe(dyn_id1, &ep);
    EXPECT_THROW(manager.Unsubscribe(dyn_id1, &ep), core::EntryPointNotSubscribed);
    EXPECT_THROW(manager.Unsubscribe(888888, &ep), core::EntryPointNotSubscribed);
}

/**
 * @brief Verifies TimeKeeper boundaries and GetMissionTime nested-lock timeout containment.
 * @task TSK-20261113-004
 * @feature FE-0070.2
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.6, CS-0100.8, CS-0100.9
 * @compliance Tests negative/positive time offsets and confirms GetMissionTime throws cleanly on 1s lock timeout rather than deadlocking.
 */
TEST(UtilsCoverageTest, TimeKeeperBoundariesAndTimeoutContainment) {
    quasar::test_support::TestScopeGuard guard{"UtilsCoverageTest.TimeKeeperBoundariesAndTimeoutContainment"};
    utils::TimeKeeper tk{};

    tk.SetSimulationTime(0LL);
    tk.SetEpochTime(-1000LL);
    EXPECT_EQ(tk.GetEpochTime(), -1000LL);

    tk.SetSimulationTime(500LL);
    EXPECT_EQ(tk.GetEpochTime(), -500LL);

    tk.SetMissionStartTime(100LL);
    EXPECT_EQ(tk.GetMissionStartTime(), 100LL);

    const std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
    // Documented limitation in TimeKeeper.hpp: GetMissionTime calls GetEpochTime() while holding non-recursive _mutex.
    EXPECT_THROW((void)tk.GetMissionTime(), std::runtime_error);
    const std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    quasar::test_support::AsyncTestLogger::instance().logStepDuration("GetMissionTime_timeout", t0, t1);
    EXPECT_LT(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count(),
              UtilsTestConstants::MAX_TEST_BUDGET.count());
}

// ---------------------------------------------------------------------------
// Deterministic Fuzzing, Concurrency & Stress Tests (CS-0100.7, .10, .11, .12, .13)
// ---------------------------------------------------------------------------

/**
 * @brief Deterministic seeded fuzzing of EventManager names and Logger strings.
 * @task TSK-20261113-004
 * @feature FE-0070
 * @constraint CS-0010.37, CS-0060.7, CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.10, CS-0100.12
 * @compliance Uses fixed-seed DeterministicRng and bounded iteration loop.
 */
TEST(UtilsFuzzTest, DeterministicSeededEventAndLogFuzzing) {
    quasar::test_support::TestScopeGuard guard{"UtilsFuzzTest.DeterministicSeededEventAndLogFuzzing"};
    quasar::test_support::DeterministicRng rng{0x123456789ABCDEF0ULL};
    utils::EventManager manager{};
    utils::Logger logger{};

    std::vector<Smp::Services::EventId> observed_ids{};
    for (std::size_t idx = 0U; idx < UtilsTestConstants::MAX_FUZZ_SAMPLES; ++idx) {
        const std::size_t len = static_cast<std::size_t>(rng.nextInt(1, 32));
        const std::string name = rng.nextAsciiString(len);
        const Smp::Services::EventId id_first = manager.QueryEventId(name.c_str());
        const Smp::Services::EventId id_second = manager.QueryEventId(name.c_str());
        EXPECT_EQ(id_first, id_second);
        observed_ids.push_back(id_first);
        logger.Log(nullptr, name.c_str(), static_cast<Smp::Services::LogMessageKind>(rng.nextInt(-2, 6)));
        quasar::test_support::AsyncTestLogger::instance().logLoopProgress(
            "fuzz_loop", idx, UtilsTestConstants::MAX_FUZZ_SAMPLES);
    }
    EXPECT_EQ(observed_ids.size(), UtilsTestConstants::MAX_FUZZ_SAMPLES);
}

/**
 * @brief Concurrent multi-thread access, creation, destruction, and stress across utils services.
 * @task TSK-20261113-004
 * @feature FE-0070
 * @constraint CS-0010.37, CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.7, CS-0100.11, CS-0100.13
 * @compliance Verifies concurrent QueryEventId/Subscribe/Emit/Unsubscribe, Logger, and TimeKeeper under bounded runtime.
 */
TEST(UtilsConcurrencyStressTest, ParallelAccessAndLifecycleChurn) {
    quasar::test_support::TestScopeGuard guard{"UtilsConcurrencyStressTest.ParallelAccessAndLifecycleChurn"};
    utils::EventManager manager{};
    utils::Logger logger{};
    utils::TimeKeeper tk{};
    tk.SetEventManager(&manager);

    const Smp::Services::EventId shared_evt = manager.QueryEventId("ConcurrentSharedEvent");
    std::atomic<std::size_t> total_execs{0U};
    std::vector<std::thread> workers{};

    for (std::size_t t_idx = 0U; t_idx < UtilsTestConstants::CONCURRENT_THREADS; ++t_idx) {
        workers.emplace_back([&manager, &logger, &tk, &total_execs, shared_evt, t_idx]() {
            const std::string ep_name = "ep_" + std::to_string(t_idx);
            CountingEntryPoint local_ep{ep_name.c_str(), total_execs};
            manager.Subscribe(shared_evt, &local_ep);
            for (std::size_t iter = 0U; iter < UtilsTestConstants::MAX_STRESS_ITERATIONS; ++iter) {
                manager.Emit(shared_evt, true);
                tk.SetSimulationTime(static_cast<Smp::Duration>(iter));
                (void)tk.GetSimulationTime();
                (void)tk.GetEpochTime();
                logger.Log(&tk, "Concurrent stress", Smp::Services::ILogger::LMK_Debug);
            }
            manager.Unsubscribe(shared_evt, &local_ep);
        });
    }

    for (std::size_t idx = 0U; idx < workers.size(); ++idx) {
        workers[idx].join();
    }

    EXPECT_GE(total_execs.load(std::memory_order_relaxed),
              UtilsTestConstants::MAX_STRESS_ITERATIONS);
}

} // namespace utils::test
