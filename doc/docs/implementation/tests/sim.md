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

<!-- CS-0100-CATALOGUE-START -->
## Registered Test Case Pages (`CS-0100.22`)

| Test Case | Documentation Page |
| :--- | :--- |
| `SimTutorialTest.Tutorial1_BasicSimulatorLifecycle` | [SimTutorialTest.Tutorial1_BasicSimulatorLifecycle](sim/SimTutorialTest.Tutorial1_BasicSimulatorLifecycle.md) |
| `SimTutorialTest.Tutorial2_IntermediateTypeRegistryAndPublication` | [SimTutorialTest.Tutorial2_IntermediateTypeRegistryAndPublication](sim/SimTutorialTest.Tutorial2_IntermediateTypeRegistryAndPublication.md) |
| `SimTutorialTest.Tutorial3_AdvancedModelResolverLinksAndTimedRun` | [SimTutorialTest.Tutorial3_AdvancedModelResolverLinksAndTimedRun](sim/SimTutorialTest.Tutorial3_AdvancedModelResolverLinksAndTimedRun.md) |
| `SimCoverageTest.TwentyPlusTypesFieldsAndEnumStreams` | [SimCoverageTest.TwentyPlusTypesFieldsAndEnumStreams](sim/SimCoverageTest.TwentyPlusTypesFieldsAndEnumStreams.md) |
| `SimCoverageTest.ExceptionsAndBoundaryStates` | [SimCoverageTest.ExceptionsAndBoundaryStates](sim/SimCoverageTest.ExceptionsAndBoundaryStates.md) |
| `SimFuzzStressTest.DeterministicFieldAndResolverFuzzing` | [SimFuzzStressTest.DeterministicFieldAndResolverFuzzing](sim/SimFuzzStressTest.DeterministicFieldAndResolverFuzzing.md) |
| `SimFuzzStressTest.ConcurrentLinkRegistryStress` | [SimFuzzStressTest.ConcurrentLinkRegistryStress](sim/SimFuzzStressTest.ConcurrentLinkRegistryStress.md) |
| `test_basic_simulation` | [test_basic_simulation](sim/test_basic_simulation.md) |
