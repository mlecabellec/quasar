# Test Case: `TransformerTest.InPlaceTransformation`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestTransformer.cpp` (lines 71–107) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `TransformerTest.InPlaceTransformation` defined in `cmake-projects/named/test/TestTransformer.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedString" as SUT0
loop Bounded test iteration / stress window
Runner -> SUT0 : std::shared_ptr<NamedString> root = NamedString::create( root , v )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedString> child1 = NamedString::create( child1...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedString> child2 = NamedString::create( child2...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : transformer.addRule(
SUT0 --> Runner : return / state updated
Runner -> SUT0 : return ctx.getNode()->getName() == child1
SUT0 --> Runner : return / state updated
Runner -> SUT0 : (const TransformContext ctx, Transformer t) -> std::vector<std::s...
SUT0 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
Runner -> Runner : ASSERT_EQ(root->getChildren().size(), 3) // child1_a, child1_b, c...
Runner -> Runner : ASSERT_TRUE(foundA)
Runner -> Runner : ASSERT_TRUE(foundB)
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedString> root = NamedString::create("root", "v")`
- `std::shared_ptr<NamedString> child1 = NamedString::create("child1", "v1", root)`
- `std::shared_ptr<NamedString> child2 = NamedString::create("child2", "v2", root)`
- `transformer.addRule(`
- `return ctx.getNode()->getName() == "child1"`
- `[](const TransformContext& ctx, Transformer& t) -> std::vector<std::shared_ptr<NamedObject>> {`
- `std::shared_ptr<NamedString> mod1 = NamedString::create("child1_a", "x")`
- `std::shared_ptr<NamedString> mod2 = NamedString::create("child1_b", "y")`

## Verification Assertions

- `ASSERT_EQ(root->getChildren().size(), 3); // child1_a, child1_b, child2`
- `ASSERT_TRUE(foundA)`
- `ASSERT_TRUE(foundB)`
- `ASSERT_TRUE(found2)`

---

[Back to `named` Tests Overview](../named.md)
