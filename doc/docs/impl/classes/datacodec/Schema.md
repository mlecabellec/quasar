# Schema

The `Schema` header defines the structures used to describe composite binary data formats.

## Key Classes

### FieldDef

Represents a single field in a composite structure.

- **Name**: The identifier for the field.
- **Codec**: The `ICodec` implementation used to encode/decode the field.
- **Bit Offset**: The position of the field relative to the start of the container.

### ContainerDef

A collection of `FieldDef` objects representing a record or structure.

- **Manage Fields**: Provides methods to add and retrieve field definitions.
- **Hierarchical**: Inherits from `NamedObject`, allowing schemas to be named.

## Usage Example

```cpp
#include "datacodec/Schema.hpp"
#include "datacodec/IntegerCodec.hpp"

using namespace datacodec;

auto schema = ContainerDef::create("MessageHeader");
auto idCodec = std::make_shared<IntegerCodec<uint16_t>>(16);

schema->addField(FieldDef::create("msgId", idCodec, 0));
schema->addField(FieldDef::create("version", idCodec, 16));
```
