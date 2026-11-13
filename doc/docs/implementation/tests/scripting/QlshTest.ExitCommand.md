# Test Case: `QlshTest.ExitCommand`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestQlsh.cpp` (lines 199–202) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Verifies that the exit() command works.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : Execute test sequence
Runner -> Runner : EXPECT_EQ(output.find( SHOULD_NOT_SEE_THIS ), std::string::npos)
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_EQ(output.find("SHOULD_NOT_SEE_THIS"), std::string::npos)`

---

[Back to `scripting` Tests Overview](../scripting.md)
