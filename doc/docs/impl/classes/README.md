# Classes

This directory contains the implementation details of the classes of the Quasar project.

It is organized by namespaces and contains documentation for the core framework and its various functional modules.

## Modules

### Core Framework
- [quasar/coretypes](quasar/coretypes/README.md): Fundamental data types (Integer, Buffer, Timestamp, etc.).
- [quasar/named](quasar/named/README.md): Hierarchical object system and tree management.

### Industrial Communication
- [resoem](resoem/README.md): Clean-room EtherCAT master implementation.
- [opcua](opcua/README.md): OPC UA server and client services.

### Logic and Control
- [logicengine](logicengine/README.md): State Machines, SFC, and Cause-Effect matrix engines.
- [calibration](calibration/README.md): Bidirectional value transformation framework.

### Infrastructure Services
- [datalogger](datalogger/README.md): High-performance data acquisition and persistent logging.
- [scripting](scripting/README.md): Lua 5.4 embedded execution environment.

### External Standards
- [Smp](Smp/README.md): Implementation of ECSS-E-ST-40-07C (Simulation Model Portability).

---

## Documentation Standards
Each file associated to a class contains:
- [IMPL-CLASSES-001] The class textual description (purpose, responsibilities, invariants).
- [IMPL-CLASSES-002] List of methods with short descriptions.
- [IMPL-CLASSES-003] List of attributes.
- [IMPL-CLASSES-004] Relations to other classes.
- [IMPL-CLASSES-005] Dependencies.
- [IMPL-CLASSES-006] Test references.
- [IMPL-CLASSES-007] Usage examples.
- [IMPL-CLASSES-008] PlantUML class diagrams.
