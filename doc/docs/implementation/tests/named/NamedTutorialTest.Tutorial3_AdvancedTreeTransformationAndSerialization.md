# Test Case: `NamedTutorialTest.Tutorial3_AdvancedTreeTransformationAndSerialization`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test_cs0100/TestNamedExtended.cpp` (lines 165–194) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `[TSK-20260328-001]`, `[TSK-20260311-001]`, `[FE-0260]`, `[FE-0150]` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Extended CS-0100 test suite for `quasar_named` covering reflexive methods, web methods, traversal rules, CopyPolicy, and temporal/quantity/variant nodes.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "NamedObject" as SUT0
participant "NamedBuffer" as SUT1
participant "PredefinedRules" as SUT2
participant "CopyPolicy" as SUT3
Runner -> Logger : TestScopeGuard start (NamedTutorialTest.Tutorial3_AdvancedT...)
Runner -> SUT0 : std::shared_ptr<NamedObject> packet = NamedObject::create( Frame )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : (void)NamedBuffer::create( Payload , rawBytes, packet)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : transformer.addRule(traversal::PredefinedRules::sliceBuffer( Payl...
SUT2 --> Runner : return / state updated
Runner -> SUT3 : transformer.addRule(
SUT3 --> Runner : return / state updated
Runner -> SUT0 : std::vector<std::shared_ptr<NamedObject>> outRoots = transformer....
SUT0 --> Runner : return / state updated
Runner -> Logger : logOutput( jsonLen , static_cast<long long>(jsonStr.size()))
Runner -> Runner : EXPECT_FALSE(outRoots.empty())
Runner -> Runner : EXPECT_FALSE(jsonStr.empty())
Runner -> Runner : EXPECT_FALSE(yamlStr.empty())
Runner -> Runner : EXPECT_FALSE(xmlStr.empty())
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> packet = NamedObject::create("Frame")`
- `(void)NamedBuffer::create("Payload", rawBytes, packet)`
- `transformer.addRule(traversal::PredefinedRules::sliceBuffer("Payload", slices, CopyPolicy::SHARE, 10))`
- `transformer.addRule(`
- `std::vector<std::shared_ptr<NamedObject>> outRoots = transformer.transform(packet)`
- `logOutput("jsonLen", static_cast<long long>(jsonStr.size()))`

## Verification Assertions

- `EXPECT_FALSE(outRoots.empty())`
- `EXPECT_FALSE(jsonStr.empty())`
- `EXPECT_FALSE(yamlStr.empty())`
- `EXPECT_FALSE(xmlStr.empty())`

---

[Back to `named` Tests Overview](../named.md)
