/**
 * @file TestScheduler.cpp
 * @brief Comprehensive CS-0100 test suite for `sched::Scheduler`.
 *
 * @details Covers all `sched::Scheduler` scheduling domains (immediate, simulation,
 * mission, epoch, placeholder Zulu), rescheduling, removal, repeat counts, exception
 * sources, concurrent schedule/execute contention, stress, seeded fuzzing, and 3-tier
 * progressive tutorials.
 *
 * @task TSK-20261113-004
 * @feature FE-0070.3, FE-0080
 * @constraint CS-0010.26, CS-0010.34, CS-0010.37, CS-0060.3, CS-0090.1, CS-0090.2, CS-0100.1, CS-0100.2, CS-0100.3, CS-0100.6, CS-0100.7, CS-0100.8, CS-0100.9, CS-0100.10, CS-0100.11, CS-0100.12, CS-0100.13, CS-0100.19, CS-0100.20, CS-0100.21
 * @compliance Uses explicit types, bounded iterations, deterministic seeded PRNG, and
 * non-blocking async test telemetry.
 */

#include <gtest/gtest.h>

#include "quasar/test_support/AsyncTestLogger.hpp"
#include "sched/Scheduler.hpp"
#include "utils/Logger.hpp"
#include "utils/TimeKeeper.hpp"

#include <Smp/IEntryPoint.h>
#include <Smp/Services/ITimeKeeper.h>
#include <core/Object.hpp>
#include <core/StandardExceptions.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <thread>
#include <vector>

namespace sched::test {

/**
 * @brief Symbolic constants for scheduler tests [CS-0040.2].
 * @task TSK-20261113-004
 * @feature FE-0070.3
 * @constraint CS-0040.2, CS-0040.6, CS-0090.1, CS-0090.2
 * @compliance Defines UPPER_SNAKE_CASE bounds and durations.
 */
struct SchedTestConstants {
    static constexpr std::size_t MAX_EVENTS_STRESS = 300U;
    static constexpr std::size_t MAX_FUZZ_EVENTS = 60U;
    static constexpr std::size_t MAX_CONCURRENT_OPS = 150U;
    static constexpr Smp::Duration CYCLE_NS = 100LL;
    static constexpr std::chrono::milliseconds MAX_EXEC_BUDGET{1500};
};

/**
 * @brief Entry point stub tracking execution count and execution order timestamps.
 * @task TSK-20261113-004
 * @feature FE-0070.3
 * @constraint CS-0090.1, CS-0090.2
 * @compliance Implements `Smp::IEntryPoint` cleanly without raw allocations.
 */
class RecordingEntryPoint : public core::Object, public virtual Smp::IEntryPoint {
public:
    RecordingEntryPoint(Smp::String8 name, std::atomic<std::size_t>& counter)
        : core::Object(name, "Recording entry point", nullptr), counter_(counter) {}

    void Execute() const override {
        counter_.fetch_add(1U, std::memory_order_relaxed);
    }

private:
    std::atomic<std::size_t>& counter_;
};

/**
 * @brief Deterministic mock `ITimeKeeper` avoiding `utils::TimeKeeper::GetMissionTime` lock bug.
 * @task TSK-20261113-004
 * @feature FE-0070.2, FE-0070.3
 * @constraint CS-0090.1, CS-0090.2
 * @compliance Enables deterministic unit testing of `AddMissionTimeEvent` and `AddEpochTimeEvent`.
 */
class MockTimeKeeper : public core::Object, public virtual Smp::Services::ITimeKeeper {
public:
    MockTimeKeeper() : core::Object("MockTimeKeeper", "Mock TK", nullptr) {}

    Smp::ComponentStateKind GetState() const override { return Smp::ComponentStateKind::CSK_Connected; }
    void Publish(Smp::IPublication*) override {}
    void Configure(Smp::Services::ILogger*, Smp::Services::ILinkRegistry*) override {}
    void Connect(Smp::ISimulator*) override {}
    void Disconnect() override {}
    const Smp::Uuid& GetUuid() const override {
        static Smp::Uuid uuid{0, 0, 0, 0, 99};
        return uuid;
    }
    Smp::IField* GetField(Smp::String8) const override { return nullptr; }
    const Smp::FieldCollection* GetFields() const override { return nullptr; }
    Smp::AnySimple GetSimpleValue(Smp::String8) const override { return Smp::AnySimple{}; }
    void SetSimpleValue(Smp::String8, Smp::AnySimple) override {}
    void GetSimpleArrayValue(Smp::String8, Smp::UInt64, Smp::AnySimple*, Smp::UInt64) const override {}
    void SetSimpleArrayValue(Smp::String8, Smp::UInt64, Smp::AnySimpleArray, Smp::UInt64) override {}
    Smp::Bool AddChild(Smp::IObject*, const Smp::ICollectionBase*) override { return false; }
    Smp::Bool RemoveChild(Smp::IObject*, const Smp::ICollectionBase*) override { return false; }
    Smp::IObject* IsChildInCollection(Smp::String8, const Smp::ICollectionBase*) const override { return nullptr; }
    Smp::IObject* GetChild(Smp::String8) const override { return nullptr; }

    Smp::Duration GetSimulationTime() const override { return sim_time_; }
    Smp::DateTime GetEpochTime() const override { return epoch_offset_ + sim_time_; }
    Smp::DateTime GetMissionStartTime() const override { return mission_start_; }
    Smp::Duration GetMissionTime() const override { return (epoch_offset_ + sim_time_) - mission_start_; }
    Smp::DateTime GetZuluTime() const override { return 1000000LL; }
    void SetEpochTime(Smp::DateTime epochTime) override { epoch_offset_ = epochTime - sim_time_; }
    void SetMissionStartTime(Smp::DateTime missionStart) override { mission_start_ = missionStart; }
    void SetMissionTime(Smp::Duration missionTime) override { mission_start_ = GetEpochTime() - missionTime; }
    void SetSimulationTime(Smp::Duration simulationTime) override { sim_time_ = simulationTime; }

private:
    Smp::Duration sim_time_{0LL};
    Smp::DateTime epoch_offset_{1000LL};
    Smp::DateTime mission_start_{500LL};
};

// ---------------------------------------------------------------------------
// 3-Tier Progressive Tutorials (CS-0100.19)
// ---------------------------------------------------------------------------

/**
 * @brief Tutorial 1 (Simple): Schedule and execute a one-shot simulation-time event.
 * @task TSK-20261113-004
 * @feature FE-0070.3
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.19, CS-0100.20
 * @compliance Demonstrates constructing Scheduler and dispatching a single event.
 */
TEST(SchedulerTutorialTest, Tutorial1_SimpleOneShotEvent) {
    quasar::test_support::TestScopeGuard guard{"SchedulerTutorialTest.Tutorial1_SimpleOneShotEvent"};
    utils::Logger logger{};
    MockTimeKeeper tk{};
    sched::Scheduler scheduler{&tk, &logger};
    std::atomic<std::size_t> count{0U};
    RecordingEntryPoint ep{"ep_one_shot", count};

    const Smp::Services::EventId id = scheduler.AddSimulationTimeEvent(&ep, 200LL, 0LL, 0LL);
    EXPECT_TRUE(scheduler.HasEvents());
    EXPECT_TRUE(scheduler.IsEventScheduled(id));
    EXPECT_EQ(scheduler.GetNextScheduledEventTime(), 200LL);

    const Smp::Duration executed_at = scheduler.ExecuteNextEvent();
    EXPECT_EQ(executed_at, 200LL);
    EXPECT_EQ(count.load(std::memory_order_relaxed), 1U);
    EXPECT_FALSE(scheduler.HasEvents());
    EXPECT_FALSE(scheduler.IsEventScheduled(id));
}

/**
 * @brief Tutorial 2 (Intermediate): Cyclic events, immediate events, and rescheduling.
 * @task TSK-20261113-004
 * @feature FE-0070.3
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.19, CS-0100.20
 * @compliance Demonstrates immediate priority over timeline and finite repeat cycles.
 */
TEST(SchedulerTutorialTest, Tutorial2_IntermediateCyclicAndImmediate) {
    quasar::test_support::TestScopeGuard guard{"SchedulerTutorialTest.Tutorial2_IntermediateCyclicAndImmediate"};
    utils::Logger logger{};
    MockTimeKeeper tk{};
    sched::Scheduler scheduler{&tk, &logger};
    std::atomic<std::size_t> imm_hits{0U};
    std::atomic<std::size_t> cyc_hits{0U};
    RecordingEntryPoint ep_imm{"ep_imm", imm_hits};
    RecordingEntryPoint ep_cyc{"ep_cyc", cyc_hits};

    (void)scheduler.AddImmediateEvent(&ep_imm);
    const Smp::Services::EventId cyc_id =
        scheduler.AddSimulationTimeEvent(&ep_cyc, 50LL, SchedTestConstants::CYCLE_NS, 2LL);

    // Immediate executes first at current sim time (0).
    EXPECT_EQ(scheduler.ExecuteNextEvent(), 0LL);
    EXPECT_EQ(imm_hits.load(std::memory_order_relaxed), 1U);

    // Cyclic executes at 50, 150, 250 (initial + 2 repeats = 3 executions).
    EXPECT_EQ(scheduler.ExecuteNextEvent(), 50LL);
    EXPECT_EQ(scheduler.ExecuteNextEvent(), 150LL);
    EXPECT_EQ(scheduler.ExecuteNextEvent(), 250LL);
    EXPECT_EQ(cyc_hits.load(std::memory_order_relaxed), 3U);
    EXPECT_FALSE(scheduler.IsEventScheduled(cyc_id));
}

/**
 * @brief Tutorial 3 (Advanced): Multi-domain scheduling (Mission/Epoch) and cancellation.
 * @task TSK-20261113-004
 * @feature FE-0070.3
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.19, CS-0100.20
 * @compliance Exercises mission-time and epoch-time conversion plus dynamic removal.
 */
TEST(SchedulerTutorialTest, Tutorial3_AdvancedMultiDomainAndCancel) {
    quasar::test_support::TestScopeGuard guard{"SchedulerTutorialTest.Tutorial3_AdvancedMultiDomainAndCancel"};
    utils::Logger logger{};
    MockTimeKeeper tk{};
    sched::Scheduler scheduler{&tk, &logger};
    std::atomic<std::size_t> hits{0U};
    RecordingEntryPoint ep{"ep_domain", hits};

    // MockTK: sim=0, epoch_offset=1000, mission_start=500 -> current mission=500, epoch=1000.
    const Smp::Services::EventId mission_id = scheduler.AddMissionTimeEvent(&ep, 700LL, 0LL, 0LL);
    const Smp::Services::EventId epoch_id = scheduler.AddEpochTimeEvent(&ep, 1300LL, 0LL, 0LL);

    // Cancel epoch event before it fires.
    scheduler.RemoveEvent(epoch_id);
    EXPECT_FALSE(scheduler.IsEventScheduled(epoch_id));
    EXPECT_TRUE(scheduler.IsEventScheduled(mission_id));

    const Smp::Duration fired_time = scheduler.ExecuteNextEvent();
    EXPECT_EQ(fired_time, 200LL);
    EXPECT_EQ(hits.load(std::memory_order_relaxed), 1U);
}

// ---------------------------------------------------------------------------
// Exhaustive Method, Exception, Boundary & Timing Tests (CS-0100.1, .3, .6, .8, .9)
// ---------------------------------------------------------------------------

/**
 * @brief Covers all Scheduler exception sources, boundary inputs, and placeholder methods.
 * @task TSK-20261113-004
 * @feature FE-0070.3
 * @constraint CS-0090.1, CS-0090.2, CS-0100.1, CS-0100.2, CS-0100.6, CS-0100.9
 * @compliance Exercises constructor null checks, negative times, zero cycleTimes, invalid IDs, and domain mismatches.
 */
TEST(SchedulerCoverageTest, AllExceptionsBoundariesAndStubs) {
    quasar::test_support::TestScopeGuard guard{"SchedulerCoverageTest.AllExceptionsBoundariesAndStubs"};
    utils::Logger logger{};
    MockTimeKeeper tk{};

    // Constructor null dependencies.
    EXPECT_THROW((sched::Scheduler{nullptr, &logger}), std::invalid_argument);
    EXPECT_THROW((sched::Scheduler{&tk, nullptr}), std::invalid_argument);

    sched::Scheduler scheduler{&tk, &logger};
    std::atomic<std::size_t> hits{0U};
    RecordingEntryPoint ep{"ep_cov", hits};

    // Component interface stubs.
    EXPECT_EQ(scheduler.GetState(), Smp::ComponentStateKind::CSK_Connected);
    scheduler.Publish(nullptr);
    scheduler.Configure(nullptr, nullptr);
    scheduler.Connect(nullptr);
    scheduler.Disconnect();
    EXPECT_EQ(scheduler.GetUuid().Data1, 0U);
    EXPECT_EQ(scheduler.GetField("f"), nullptr);
    EXPECT_EQ(scheduler.GetFields(), nullptr);
    EXPECT_EQ(static_cast<int>(scheduler.GetSimpleValue("f").type),
              static_cast<int>(Smp::PrimitiveTypeKind::PTK_None));
    scheduler.SetSimpleValue("f", Smp::AnySimple{});
    scheduler.GetSimpleArrayValue("f", 0U, nullptr, 0U);
    scheduler.SetSimpleArrayValue("f", 0U, nullptr, 0U);
    EXPECT_FALSE(scheduler.AddChild(nullptr, nullptr));
    EXPECT_FALSE(scheduler.RemoveChild(nullptr, nullptr));
    EXPECT_EQ(scheduler.IsChildInCollection(nullptr, nullptr), nullptr);
    EXPECT_EQ(scheduler.GetChild(nullptr), nullptr);

    // Empty timeline returns max() and -1 on ExecuteNextEvent.
    EXPECT_EQ(scheduler.GetCurrentEventId(), -1);
    EXPECT_EQ(scheduler.GetNextScheduledEventTime(), std::numeric_limits<Smp::Duration>::max());
    EXPECT_EQ(scheduler.ExecuteNextEvent(), -1);

    // AddImmediateEvent null check.
    EXPECT_THROW((void)scheduler.AddImmediateEvent(nullptr), std::invalid_argument);

    // AddSimulationTimeEvent validation checks.
    EXPECT_THROW((void)scheduler.AddSimulationTimeEvent(nullptr, 0LL, 0LL, 0LL), std::invalid_argument);
    EXPECT_THROW((void)scheduler.AddSimulationTimeEvent(&ep, -1LL, 0LL, 0LL), core::InvalidEventTime);
    EXPECT_THROW((void)scheduler.AddSimulationTimeEvent(&ep, 0LL, 0LL, 1LL), core::InvalidCycleTime);

    // AddMissionTimeEvent validation checks.
    EXPECT_THROW((void)scheduler.AddMissionTimeEvent(nullptr, 500LL, 0LL, 0LL), std::invalid_argument);
    EXPECT_THROW((void)scheduler.AddMissionTimeEvent(&ep, 499LL, 0LL, 0LL), core::InvalidEventTime);
    EXPECT_THROW((void)scheduler.AddMissionTimeEvent(&ep, 500LL, 0LL, 1LL), core::InvalidCycleTime);

    // AddEpochTimeEvent validation checks.
    EXPECT_THROW((void)scheduler.AddEpochTimeEvent(nullptr, 1000LL, 0LL, 0LL), std::invalid_argument);
    EXPECT_THROW((void)scheduler.AddEpochTimeEvent(&ep, 999LL, 0LL, 0LL), core::InvalidEventTime);
    EXPECT_THROW((void)scheduler.AddEpochTimeEvent(&ep, 1000LL, 0LL, 1LL), core::InvalidCycleTime);

    // Placeholder Zulu methods return -1 cleanly.
    EXPECT_EQ(scheduler.AddZuluTimeEvent(&ep, 0LL, 0LL, 0LL), -1);
    EXPECT_EQ(scheduler.AddRelativeZuluTimeEvent(&ep, 0LL, 0LL, 0LL), -1);

    // SetEventSimulationTime rescheduling and negative-time deletion.
    const Smp::Services::EventId sim_id = scheduler.AddSimulationTimeEvent(&ep, 10LL, 0LL, 0LL);
    scheduler.SetEventSimulationTime(sim_id, 50LL);
    EXPECT_EQ(scheduler.GetNextScheduledEventTime(), 50LL);
    scheduler.SetEventSimulationTime(sim_id, -1LL);
    EXPECT_FALSE(scheduler.IsEventScheduled(sim_id));

    // InvalidEventId checks for RemoveEvent & SetEventSimulationTime.
    EXPECT_THROW(scheduler.RemoveEvent(999999), core::InvalidEventId);
    EXPECT_THROW(scheduler.SetEventSimulationTime(999999, 10LL), core::InvalidEventId);

    // Domain mismatch throws InvalidEventId on SetEventSimulationTime.
    const Smp::Services::EventId m_id = scheduler.AddMissionTimeEvent(&ep, 600LL, 0LL, 0LL);
    EXPECT_THROW(scheduler.SetEventSimulationTime(m_id, 20LL), core::InvalidEventId);
    scheduler.RemoveEvent(m_id);

    // Infinite repeat (< 0) reschedules until explicitly removed.
    const Smp::Services::EventId inf_id = scheduler.AddSimulationTimeEvent(&ep, 100LL, 25LL, -1LL);
    EXPECT_EQ(scheduler.ExecuteNextEvent(), 100LL);
    EXPECT_EQ(scheduler.ExecuteNextEvent(), 125LL);
    EXPECT_TRUE(scheduler.IsEventScheduled(inf_id));
    scheduler.RemoveEvent(inf_id);

    // Exercise no-op setter stubs for 100% method coverage.
    scheduler.SetEventMissionTime(0, 0LL);
    scheduler.SetEventEpochTime(0, 0LL);
    scheduler.SetEventZuluTime(0, 0LL);
    scheduler.SetEventCycleTime(0, 0LL);
    scheduler.SetEventRepeat(0, 0LL);
}

// ---------------------------------------------------------------------------
// Deterministic Fuzzing, Concurrency & Bounded-Duration Stress (CS-0100.7, .8, .10-.13)
// ---------------------------------------------------------------------------

/**
 * @brief Deterministic seeded fuzzing of event timeline ordering and bounded execution time.
 * @task TSK-20261113-004
 * @feature FE-0070.3
 * @constraint CS-0010.37, CS-0060.7, CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.8, CS-0100.10, CS-0100.12, CS-0100.13
 * @compliance Ensures events execute in strictly non-decreasing simulation timestamp order.
 */
TEST(SchedulerFuzzStressTest, DeterministicTimelineFuzzAndDurationBound) {
    quasar::test_support::TestScopeGuard guard{"SchedulerFuzzStressTest.DeterministicTimelineFuzzAndDurationBound"};
    const std::chrono::steady_clock::time_point start_tp = std::chrono::steady_clock::now();
    quasar::test_support::DeterministicRng rng{0xFEEDBEEF2026ULL};
    utils::Logger logger{};
    MockTimeKeeper tk{};
    sched::Scheduler scheduler{&tk, &logger};
    std::atomic<std::size_t> exec_count{0U};
    RecordingEntryPoint ep{"ep_fuzz", exec_count};

    for (std::size_t idx = 0U; idx < SchedTestConstants::MAX_FUZZ_EVENTS; ++idx) {
        const Smp::Duration target_t = static_cast<Smp::Duration>(rng.nextInt(0, 10000));
        (void)scheduler.AddSimulationTimeEvent(&ep, target_t, 0LL, 0LL);
    }

    Smp::Duration prev_t = -1LL;
    for (std::size_t step = 0U; step < SchedTestConstants::MAX_FUZZ_EVENTS; ++step) {
        const Smp::Duration fired_t = scheduler.ExecuteNextEvent();
        EXPECT_GE(fired_t, prev_t);
        prev_t = fired_t;
    }

    EXPECT_EQ(exec_count.load(std::memory_order_relaxed), SchedTestConstants::MAX_FUZZ_EVENTS);
    const std::chrono::steady_clock::time_point end_tp = std::chrono::steady_clock::now();
    EXPECT_LT(std::chrono::duration_cast<std::chrono::milliseconds>(end_tp - start_tp).count(),
              SchedTestConstants::MAX_EXEC_BUDGET.count());
}

/**
 * @brief Concurrent producers scheduling/cancelling while consumer drains timeline safely.
 * @task TSK-20261113-004
 * @feature FE-0070.3
 * @constraint CS-0010.37, CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.7, CS-0100.11
 * @compliance Exercises concurrent schedule, IsEventScheduled, and ExecuteNextEvent under bounded loops.
 */
TEST(SchedulerFuzzStressTest, ConcurrentScheduleCancelAndExecute) {
    quasar::test_support::TestScopeGuard guard{"SchedulerFuzzStressTest.ConcurrentScheduleCancelAndExecute"};
    utils::Logger logger{};
    MockTimeKeeper tk{};
    sched::Scheduler scheduler{&tk, &logger};
    std::atomic<std::size_t> exec_count{0U};
    RecordingEntryPoint ep{"ep_conc", exec_count};

    std::thread producer([&scheduler, &ep]() {
        for (std::size_t idx = 0U; idx < SchedTestConstants::MAX_CONCURRENT_OPS; ++idx) {
            const Smp::Services::EventId id =
                scheduler.AddSimulationTimeEvent(&ep, static_cast<Smp::Duration>(idx + 1U), 0LL, 0LL);
            (void)scheduler.IsEventScheduled(id);
            if ((idx % 3U) == 0U) {
                try {
                    scheduler.RemoveEvent(id);
                } catch (const core::InvalidEventId&) {
                    // Event already executed by consumer thread; expected race outcome.
                }
            } else {
                // Keep event for consumer execution [CS-0060.3].
            }
        }
    });

    std::thread consumer([&scheduler]() {
        for (std::size_t step = 0U; step < SchedTestConstants::MAX_EVENTS_STRESS; ++step) {
            if (scheduler.HasEvents()) {
                (void)scheduler.ExecuteNextEvent();
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    // Drain any remaining scheduled events deterministically.
    for (std::size_t drain = 0U; drain < SchedTestConstants::MAX_EVENTS_STRESS; ++drain) {
        if (!scheduler.HasEvents()) {
            break;
        } else {
            (void)scheduler.ExecuteNextEvent();
        }
    }
    EXPECT_FALSE(scheduler.HasEvents());
}

} // namespace sched::test
