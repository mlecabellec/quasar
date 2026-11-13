# Test Case: `QlshTest.MultiLineAccumulation`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestQlsh.cpp` (lines 118–125) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Verifies the multi-line statement accumulator logic.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : print( SUM= .. adder(10, 20))
Runner -> Runner : EXPECT_NE(output.find( SUM=30 ), std::string::npos)
@enduml
```

## Observed Operations

- `"print('SUM=' .. adder(10, 20))"`

## Verification Assertions

- `EXPECT_NE(output.find("SUM=30"), std::string::npos)`

---

[Back to `scripting` Tests Overview](../scripting.md)
