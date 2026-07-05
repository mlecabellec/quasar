# IntegerCodec

`IntegerCodec` is a template class for encoding and decoding integer values.

## Features

- **Templated**: Supports any integral type `T`.
- **Endianness**: Configurable byte ordering.
- **Flexible Size**: Can operate on a subset of bits (e.g., a 12-bit integer).

## API Overview

### Constructor

- `IntegerCodec(size_t bitSize, bool isBigEndian = true)`: Initializes the codec with a specific bit width.

### Core Methods

- `decode()`: Returns a `NamedInteger<T>`.
- `encode()`: Writes the value of a `NamedInteger<T>` to the buffer.
- `getBitSize()`: Returns the configured bit size.

## Usage Example

```cpp
#include "datacodec/IntegerCodec.hpp"

using namespace datacodec;

// A 12-bit Little Endian integer codec
auto codec = std::make_shared<IntegerCodec<uint16_t>>(12, false);
```
