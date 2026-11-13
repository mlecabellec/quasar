# Test Case: `UtilsCoverageTest.AllComponentStubsAndMetadata`

| Property | Value |
| :--- | :--- |
| **Module** | `utils` |
| **Source File** | `cmake-projects/utils/test/TestUtils.cpp` (lines 186–213) |
| **Category** | Coverage, Enum, Boundary & Exception Containment (`CS-0100.1`, `CS-0100.3`, `CS-0100.4`, `CS-0100.6`, `CS-0100.9`) |
| **Traceability** | `TSK-20261113-004`, `FE-0070,`, `CS-0090.1,` |
| **Async Telemetry** | Enabled (`TestScopeGuard` + `AsyncTestLogger`) |

## Summary

Verifies all EventManager, Logger, and TimeKeeper IComponent stubs and metadata.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "AsyncTestLogger" as Logger
participant "Smp" as SUT0
participant "GetState" as SUT1
participant "ComponentStateKind" as SUT2
participant "Publish" as SUT3
Runner -> Logger : TestScopeGuard start (UtilsCoverageTest.AllComponentStubsAn...)
loop Bounded test iteration / stress window
Runner -> SUT0 : svc->Publish(nullptr)
SUT0 --> Runner : return / state updated
Runner -> SUT1 : svc->Configure(nullptr, nullptr)
SUT1 --> Runner : return / state updated
Runner -> SUT2 : svc->Connect(nullptr)
SUT2 --> Runner : return / state updated
Runner -> SUT3 : svc->Disconnect()
SUT3 --> Runner : return / state updated
Runner -> SUT0 : svc->SetSimpleValue( missing , Smp::AnySimple )
SUT0 --> Runner : return / state updated
Runner -> SUT1 : svc->GetSimpleArrayValue( missing , 0U, nullptr, 0U)
SUT1 --> Runner : return / state updated
end
Runner -> Runner : EXPECT_EQ(svc->GetState(), Smp::ComponentStateKind::CSK_Connected)
Runner -> Runner : EXPECT_NE(svc->GetUuid().Data1, 999999U)
Runner -> Runner : EXPECT_EQ(svc->GetField( missing ), nullptr)
Runner -> Runner : EXPECT_EQ(svc->GetFields(), nullptr)
Runner -> Logger : ~TestScopeGuard flush metrics and duration
@enduml
```

## Observed Operations

- `svc->Publish(nullptr)`
- `svc->Configure(nullptr, nullptr)`
- `svc->Connect(nullptr)`
- `svc->Disconnect()`
- `svc->SetSimpleValue("missing", Smp::AnySimple{})`
- `svc->GetSimpleArrayValue("missing", 0U, nullptr, 0U)`
- `svc->SetSimpleArrayValue("missing", 0U, nullptr, 0U)`

## Verification Assertions

- `EXPECT_EQ(svc->GetState(), Smp::ComponentStateKind::CSK_Connected)`
- `EXPECT_NE(svc->GetUuid().Data1, 999999U)`
- `EXPECT_EQ(svc->GetField("missing"), nullptr)`
- `EXPECT_EQ(svc->GetFields(), nullptr)`
- `EXPECT_EQ(static_cast<int>(svc->GetSimpleValue("missing").type),`
- `EXPECT_FALSE(svc->AddChild(nullptr, nullptr))`
- `EXPECT_FALSE(svc->RemoveChild(nullptr, nullptr))`
- `EXPECT_EQ(svc->IsChildInCollection(nullptr, nullptr), nullptr)`
- `EXPECT_EQ(svc->GetChild(nullptr), nullptr)`

---

[Back to `utils` Tests Overview](../utils.md)
