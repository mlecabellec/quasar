# Test Case: `test_basic_simulation`

| Property | Value |
| :--- | :--- |
| **Module** | `sim` |
| **Source File** | `cmake-projects/sim/test/test_basic_simulation.cpp` (lines 1–183) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Test file for basic simulator functionality.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "GetUuid" as SUT0
participant "Smp" as SUT1
participant "StopSimulation" as SUT2
participant "Object" as SUT3
Runner -> SUT0 : * outlined in FE-0030 (e.g., Number operations, String class, Buf...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : * Contribution to FE-0050.6 (Simulation Lifecycle):
SUT1 --> Runner : return / state updated
Runner -> SUT2 : * Contribution to FE-0050.4 (UUIDs):
SUT2 --> Runner : return / state updated
Runner -> SUT3 : * Contribution to FE-0050.1 (Primitive Types):
SUT3 --> Runner : return / state updated
Runner -> SUT0 : _simulator->Hold(true) // FE-0050.6.1 Example of transitioning si...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : sim::Model::Publish(receiver) // FE-0050.6.1 Component state tran...
SUT1 --> Runner : return / state updated
alt Exception / boundary / conditional branch check
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
@enduml
```

## Observed Operations

- `*   outlined in FE-0030 (e.g., Number operations, String class, Buffer/BitBuffer`
- `* Contribution to FE-0050.6 (Simulation Lifecycle):`
- `* Contribution to FE-0050.4 (UUIDs):`
- `* Contribution to FE-0050.1 (Primitive Types):`
- `_simulator->Hold(true); // [FE-0050.6.1] Example of transitioning simulation state (Executing -> Standby via Hold).`
- `sim::Model::Publish(receiver); // [FE-0050.6.1] Component state transition: Created -> Publishing.`
- `sim::Model::Configure(logger, linkRegistry); // [FE-0050.6.1] Component state transition: Publishing -> Configured.`
- `sim::Model::Connect(simulator); // [FE-0050.6.1] Component state transition: Configured -> Connected.`

## Verification Assertions

- Verified via exit status `0` and runtime checks inside executable.

---

[Back to `sim` Tests Overview](../sim.md)
