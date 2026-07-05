# BinaryMapper

`BinaryMapper` is the high-level utility responsible for mapping between raw binary buffers and `NamedObject` hierarchies using a defined `Schema` (`ContainerDef`).

## Features

- **Automated Mapping**: Iterates through schema fields and invokes respective codecs.
- **Context Awareness**: Handles conditional fields and relative bit offsets.
- **Error Handling**: Gracefully handles decoding errors for individual fields.

## API Overview

### Static Methods

- `decode(std::shared_ptr<ContainerDef> schema, const BitBufferSlice &buffer)`: Returns a fully populated `NamedObject` hierarchy.
- `encode(std::shared_ptr<ContainerDef> schema, const std::shared_ptr<NamedObject> &data, BitBufferSlice &buffer)`: Writes a hierarchy back to binary.

## Usage Example

```cpp
#include "datacodec/BinaryMapper.hpp"

using namespace datacodec;

auto schema = // ... define schema ...
auto buffer = // ... get buffer ...

auto result = BinaryMapper::decode(schema, buffer);
auto intVal = std::dynamic_pointer_cast<NamedInteger<int>>(result->getLastChild());
```
