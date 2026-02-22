# NamedString

`NamedString` is a `NamedObject` that holds an immutable string value, inheriting from `coretypes::String`.

## Features

- **Hierarchical**: Can be used as a leaf or node in a `NamedObject` tree.
- **Immutable**: Thread-safe sharing of string data.

## API Overview

### Factory Methods

- `create(const std::string &name, const std::string &value, std::shared_ptr<NamedObject> parent = nullptr)`

### Core Methods

- `clone()`: Creates a new named string with the same content.
- Inherits `length()`, `empty()`, `equals()`, `compareTo()`, and `toString()` from `String`.

## Usage Example

```cpp
#include "quasar/named/NamedString.hpp"

using namespace quasar::named;

auto root = NamedObject::create("system");
auto version = NamedString::create("version", "1.0.4-beta", root);

std::cout << version->getName() << ": " << version->toString() << std::endl;
```
