# Floating Point Types

The `coretypes` module defines several standard floating-point types based on the `FloatingPoint` template class.

## Available Types

| project Type | C++ Equivalent | Bits |
| :--- | :--- | :--- |
| `Float32` | `float` | 32 |
| `Float64` | `double` | 64 |

## Features

- **Polymorphic**: Implements the `Number` interface.
- **Safe Arithmetic**: Operations like `safeDivide` throw `std::runtime_error` on division by zero instead of returning `Inf`.
- **Total Ordering**: Provides a reliable `compareTo` implementation even for `NaN`.

## Usage

```cpp
#include "quasar/coretypes/FloatingPoint.hpp"

using namespace quasar::coretypes;

FloatingPoint<double> d(1.234);
if (d.getType() == "FloatingPoint") {
    std::cout << "Dynamic type identified." << std::endl;
}
```
