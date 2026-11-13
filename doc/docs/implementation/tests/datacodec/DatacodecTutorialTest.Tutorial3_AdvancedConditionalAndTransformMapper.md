# Test Case: `DatacodecTutorialTest.Tutorial3_AdvancedConditionalAndTransformMapper`

| Property | Value |
| :--- | :--- |
| **Module** | `datacodec` |
| **Source File** | `cmake-projects/datacodec/test/TestDatacodecExtended.cpp` (lines 141–172) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `TSK-20261113-004,`, `FE-0010,`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Tutorial 3 (Advanced): Compose `ConditionalFieldDef`, `TransformCodec`, and `BinaryMapper`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "ContainerDef" as SUT0
participant "FieldDef" as SUT1
participant "ConditionalFieldDef" as SUT2
participant "DatacodecTestConstants" as SUT3
Runner -> Logger : TestScopeGuard start (DatacodecTutorialTest.Tutorial3_Advan...)
Runner -> SUT0 : (std::shared_ptr<quasar::named::NamedObject> obj) -> std::shared_...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : (std::shared_ptr<quasar::named::NamedObject> obj) -> std::shared_...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : std::shared_ptr<datacodec::ContainerDef> schema = datacodec::Cont...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : schema->addField(datacodec::FieldDef::create( header , rawU16, 0U))
SUT3 --> Runner : return / state updated
Runner -> SUT0 : schema->addField(datacodec::ConditionalFieldDef::create(
SUT0 --> Runner : return / state updated
Runner -> SUT1 : includeOptional (const quasar::named::NamedObject* /*ctx*/) -> bool
SUT1 --> Runner : return / state updated
Runner -> Runner : ASSERT_NE(decoded, nullptr)
Runner -> Runner : EXPECT_EQ(decoded->getName(), ConditionalPacket )
Runner -> Runner : ASSERT_NE(decodedSkipped, nullptr)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `[](std::shared_ptr<quasar::named::NamedObject> obj) -> std::shared_ptr<quasar::named::NamedObject> {`
- `[](std::shared_ptr<quasar::named::NamedObject> obj) -> std::shared_ptr<quasar::named::NamedObject> {`
- `std::shared_ptr<datacodec::ContainerDef> schema = datacodec::ContainerDef::create("ConditionalPacket")`
- `schema->addField(datacodec::FieldDef::create("header", rawU16, 0U))`
- `schema->addField(datacodec::ConditionalFieldDef::create(`
- `[&includeOptional](const quasar::named::NamedObject* /*ctx*/) -> bool {`

## Verification Assertions

- `ASSERT_NE(decoded, nullptr)`
- `EXPECT_EQ(decoded->getName(), "ConditionalPacket")`
- `ASSERT_NE(decodedSkipped, nullptr)`

---

[Back to `datacodec` Tests Overview](../datacodec.md)
