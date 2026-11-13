# Test Case: `ResoemTutorialTest.Tutorial3_MasterServiceConfigAndEniDocumentParsing`

| Property | Value |
| :--- | :--- |
| **Module** | `resoem` |
| **Source File** | `cmake-projects/resoem/test_cs0100/TestResoemExtended.cpp` (lines 101–119) |
| **Category** | Pedagogical Tutorial (`CS-0100.19`) |
| **Traceability** | `[TSK-20260920-001]`, `[FE-0040]`, `[CS-0010.37]`, `[FE-0040.8]` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Extended CS-0100 test suite for `resoem` virtual EtherCAT bus, frame builder, ENI parser, fault injection, and concurrency.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "EthercatMasterService" as SUT0
participant "EniDocument" as SUT1
Runner -> Logger : TestScopeGuard start (ResoemTutorialTest.Tutorial3_MasterSe...)
Runner -> SUT0 : EthercatMasterService::create( Master0 )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : master->setInterface( lo )
SUT1 --> Runner : return / state updated
Runner -> SUT0 : master->setRedundancy(true, lo )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : master->setRedundancy(false, )
SUT1 --> Runner : return / state updated
Runner -> SUT0 : Result<> parsed = doc.load_from_memory(minimalEni)
SUT0 --> Runner : return / state updated
Runner -> Logger : logOutput( eniParsed , parsed.has_value())
Runner -> Runner : EXPECT_EQ(master->getType(), EthercatMasterService )
Runner -> Runner : EXPECT_TRUE(parsed.has_value())
Runner -> Runner : EXPECT_TRUE(doc.save_to_string().has_value())
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `EthercatMasterService::create("Master0")`
- `master->setInterface("lo")`
- `master->setRedundancy(true, "lo")`
- `master->setRedundancy(false, "")`
- `Result<> parsed = doc.load_from_memory(minimalEni)`
- `logOutput("eniParsed", parsed.has_value())`

## Verification Assertions

- `EXPECT_EQ(master->getType(), "EthercatMasterService")`
- `EXPECT_TRUE(parsed.has_value())`
- `EXPECT_TRUE(doc.save_to_string().has_value())`

---

[Back to `resoem` Tests Overview](../resoem.md)
