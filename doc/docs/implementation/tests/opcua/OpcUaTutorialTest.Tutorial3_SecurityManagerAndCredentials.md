# Test Case: `OpcUaTutorialTest.Tutorial3_SecurityManagerAndCredentials`

| Property | Value |
| :--- | :--- |
| **Module** | `opcua` |
| **Source File** | `cmake-projects/opcua/test_cs0100/TestOpcUaExtended.cpp` (lines 179–209) |
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
participant "UA_Server_new" as SUT0
participant "UA_Server_getConfig" as SUT1
participant "UA_Server_delete" as SUT2
participant "UA_Client_new" as SUT3
Runner -> Logger : TestScopeGuard start (OpcUaTutorialTest.Tutorial3_SecurityM...)
Runner -> SUT0 : UA_StatusCode missingCert = secMgr.loadCertificate( /nonexistent/...
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<OpcUaServerService> srv = OpcUaServerService::cre...
SUT1 --> Runner : return / state updated
Runner -> SUT2 : srv->loadCertificate( /nonexistent/a.der , /nonexistent/b.der )
SUT2 --> Runner : return / state updated
Runner -> SUT3 : srv->loadTrustList(std::vector<std::string> )
SUT3 --> Runner : return / state updated
Runner -> SUT0 : srv->addUser( operator , secret123 )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : std::shared_ptr<OpcUaClientService> cli = OpcUaClientService::cre...
SUT1 --> Runner : return / state updated
Runner -> Runner : EXPECT_EQ(missingCert, UA_STATUSCODE_BADNOTFOUND)
Runner -> Runner : EXPECT_EQ(secMgr.loadTrustList(std::vector<std::string> ), UA_STA...
Runner -> Runner : ASSERT_NE(rawServer, nullptr)
Runner -> Runner : EXPECT_EQ(secMgr.configureServer(rawServer, srvCfg), UA_STATUSCOD...
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `UA_StatusCode missingCert = secMgr.loadCertificate("/nonexistent/cert.der", "/nonexistent/key.der")`
- `std::shared_ptr<OpcUaServerService> srv = OpcUaServerService::create("SecSrv")`
- `srv->loadCertificate("/nonexistent/a.der", "/nonexistent/b.der")`
- `srv->loadTrustList(std::vector<std::string>{})`
- `srv->addUser("operator", "secret123")`
- `std::shared_ptr<OpcUaClientService> cli = OpcUaClientService::create("SecCli")`
- `cli->loadCertificate("/nonexistent/a.der", "/nonexistent/b.der")`
- `cli->loadTrustList(std::vector<std::string>{})`

## Verification Assertions

- `EXPECT_EQ(missingCert, UA_STATUSCODE_BADNOTFOUND)`
- `EXPECT_EQ(secMgr.loadTrustList(std::vector<std::string>{}), UA_STATUSCODE_GOOD)`
- `ASSERT_NE(rawServer, nullptr)`
- `EXPECT_EQ(secMgr.configureServer(rawServer, srvCfg), UA_STATUSCODE_GOOD)`
- `ASSERT_NE(rawClient, nullptr)`
- `EXPECT_EQ(secMgr.configureClient(rawClient), UA_STATUSCODE_GOOD)`

---

[Back to `opcua` Tests Overview](../opcua.md)
