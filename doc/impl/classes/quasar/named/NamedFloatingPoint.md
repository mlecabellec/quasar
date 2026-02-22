# NamedFloatingPoint

`NamedFloatingPoint` is a template class that wraps a floating-point value within the `NamedObject` hierarchy.

## Features

- **Hierarchical**: Integrates floating-point values into the object tree.
- **Safe Arithmetic**: Inherits all safe arithmetic checks from `coretypes::FloatingPoint`.
- **Flexible Types**: Supports `float` and `double`.

## API Overview

### Factory Methods

- `create(const std::string &name, T value, std::shared_ptr<NamedObject> parent = nullptr)`

### Core Methods

- `value()`: Returns the primitive value.
- `clone()`: Creates a new named floating-point object with the same value.

## Usage Example

```cpp
#include "quasar/named/NamedFloatingPoint.hpp"

using namespace quasar::named;

auto root = NamedObject::create("sensor_data");
auto temp = NamedFloatingPoint<double>::create("temperature", 25.5, root);

std::cout << temp->getName() << ": " << temp->value() << std::endl;
```
