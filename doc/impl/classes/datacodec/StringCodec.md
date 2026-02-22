# StringCodec

`StringCodec` is used to encode and decode character strings.

## Features

- **Fixed Length**: Supports fixed-length strings with padding.
- **Variable Length**: Supports variable-length strings (null-terminated or length-prefixed).
- **Customizable**: Configurable maximum length.

## API Overview

### Constructor

- `StringCodec(size_t maxByteLength, bool isFixedLength = true)`

### Core Methods

- `decode()`: Returns a `NamedString`.
- `encode()`: Writes a `NamedString` value.
- `getEncodedBitSize()`: Calculates required bits for variable strings.

## Usage Example

```cpp
#include "datacodec/StringCodec.hpp"

using namespace datacodec;

auto fixedCodec = std::make_shared<StringCodec>(32, true);
```
