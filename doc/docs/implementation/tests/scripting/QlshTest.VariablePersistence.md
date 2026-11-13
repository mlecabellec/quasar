# Test Case: `QlshTest.VariablePersistence`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestQlsh.cpp` (lines 182–194) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Verifies that variables are persisted between interactive lines.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : print( MYVAR_VALUE= .. myVar)
Runner -> Runner : print( VAL= .. tostring(myLocal))
Runner -> Runner : EXPECT_NE(output.find( MYVAR_VALUE=12345 ), std::string::npos)
Runner -> Runner : EXPECT_NE(outputLocal.find( VAL=persistent_secret ), std::string:...
@enduml
```

## Observed Operations

- `"print('MYVAR_VALUE=' .. myVar)"`
- `"print('VAL=' .. tostring(myLocal))"`

## Verification Assertions

- `EXPECT_NE(output.find("MYVAR_VALUE=12345"), std::string::npos)`
- `EXPECT_NE(outputLocal.find("VAL=persistent_secret"), std::string::npos)`

---

[Back to `scripting` Tests Overview](../scripting.md)
