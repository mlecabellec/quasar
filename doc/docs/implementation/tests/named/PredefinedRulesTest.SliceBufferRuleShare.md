# Test Case: `PredefinedRulesTest.SliceBufferRuleShare`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestPredefinedRules.cpp` (lines 17–46) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `PredefinedRulesTest.SliceBufferRuleShare` defined in `cmake-projects/named/test/TestPredefinedRules.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedBuffer" as SUT0
participant "PredefinedRules" as SUT1
participant "CopyPolicy" as SUT2
Runner -> SUT0 : std::shared_ptr<NamedBuffer> root = NamedBuffer::create( Payload ...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : transformer.addRule(PredefinedRules::sliceBuffer( Payload ,
SUT1 --> Runner : return / state updated
Runner -> SUT2 : std::vector<std::shared_ptr<NamedObject>> resultVec = transformer...
SUT2 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedBufferSlice> header = resultVec 0 ->as<Named...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<NamedBufferSlice> body = resultVec 1 ->as<NamedBu...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : root->set(0, 0xAA)
SUT2 --> Runner : return / state updated
Runner -> Runner : ASSERT_EQ(resultVec.size(), 2)
Runner -> Runner : ASSERT_NE(header, nullptr)
Runner -> Runner : EXPECT_EQ(header->getName(), Header )
Runner -> Runner : EXPECT_EQ(header->size(), 2)
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedBuffer> root = NamedBuffer::create("Payload", data)`
- `transformer.addRule(PredefinedRules::sliceBuffer("Payload", {`
- `std::vector<std::shared_ptr<NamedObject>> resultVec = transformer.transform(root)`
- `std::shared_ptr<NamedBufferSlice> header = resultVec[0]->as<NamedBufferSlice>()`
- `std::shared_ptr<NamedBufferSlice> body = resultVec[1]->as<NamedBufferSlice>()`
- `root->set(0, 0xAA)`

## Verification Assertions

- `ASSERT_EQ(resultVec.size(), 2)`
- `ASSERT_NE(header, nullptr)`
- `EXPECT_EQ(header->getName(), "Header")`
- `EXPECT_EQ(header->size(), 2)`
- `EXPECT_EQ(header->get(0), 0x11)`
- `EXPECT_EQ(header->get(1), 0x22)`
- `ASSERT_NE(body, nullptr)`
- `EXPECT_EQ(body->getName(), "Body")`
- `EXPECT_EQ(body->size(), 4)`
- `EXPECT_EQ(body->get(0), 0x33)`

---

[Back to `named` Tests Overview](../named.md)
