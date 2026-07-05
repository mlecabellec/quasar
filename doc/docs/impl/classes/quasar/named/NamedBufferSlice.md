# NamedBufferSlice

`NamedBufferSlice` provides a named view into a sub-region of a `Buffer`.

## Features

- **View-Only**: Does not own the underlying data.
- **Sub-slicing**: Can create further nested slices.
- **Named Hierarchy**: Allows mapping structural segments of a buffer to a name tree.

## API Overview

### Factory Methods

- `create(const std::string &name, std::shared_ptr<coretypes::Buffer> buffer, size_t offset, size_t size, std::shared_ptr<NamedObject> parent = nullptr)`

### Core Methods

- `clone()`: Creates a new named slice view.
- `get(size_t index)`, `set(size_t index, uint8_t value)`: Access relative to slice start.

## Usage Example

```cpp
#include "quasar/named/NamedBufferSlice.hpp"

using namespace quasar::named;

auto mainBuf = std::make_shared<coretypes::Buffer>(100);
auto subSection = NamedBufferSlice::create("data", mainBuf, 10, 50);
```
