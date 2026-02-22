# FloatCodec

`FloatCodec` is a template class for encoding and decoding floating-point values.

## Features

- **Supports float and double**: Uses `sizeof(T) * 8` as default bit size.
- **Endianness**: Supports both Big Endian and Little Endian formats.

## API Overview

### Constructor

- `FloatCodec(bool isBigEndian = true)`

### Core Methods

- `decode()`: Returns a `NamedFloatingPoint<T>`.
- `encode()`: Writes a `NamedFloatingPoint<T>` value.

## Usage Example

```cpp
#include "datacodec/FloatCodec.hpp"

using namespace datacodec;

auto doubleCodec = std::make_shared<FloatCodec<double>>();
```
