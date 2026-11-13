# Test Case: `test-inverter`

| Property | Value |
| :--- | :--- |
| **Module** | `sim-test` |
| **Source File** | `cmake-projects/sim-test/src/test_inverter.cpp` (lines 1–103) |
| **Category** | Cross-Module / Third-Party Integration (`CS-0100.16`–`CS-0100.18`) |
| **Traceability** | `TSK-20261113-003`, `FE-0020`, `CS-0010,`, `CS-0010.15,` |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Entry point validating boolean inversion on `sample-inverter`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "Exit" as SUT0
participant "Contract" as SUT1
participant "LoadLibrary" as SUT2
participant "Smp" as SUT3
loop Bounded test iteration / stress window
Runner -> SUT0 : * and call simulator.Exit() .
SUT0 --> Runner : return / state updated
Runner -> SUT1 : * Specific Values: 0 (all 5 inversion checks passed) 1 (missing i...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : * - Hardware / Bus / I/O ( LoadLibrary failure): Caught by catch ...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : * - Validation / Contract (null fields/entry point or non-inverte...
SUT3 --> Runner : return / state updated
Runner -> SUT0 : * Limitations: Non- std::exception throws are unhandled simulator...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : Smp::ISimpleField* input = dynamic_cast<Smp::ISimpleField*>(compo...
SUT1 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
@enduml
```

## Observed Operations

- `* and call `simulator.Exit()`.`
- `*   Specific Values: 0 (all 5 inversion checks passed); 1 (missing interface, inversion failure, or caught exception).`
- `*   - Hardware / Bus / I/O (`LoadLibrary` failure): Caught by `catch (const std::exception&)`; logs error and returns 1.`
- `*   - Validation / Contract (null fields/entry point or non-inverted output): Logs error and returns 1.`
- `*   Limitations: Non-`std::exception` throws are unhandled; `simulator.Exit()` is skipped on early failure returns.`
- `Smp::ISimpleField* input = dynamic_cast<Smp::ISimpleField*>(component->GetField("Input"))`
- `Smp::ISimpleField* output = dynamic_cast<Smp::ISimpleField*>(component->GetField("Output"))`
- `Smp::IEntryPoint* ep = dynamic_cast<Smp::IEntryPoint*>(component->GetChild("Execute"))`

## Verification Assertions

- Verified via exit status `0` and runtime checks inside executable.

---

[Back to `sim-test` Tests Overview](../sim-test.md)
