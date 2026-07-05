# FloatingPoint

The `FloatingPoint` template class provides an object-oriented wrapper for primitive floating-point types (`float`, `double`). It extends the `Number` abstract base class.

## Key Features

- **Immutability**: Once created, the value cannot be changed.
- **Thread-Safety**: Safe for concurrent reads.
- **Safe Arithmetic**: Methods like `safeAdd`, `safeSubtract`, `safeMultiply`, and `safeDivide` throw exceptions on overflow (Infinity) or undefined operations (NaN).
- **Total Ordering**: Implements a consistent ordering even for NaN values (NaN is considered greater than all other values).

## API Overview

### Constructors

- `FloatingPoint(T value)`: Wraps a primitive value.
- `FloatingPoint(const std::string &s)`: Parses a string into a floating-point value.

### Arithmetic Operations

Standard operations return new `FloatingPoint` instances:
- `add`, `subtract`, `multiply`, `divide`

Safe versions (throw `std::overflow_error` or `std::runtime_error`):
- `safeAdd`, `safeSubtract`, `safeMultiply`, `safeDivide`

### Conversions

- `toInt()`, `toLong()`: Truncate fractional part, with bounds checking.
- `toFloat()`, `toDouble()`: Standard conversions with range checks for `toDouble` to `float`.
- `toString()`: Returns the string representation.

### Comparison

- `compareTo(const Number &other)`: Polymorphic comparison.
- `equals(const Number &other)`: Polymorphic equality check.
- `operator==`, `operator!=`, etc.: Comparison with primitive values.

## Usage Example

```cpp
#include "quasar/coretypes/FloatingPoint.hpp"

using namespace quasar::coretypes;

FloatingPoint<double> pi(3.14159);
FloatingPoint<double> two(2.0);

auto result = pi.multiply(two);
std::cout << result.toString() << std::endl; // 6.283180

try {
    FloatingPoint<double> large(1e308);
    auto overflow = large.safeMultiply(large); // Throws std::overflow_error
} catch (const std::overflow_error& e) {
    // Handle overflow
}
```
