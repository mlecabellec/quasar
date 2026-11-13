# Test Case: `ResoesTutorialTest.Tutorial2_ObjectDictionaryAndCoePdoPackUnpack`

| Property | Value |
| :--- | :--- |
| **Module** | `resoes` |
| **Source File** | `cmake-projects/resoes/test_cs0100/TestResoesExtended.cpp` (lines 100–148) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `[TSK-20261113-001]`, `[FE-0280]`, `[CS-0010.37]` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Extended CS-0100 test suite for `resoes` EtherCAT SubDevice stack.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "OdObjectType" as SUT0
participant "OdDataType" as SUT1
Runner -> Logger : TestScopeGuard start (ResoesTutorialTest.Tutorial2_ObjectDi...)
Runner -> SUT0 : ObjectDictionary od = app.od()
SUT0 --> Runner : return / state updated
Runner -> SUT1 : ).has_value())
SUT1 --> Runner : return / state updated
Runner -> SUT0 : ).has_value())
SUT0 --> Runner : return / state updated
Runner -> SUT1 : Result<std::vector<byte>> readBack = od.read_bytes(0x7000U, 0U)
SUT1 --> Runner : return / state updated
Runner -> Runner : ASSERT_TRUE(app.init().has_value())
Runner -> Runner : ASSERT_TRUE(od.register_object(outVar).has_value())
Runner -> Runner : ASSERT_TRUE(od.register_object(inVar).has_value())
Runner -> Runner : ASSERT_TRUE(od.register_object(OdObjectEntry 0x1600U, OdObjectTyp...
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `ObjectDictionary& od = app.od()`
- `}}).has_value())`
- `}}).has_value())`
- `Result<std::vector<byte>> readBack = od.read_bytes(0x7000U, 0U)`

## Verification Assertions

- `ASSERT_TRUE(app.init().has_value())`
- `ASSERT_TRUE(od.register_object(outVar).has_value())`
- `ASSERT_TRUE(od.register_object(inVar).has_value())`
- `ASSERT_TRUE(od.register_object(OdObjectEntry{0x1600U, OdObjectType::Array, 1U, "RxPdoMap", {`
- `ASSERT_TRUE(od.register_object(OdObjectEntry{0x1C12U, OdObjectType::Array, 1U, "Sm2Assign", {`
- `ASSERT_TRUE(app.coe().compute_pdo_sizes_bytes(rxBytes, txBytes).has_value())`
- `EXPECT_EQ(rxBytes, 2U)`
- `ASSERT_TRUE(app.coe().unpack_rx_pdo(masterOut).has_value())`
- `ASSERT_TRUE(readBack.has_value())`
- `ASSERT_EQ(readBack.value().size(), 2U)`

---

[Back to `resoes` Tests Overview](../resoes.md)
