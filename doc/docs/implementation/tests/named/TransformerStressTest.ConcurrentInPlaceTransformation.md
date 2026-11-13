# Test Case: `TransformerStressTest.ConcurrentInPlaceTransformation`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestTransformerStress.cpp` (lines 32–103) |
| **Category** | Determinism, Fuzzing, Concurrency & Stress (`CS-0100.7`, `CS-0100.10`, `CS-0100.11`, `CS-0100.12`) |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Verify concurrent in-place transformation on different subtrees.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
loop Bounded test iteration / stress window
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : level1.push_back(NamedObject::create( l1_ + std::to_string(i), ro...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> l2 = NamedObject::create( l2_ + std:...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : (void)NamedObject::create( l3_ + std::to_string(k), l2)
SUT0 --> Runner : return / state updated
Runner -> SUT0 : transformer.addRule(
SUT0 --> Runner : return / state updated
Runner -> SUT0 : (const TransformContext ctx, Transformer t) -> std::vector<std::s...
SUT0 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
Runner -> Runner : ASSERT_FALSE(failed)
Runner -> Runner : ASSERT_EQ(root->getChildren().size(), 10)
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `level1.push_back(NamedObject::create("l1_" + std::to_string(i), root))`
- `std::shared_ptr<NamedObject> l2 = NamedObject::create("l2_" + std::to_string(j), l1)`
- `(void)NamedObject::create("l3_" + std::to_string(k), l2)`
- `transformer.addRule(`
- `[](const TransformContext& ctx, Transformer& t) -> std::vector<std::shared_ptr<NamedObject>> {`
- `std::shared_ptr<NamedObject> node = ctx.getNode()`
- `node->setName(node->getName() + "_visited")`

## Verification Assertions

- `ASSERT_FALSE(failed)`
- `ASSERT_EQ(root->getChildren().size(), 10)`

---

[Back to `named` Tests Overview](../named.md)
