# Test Case: `QlshTest.ZmqPluginInteraction`

| Property | Value |
| :--- | :--- |
| **Module** | `scripting` |
| **Source File** | `cmake-projects/scripting/test/TestQlsh.cpp` (lines 108–113) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Verifies that the ZMQ plugin can be loaded and used within qlsh.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : std::string script = ctx = quasar.zmq.Context.new() n
Runner -> Runner : std::string output = run_qlsh(script, quasar_zmq.so )
Runner -> Runner : EXPECT_NE(output.find( ZMQ_CONTEXT_READY ), std::string::npos)
@enduml
```

## Observed Operations

- `std::string script = "ctx = quasar.zmq.Context.new()\n"`
- `std::string output = run_qlsh(script, {"quasar_zmq.so"})`

## Verification Assertions

- `EXPECT_NE(output.find("ZMQ_CONTEXT_READY"), std::string::npos)`

---

[Back to `scripting` Tests Overview](../scripting.md)
