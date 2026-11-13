# Test Case: `resoem_dc_calc`

| Property | Value |
| :--- | :--- |
| **Module** | `resoem` |
| **Source File** | `cmake-projects/resoem/test/test_dc_calc.cpp` (lines 1–188) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `resoem_dc_calc` defined in `cmake-projects/resoem/test/test_dc_calc.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "Enumerator" as SUT0
participant "SlaveInfo" as SUT1
participant "ParentDelay" as SUT2
participant "Internal" as SUT3
loop Bounded test iteration / stress window
Runner -> SUT0 : assert(slaves 0 .propagation_delay == 0)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : assert(slaves 1 .propagation_delay == 100)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : assert(slaves 2 .propagation_delay == 200)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : assert(slaves 1 .propagation_delay == 100)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : assert(slaves 2 .propagation_delay == 200)
SUT0 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
@enduml
```

## Observed Operations

- `assert(slaves[0].propagation_delay == 0)`
- `assert(slaves[1].propagation_delay == 100)`
- `assert(slaves[2].propagation_delay == 200)`
- `assert(slaves[1].propagation_delay == 100)`
- `assert(slaves[2].propagation_delay == 200)`

## Verification Assertions

- Verified via exit status `0` and runtime checks inside executable.

---

[Back to `resoem` Tests Overview](../resoem.md)
