# Datalogger Classes

This directory contains the implementation details of the data logging and acquisition module.

## Core Services
- [DataLoggerService](DataLoggerService.md): Central orchestrator and background service.
- [RingBuffer](RingBuffer.md): Thread-safe queue for high-throughput log ingestion.

## Data Structures
- [LogEntry](LogEntry.md): Unified record for events and data samples.

## Interfaces
- [Interfaces](Interfaces.md): Definitions for `IRecorder` and `IFilter`.
- [DataAccess](DataAccess.md): Definitions for `IDataAccessor` and `DataView`.

## Implementations
- [ADevRecorder](ADevRecorder.md): Base class for hierarchical recorders.
- [CsvFileWriter](CsvFileWriter.md): Double-buffered CSV storage backend.
- [MathFilter](MathFilter.md): Linear transformation filter.
- [ValueThresholdTrigger](ValueThresholdTrigger.md): Conditional gating filter.
