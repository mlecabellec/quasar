# Datalogger Module Tests

This document describes the testing suite for the `datalogger` module, located in `cmake-projects/datalogger/test/`.

## 1. Unit Tests

### Ring Buffer Tests (`TestDataloggerRingBuffer.cpp`)
- **FIFO Logic**: Verifies that elements are popped in the same order they were pushed.
- **Wrap-around**: Verifies that when the buffer is full, new elements overwrite the oldest ones.
- **Thread Safety**: Multiple producers pushing simultaneously with a single consumer popping.

### Service Tests (`TestDataLoggerService.cpp`)
- **Singleton Lifecycle**: Verifies `getInstance()`, `initDefault()`, and `resetInstance()`.
- **Pipeline Integrity**: Verifies that logs pushed to the service correctly pass through filters and reach recorders.
- **Flush Behavior**: Ensures all buffered data is persisted when `flush()` is called.

### Recorder Tests (`TestCsvFileWriter.cpp`)
- **Double Buffering**: Verifies that the front buffer is swapped to the back buffer and written by the background thread.
- **Formatting**: Validates ISO-8601 timestamping and correct CSV escaping for string payloads.

## 2. Stress and Performance (`TestDataloggerStress.cpp`)
- **High Throughput**: Sustained logging of 50,000 samples/sec for 1 minute.
- **Concurrent Access**: 10 threads logging simultaneously to the same global service.
- **Zero-Loss**: Verification that the ring buffer handles bursts without crashing or losing the most recent data (via overwriting policy).
