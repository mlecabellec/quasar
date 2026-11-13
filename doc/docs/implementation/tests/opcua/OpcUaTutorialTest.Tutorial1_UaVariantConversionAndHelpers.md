# Test Case: `OpcUaTutorialTest.Tutorial1_UaVariantConversionAndHelpers`

| Property | Value |
| :--- | :--- |
| **Module** | `opcua` |
| **Source File** | `cmake-projects/opcua/test_cs0100/TestOpcUaExtended.cpp` (lines 93–128) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `[TSK-20260311-005]`, `[TSK-20261113-003]`, `[FE-0200]`, `[FE-0220]`, `[CS-0040.2]` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Extended CS-0100 test suite for `quasar_opcua` covering server/client mirroring, `UA_Common`, `OpcUaSecurityManager`, error boundaries, fuzzing, and concurrent client task queues.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "UA_String_fromStdString" as SUT0
participant "UA_String_clear" as SUT1
participant "UA_ByteString_fromStdString" as SUT2
participant "UA_ByteString_clear" as SUT3
Runner -> Logger : TestScopeGuard start (OpcUaTutorialTest.Tutorial1_UaVariant...)
Runner -> SUT0 : named::NamedInteger<int64_t>::create( Counter , 1024)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : named::NamedBoolean::create( Active , true)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : named::NamedString::create( Tag , Pump01 )
SUT2 --> Runner : return / state updated
Runner -> SUT3 : named::NamedFloatingPoint<double>::create( Temp , 21.5)
SUT3 --> Runner : return / state updated
Runner -> Logger : logOutput( variantMarshaling , ok )
Runner -> Runner : EXPECT_EQ(uaStr.length, 11U)
Runner -> Runner : EXPECT_EQ(uaBs.length, 3U)
Runner -> Runner : EXPECT_FALSE(UA_Variant_isEmpty( vInt))
Runner -> Runner : EXPECT_FALSE(UA_Variant_isEmpty( vBool))
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `named::NamedInteger<int64_t>::create("Counter", 1024)`
- `named::NamedBoolean::create("Active", true)`
- `named::NamedString::create("Tag", "Pump01")`
- `named::NamedFloatingPoint<double>::create("Temp", 21.5)`
- `logOutput("variantMarshaling", "ok")`

## Verification Assertions

- `EXPECT_EQ(uaStr.length, 11U)`
- `EXPECT_EQ(uaBs.length, 3U)`
- `EXPECT_FALSE(UA_Variant_isEmpty(&vInt))`
- `EXPECT_FALSE(UA_Variant_isEmpty(&vBool))`
- `EXPECT_FALSE(UA_Variant_isEmpty(&vStr))`
- `EXPECT_FALSE(UA_Variant_isEmpty(&vDbl))`

---

[Back to `opcua` Tests Overview](../opcua.md)
