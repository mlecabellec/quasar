# Utils Tests

This document describes the tests for the `utils` module, which provides common simulation services.

## Unit Tests

### test_logger
Verifies that the `Logger` correctly outputs messages to the console with appropriate prefixes and level markers.

### test_timekeeper
Tests the `TimeKeeper`'s ability to track simulation time and perform conversions between various time references (Epoch, Mission, Zulu).

### test_event_manager
Verifies the `EventManager`'s subscribe/emit logic, ensuring that all registered entry points are called when an event is triggered.
