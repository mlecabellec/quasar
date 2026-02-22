# Sched Tests

This document describes the tests for the `sched` module, which implements the simulation scheduler.

## Integration Tests

The `sched` module's functionality is primarily verified through integration tests in the `sim` project, such as `test_basic_simulation`, which exercises the scheduler's ability to dispatch events at precise simulation times.

### test_timer_precision
Verifies that events added via `AddSimulationTimeEvent` are executed at the exact time specified by the `TimeKeeper`.

### test_cyclic_events
Verifies that cyclic events are correctly re-scheduled after execution based on their cycle time and repeat count.
