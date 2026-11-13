# Utils Tests

This document describes the tests for the `utils` module, which provides common simulation services.

## Unit Tests

### test_logger
Verifies that the `Logger` correctly outputs messages to the console with appropriate prefixes and level markers.

### test_timekeeper
Tests the `TimeKeeper`'s ability to track simulation time and perform conversions between various time references (Epoch, Mission, Zulu).

### test_event_manager
Verifies the `EventManager`'s subscribe/emit logic, ensuring that all registered entry points are called when an event is triggered.

<!-- CS-0100-CATALOGUE-START -->
## Registered Test Case Pages (`CS-0100.22`)

| Test Case | Documentation Page |
| :--- | :--- |
| `UtilsTutorialTest.Tutorial1_SimpleEventManagerFlow` | [UtilsTutorialTest.Tutorial1_SimpleEventManagerFlow](utils/UtilsTutorialTest.Tutorial1_SimpleEventManagerFlow.md) |
| `UtilsTutorialTest.Tutorial2_IntermediateTimeKeeperWithEvents` | [UtilsTutorialTest.Tutorial2_IntermediateTimeKeeperWithEvents](utils/UtilsTutorialTest.Tutorial2_IntermediateTimeKeeperWithEvents.md) |
| `UtilsTutorialTest.Tutorial3_AdvancedMultiServiceComposition` | [UtilsTutorialTest.Tutorial3_AdvancedMultiServiceComposition](utils/UtilsTutorialTest.Tutorial3_AdvancedMultiServiceComposition.md) |
| `UtilsCoverageTest.AllComponentStubsAndMetadata` | [UtilsCoverageTest.AllComponentStubsAndMetadata](utils/UtilsCoverageTest.AllComponentStubsAndMetadata.md) |
| `UtilsCoverageTest.LoggerAllMessageKindsAndNullGuards` | [UtilsCoverageTest.LoggerAllMessageKindsAndNullGuards](utils/UtilsCoverageTest.LoggerAllMessageKindsAndNullGuards.md) |
| `UtilsCoverageTest.EventManagerAllExceptionSourcesAndBranches` | [UtilsCoverageTest.EventManagerAllExceptionSourcesAndBranches](utils/UtilsCoverageTest.EventManagerAllExceptionSourcesAndBranches.md) |
| `UtilsCoverageTest.TimeKeeperBoundariesAndTimeoutContainment` | [UtilsCoverageTest.TimeKeeperBoundariesAndTimeoutContainment](utils/UtilsCoverageTest.TimeKeeperBoundariesAndTimeoutContainment.md) |
| `UtilsFuzzTest.DeterministicSeededEventAndLogFuzzing` | [UtilsFuzzTest.DeterministicSeededEventAndLogFuzzing](utils/UtilsFuzzTest.DeterministicSeededEventAndLogFuzzing.md) |
| `UtilsConcurrencyStressTest.ParallelAccessAndLifecycleChurn` | [UtilsConcurrencyStressTest.ParallelAccessAndLifecycleChurn](utils/UtilsConcurrencyStressTest.ParallelAccessAndLifecycleChurn.md) |
