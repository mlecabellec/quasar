# NamedBitBufferSlice

`NamedBitBufferSlice` provides a named view into a range of bits within a `BitBuffer`. It is useful for representing fields within a larger bitstream.

## Features

- **Non-Owning**: Operates on a reference to an existing buffer.
- **Hierarchical**: Can represent structural sub-components of a larger data structure.
- **Named Interface**: Provides the standard `NamedObject` API for the slice.

## API Overview

### Factory Methods

- `create(const std::string &name, std::shared_ptr<coretypes::BitBuffer> buffer, size_t offset, size_t size, std::shared_ptr<NamedObject> parent = nullptr)`

### Core Methods

- `clone()`: Creates a new slice pointing to the same data.
- Inherits `getBit`, `setBit` from `BitBufferSlice`.

## Usage Example

```cpp
#include "quasar/named/NamedBitBufferSlice.hpp"

using namespace quasar::named;

auto rawBuffer = std::make_shared<coretypes::BitBuffer>(100);
auto header = NamedBitBufferSlice::create("header", rawBuffer, 0, 32);

if (header->getBit(0)) {
    // Process header
}
```
