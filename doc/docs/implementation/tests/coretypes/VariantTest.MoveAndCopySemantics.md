# Test Case: `VariantTest.MoveAndCopySemantics`

| Property | Value |
| :--- | :--- |
| **Module** | `coretypes` |
| **Source File** | `cmake-projects/coretypes/test/coretypes_variant_test.cpp` (lines 88–103) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Verifies that Variant correctly implements copy and move semantics.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
Runner -> Runner : Execute test sequence
Runner -> Runner : EXPECT_EQ(v2.getAs<std::string>(), CopyMe )
Runner -> Runner : EXPECT_EQ(v1.getAs<std::string>(), CopyMe )
Runner -> Runner : EXPECT_EQ(v2.getAs<std::string>(), MoveMe )
@enduml
```

## Observed Operations

- Invokes standalone executable entry flow.

## Verification Assertions

- `EXPECT_EQ(v2.getAs<std::string>(), "CopyMe")`
- `EXPECT_EQ(v1.getAs<std::string>(), "CopyMe")`
- `EXPECT_EQ(v2.getAs<std::string>(), "MoveMe")`

---

[Back to `coretypes` Tests Overview](../coretypes.md)
