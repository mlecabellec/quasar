# Sim Tests

This document describes the tests for the `sim` module, which implements the core simulation engine.

## test_basic_simulation

### Description
The `test_basic_simulation` executable is a comprehensive integration test that exercises the entire simulator lifecycle.

### Scenario
1.  **Setup**: Creates a `Simulator` instance and registers mock factories.
2.  **Execution**:
    -   Calls `Publish()`, `Configure()`, `Connect()`.
    -   Transitions to `Initialise()`.
    -   Schedules several events (immediate and timed).
    -   Calls `Run(duration)` to process the events.
3.  **Verification**:
    -   Asserts that all components transitioned through the correct states.
    -   Verifies that entry points were executed in the expected order and at the correct simulation times.
    -   Checks that the simulator state is `Executing` after initialization and `Ready` if stopped.
