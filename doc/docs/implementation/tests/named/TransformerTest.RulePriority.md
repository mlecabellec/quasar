# Test Case: `TransformerTest.RulePriority`

| Property | Value |
| :--- | :--- |
| **Module** | `named` |
| **Source File** | `cmake-projects/named/test/TestTransformer.cpp` (lines 226–254) |
| **Category** | Unit & Regression Verification |
| **Traceability** | See module specification (`FE-*` / `TSK-*`) |
| **Async Telemetry** | Standard GTest / CTest stdout |

## Summary

Executes verification scenario `TransformerTest.RulePriority` defined in `cmake-projects/named/test/TestTransformer.cpp`.

## Sequence Diagram

```plantuml
@startuml
autonumber
participant "TestRunner" as Runner
participant "NamedObject" as SUT0
Runner -> SUT0 : std::shared_ptr<NamedObject> root = NamedObject::create( target )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : transformer.addRule(
SUT0 --> Runner : return / state updated
Runner -> SUT0 : (const TransformContext ctx) return ctx.getNode()->getName() == t...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : (const TransformContext ctx, Transformer t) -> std::vector<std::s...
SUT0 --> Runner : return / state updated
Runner -> SUT0 : return NamedObject::create( loser )
SUT0 --> Runner : return / state updated
Runner -> SUT0 : transformer.addRule(
SUT0 --> Runner : return / state updated
Runner -> Runner : ASSERT_EQ(result.size(), 1)
Runner -> Runner : ASSERT_EQ(result 0 ->getName(), winner )
@enduml
```

## Observed Operations

- `std::shared_ptr<NamedObject> root = NamedObject::create("target")`
- `transformer.addRule(`
- `[](const TransformContext& ctx) { return ctx.getNode()->getName() == "target"; },`
- `[](const TransformContext& ctx, Transformer& t) -> std::vector<std::shared_ptr<NamedObject>> {`
- `return {NamedObject::create("loser")}`
- `transformer.addRule(`
- `[](const TransformContext& ctx) { return ctx.getNode()->getName() == "target"; },`
- `[](const TransformContext& ctx, Transformer& t) -> std::vector<std::shared_ptr<NamedObject>> {`

## Verification Assertions

- `ASSERT_EQ(result.size(), 1)`
- `ASSERT_EQ(result[0]->getName(), "winner")`

---

[Back to `named` Tests Overview](../named.md)
