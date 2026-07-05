# Integer Types

The `coretypes` module defines several standard integer types based on the `Integer` template class.

## Available Types

The following aliases are typically used in the project:

| project Type | C++ Equivalent | Range |
| :--- | :--- | :--- |
| `Int8` | `int8_t` | -128 to 127 |
| `UInt8` | `uint8_t` | 0 to 255 |
| `Int16` | `int16_t` | -32,768 to 32,767 |
| `UInt16` | `uint16_t` | 0 to 65,535 |
| `Int32` | `int32_t` | -2^31 to 2^31 - 1 |
| `UInt32` | `uint32_t` | 0 to 2^32 - 1 |
| `Int64` | `int64_t` | -2^63 to 2^63 - 1 |
| `UInt64` | `uint64_t` | 0 to 2^64 - 1 |

## Features

- **Polymorphic**: Can be used via the `Number` interface.
- **Safe**: Supports overflow detection via `safeAdd`, `safeSubtract`, etc.
- **Immutable**: Thread-safe sharing.

## Usage

```cpp
#include "quasar/coretypes/Integer.hpp"

using namespace quasar::coretypes;

Integer<int32_t> val(100);
auto result = val.safeAdd(Integer<int32_t>(50));
std::cout << result->toInt() << std::endl; // 150
```
