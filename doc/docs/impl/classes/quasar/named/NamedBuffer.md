# NamedBuffer

`NamedBuffer` is a `NamedObject` that manages a contiguous byte buffer, inheriting from `coretypes::Buffer`.

## Features

- **Byte Access**: Provides `get`, `set` for individual bytes.
- **Numeric IO**: Inherits `readInt`, `writeInt`, `readDouble`, etc. with endianness support.
- **Bitwise Ops**: Supports `bitwiseAnd`, `bitwiseOr`, etc. on byte buffers.

## API Overview

### Factory Methods

- `create(const std::string &name, size_t size, std::shared_ptr<NamedObject> parent = nullptr)`
- `create(const std::string &name, const std::vector<uint8_t> &data, std::shared_ptr<NamedObject> parent = nullptr)`

### Core Methods

- `clone()`: Deep copy of the buffer content into a new named object.
- `size()`: Returns buffer size in bytes.

## Usage Example

```cpp
#include "quasar/named/NamedBuffer.hpp"

using namespace quasar::named;

auto buf = NamedBuffer::create("payload", 1024);
buf->writeInt(0x1234, 0, coretypes::BigEndian);
```
