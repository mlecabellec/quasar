# String

The `String` class is an immutable wrapper for `std::string`, providing a consistent API within the Quasar framework and ensuring thread-safe sharing of string data.

## Key Features

- **Immutability**: The internal string value cannot be modified after construction.
- **Thread-Safety**: Safe for concurrent read operations across threads.
- **Easy Integration**: Provides simple conversion to and from `std::string`.

## API Overview

### Constructors

- `String()`: Initializes an empty string.
- `String(const std::string &s)`: Wraps a `std::string`.
- `String(const char *s)`: Wraps a C-style string.

### Core Methods

- `toString()`: Returns a copy of the underlying `std::string`.
- `value()`: Returns a constant reference to the internal `std::string`.
- `length()`: Returns the number of characters.
- `empty()`: Returns `true` if empty.

### Comparison

- `equals(const String &other)`: Checks for identity of content.
- `compareTo(const String &other)`: Lexicographical comparison.

## Usage Example

```cpp
#include "quasar/coretypes/String.hpp"

using namespace quasar::coretypes;

String msg("Hello Quasar");
if (!msg.empty()) {
    std::cout << "Length: " << msg.length() << std::endl;
}

String other("Hello Quasar");
if (msg.equals(other)) {
    // Strings are equal
}
```
