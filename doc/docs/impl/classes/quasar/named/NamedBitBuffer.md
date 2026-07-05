# NamedBitBuffer

`NamedBitBuffer` is a specialized `NamedObject` that manages a bit-addressable buffer. It combines the hierarchical features of `NamedObject` with the bit manipulation capabilities of `coretypes::BitBuffer`.

## Features

- **Hierarchical**: Can be part of a `NamedObject` tree.
- **Bit-Level Access**:Inherits `getBit`, `setBit`, `sliceBits`, `concatBits`, and `reverseBits` from `BitBuffer`.
- **Factory Creation**: Use `create()` to ensure proper initialization of the hierarchy.

## API Overview

### Factory Methods

- `create(const std::string &name, size_t bitSize, std::shared_ptr<NamedObject> parent = nullptr)`: Creates a bit buffer with the specified bit capacity.

### Core Methods

- `clone()`: Creates a shallow copy (state only, no hierarchy).
- Inherits all `BitBuffer` operations for bit manipulation.
- Inherits all `NamedObject` operations for tree management.

## Usage Example

```cpp
#include "quasar/named/NamedBitBuffer.hpp"

using namespace quasar::named;

auto root = NamedObject::create("root");
auto bitBuf = NamedBitBuffer::create("config", 16, root);

bitBuf->setBit(0, true);
bitBuf->setBit(15, true);

std::cout << "Bit size: " << bitBuf->bitSize() << std::endl;
```
