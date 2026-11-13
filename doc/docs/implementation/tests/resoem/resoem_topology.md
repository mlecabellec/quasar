# Test Case: `resoem_topology`

| Property | Value |
| :--- | :--- |
| **Module** | `resoem` |
| **Source File** | `cmake-projects/resoem/test/test_topology.cpp` (lines 1–189) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `resoem_topology` defined in `cmake-projects/resoem/test/test_topology.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "Enumerator" as SUT0
participant "Slave0" as SUT1
participant "Slave1" as SUT2
participant "Slave2" as SUT3
loop Bounded test iteration / stress window
Runner -> SUT0 : int count = slaves.size()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : s.children_indices.clear()
SUT1 --> Runner : return / state updated
Runner -> SUT2 : slaves parent .children_indices.push_back(i)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : assert(slaves 0 .parent_index == -1)
SUT3 --> Runner : return / state updated
Runner -> SUT0 : assert(slaves 1 .parent_index == 0)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : assert(slaves 1 .parent_port == 3)
SUT1 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
@enduml
```

## Observed Operations

- `int count = slaves.size()`
- `s.children_indices.clear()`
- `slaves[parent].children_indices.push_back(i)`
- `assert(slaves[0].parent_index == -1)`
- `assert(slaves[1].parent_index == 0)`
- `assert(slaves[1].parent_port == 3)`
- `assert(slaves[2].parent_index == 1)`
- `assert(slaves[2].parent_port == 3)`

## Verification Assertions

- Verified via exit status `0` and runtime checks inside executable.

---

[Back to `resoem` Tests Overview](../resoem.md)
