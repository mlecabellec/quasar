# Test Case: `TransformerTest.RecursiveModification`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestTransformer.cpp` (lines 184–224) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `TransformerTest.RecursiveModification` defined in `cmake-projects/named/test/TestTransformer.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
loop Bounded test iteration / stress window
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( root )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : std::shared_ptr<NamedObject> branch = NamedObject::create( branch...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : NamedInteger<int>::create( leaf , 42, branch)
SUT0 --> Runner : return / state updated
Runner -> SUT0 : transformer.addRule(
SUT0 --> Runner : return / state updated
Runner -> SUT0 : return ctx.getNode()->getName() == branch
SUT0 --> Runner : return / state updated
Runner -> SUT0 : (const TransformContext ctx, Transformer t) -> std::vector<std::s...
SUT0 --> Runner : return / state updated
end
alt Exception / boundary / conditional branch check
Runner -> Runner : ASSERT_EQ(result.size(), 1)
Runner -> Runner : ASSERT_NE(newBranch, nullptr)
Runner -> Runner : ASSERT_NE(newLeaf, nullptr)
else Nominal path verified
Runner -> Runner : Post-conditions hold
end
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("root")`
- `std::shared_ptr<NamedObject> branch = NamedObject::create("branch", root)`
- `NamedInteger<int>::create("leaf", 42, branch)`
- `transformer.addRule(`
- `return ctx.getNode()->getName() == "branch"`
- `[](const TransformContext& ctx, Transformer& t) -> std::vector<std::shared_ptr<NamedObject>> {`
- `std::shared_ptr<NamedObject> newBranch = NamedObject::create("branch_mod")`
- `std::vector<std::shared_ptr<NamedObject>> transformed = t.transformSubtree(child, ctx.getDepth() + 1, ctx.getPath() + "/" + child->getName())`

## Verification Assertions

- `ASSERT_EQ(result.size(), 1)`
- `ASSERT_NE(newBranch, nullptr)`
- `ASSERT_NE(newLeaf, nullptr)`
- `ASSERT_EQ(newLeaf->getType(), "NamedInteger")`

---

[Back to `named` Tests Overview](../named.md)
