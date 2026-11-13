# Test Case: `JsonMapperTest.InvalidJson`

| Property | Value |
| :--- | :--- |
| **Module** | `datacodec` |
| **Source File** | `cmake-projects/datacodec/test/TestJsonMapper.cpp` (lines 167–174) |
| **Category** | Unit & Regression Verification |
| **Traceability** | `TSK-20261113-003,`, `FE-0010,` |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Verifies syntactically malformed JSON yields JsonMapperErrorCode::InvalidJson.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "JsonMapper" as SUT0
participant "JsonMapperErrorCode" as SUT1
Runner -> SUT0 : Execute test sequence
SUT0 --> Runner : Verification result
Runner -> Runner : ASSERT_FALSE(result.has_value())
Runner -> Runner : EXPECT_EQ(result.error(), JsonMapperErrorCode::InvalidJson)
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `ASSERT_FALSE(result.has_value())`
- `EXPECT_EQ(result.error(), JsonMapperErrorCode::InvalidJson)`

---

[Back to `datacodec` Tests Overview](../datacodec.md)
