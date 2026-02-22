# Advanced Schema

The `AdvancedSchema` header provides sophisticated components for complex binary format definitions.

## Key Classes

### ConditionalFieldDef

A field that is only included in the binary representation if a specific condition is met at runtime.

- **Predicate**: A function that takes the current decoding context (`NamedObject*`) and returns `bool`.
- **Logic**: Used for optional or variant fields.

### TransformCodec

A decorator codec that applies a transformation to the value after decoding or before encoding.

- **Use Cases**: Scaling values, applying offsets, or converting between units.
- **Transformers**: Functional objects for `Transformer` and `ReverseTransformer`.

## Usage Example

```cpp
#include "datacodec/AdvancedSchema.hpp"

using namespace datacodec;

// A field present only if another field is set
auto condField = ConditionalFieldDef::create("optionalField", codec, offset, 
    [](const NamedObject* ctx) {
        // ... check ctx for condition ...
        return true;
    });
```
