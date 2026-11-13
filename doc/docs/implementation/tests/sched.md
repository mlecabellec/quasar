# Sched Tests

This document describes the tests for the `sched` module, which implements the simulation scheduler.

## Integration Tests

The `sched` module's functionality is primarily verified through integration tests in the `sim` project, such as `test_basic_simulation`, which exercises the scheduler's ability to dispatch events at precise simulation times.

### test_timer_precision
Verifies that events added via `AddSimulationTimeEvent` are executed at the exact time specified by the `TimeKeeper`.

### test_cyclic_events
Verifies that cyclic events are correctly re-scheduled after execution based on their cycle time and repeat count.

<!-- CS-0100-CATALOGUE-START -->
## Registered Test Case Pages (`CS-0100.22`)

| Test Case | Documentation Page |
| :--- | :--- |
| `SchedulerTutorialTest.Tutorial1_SimpleOneShotEvent` | [SchedulerTutorialTest.Tutorial1_SimpleOneShotEvent](sched/SchedulerTutorialTest.Tutorial1_SimpleOneShotEvent.md) |
| `SchedulerTutorialTest.Tutorial2_IntermediateCyclicAndImmediate` | [SchedulerTutorialTest.Tutorial2_IntermediateCyclicAndImmediate](sched/SchedulerTutorialTest.Tutorial2_IntermediateCyclicAndImmediate.md) |
| `SchedulerTutorialTest.Tutorial3_AdvancedMultiDomainAndCancel` | [SchedulerTutorialTest.Tutorial3_AdvancedMultiDomainAndCancel](sched/SchedulerTutorialTest.Tutorial3_AdvancedMultiDomainAndCancel.md) |
| `SchedulerCoverageTest.AllExceptionsBoundariesAndStubs` | [SchedulerCoverageTest.AllExceptionsBoundariesAndStubs](sched/SchedulerCoverageTest.AllExceptionsBoundariesAndStubs.md) |
| `SchedulerFuzzStressTest.DeterministicTimelineFuzzAndDurationBound` | [SchedulerFuzzStressTest.DeterministicTimelineFuzzAndDurationBound](sched/SchedulerFuzzStressTest.DeterministicTimelineFuzzAndDurationBound.md) |
| `SchedulerFuzzStressTest.ConcurrentScheduleCancelAndExecute` | [SchedulerFuzzStressTest.ConcurrentScheduleCancelAndExecute](sched/SchedulerFuzzStressTest.ConcurrentScheduleCancelAndExecute.md) |
