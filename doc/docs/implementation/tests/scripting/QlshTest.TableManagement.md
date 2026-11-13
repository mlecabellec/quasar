# Test Case: `QlshTest.TableManagement`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestQlsh.cpp` (lines 242–248) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Verifies complex table management in qlsh.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : print( TABLE_SUM= .. myTable.a + myTable.b + myTable.c)
Runner -> Runner : EXPECT_NE(output.find( TABLE_SUM=6 ), std::string::npos)
@enduml
```

## Observed Operations

- `"print('TABLE_SUM=' .. myTable.a + myTable.b + myTable.c)"`

## Verification Assertions

- `EXPECT_NE(output.find("TABLE_SUM=6"), std::string::npos)`

---

[Back to `scripting` Tests Overview](../scripting.md)
