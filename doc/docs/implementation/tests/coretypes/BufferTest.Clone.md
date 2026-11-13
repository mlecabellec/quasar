# Test Case: `BufferTest.Clone`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/TestBuffer.cpp` (lines 178–200) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `BufferTest.Clone` defined in `cmake-projects/coretypes/test/TestBuffer.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "Buffer" as SUT0
Runner -> SUT0 : Buffer b2 = b.clone()
SUT0 --> Runner : return / state updated
Runner -> SUT0 : b.set(0, 0xbb)
SUT0 --> Runner : return / state updated
Runner -> Runner : EXPECT_TRUE(b.equals(b2))
Runner -> Runner : EXPECT_FALSE(b.equals(b2))
Runner -> Runner : EXPECT_EQ(b2.toString(), aa )
@enduml
```

## Observed Operations

- `Buffer b2 = b.clone()`
- `b.set(0, 0xbb)`

## Verification Assertions

- `EXPECT_TRUE(b.equals(b2))`
- `EXPECT_FALSE(b.equals(b2))`
- `EXPECT_EQ(b2.toString(), "aa")`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
