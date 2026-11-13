# Test Case: `SerializationTest.BinaryRoundTrip`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestSerialization.cpp` (lines 228–237) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `SerializationTest.BinaryRoundTrip` defined in `cmake-projects/named/test/TestSerialization.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "Trip" as SUT0
Runner -> SUT0 : Execute test sequence
SUT0 --> Runner : Verification result
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- Verified via exit status `0` and runtime checks inside executable.

---

[Back to `named` Tests Overview](../named.md)
