# Serialization

The `serialization` namespace provides utilities for converting `NamedObject` hierarchies to and from standard text formats: XML, YAML, and JSON.

## Supported Formats

- **XML**: Hierarchical tags mapping to object names and types.
- **YAML**: Human-readable indentation-based format.
- **JSON**: Standard data interchange format.

## API Overview

All functions are located in `namespace quasar::named::serialization`.

### XML

- `toXml(const std::shared_ptr<NamedObject> &obj)`: Serializes a tree to an XML string.
- `fromXml(const std::string &xml)`: Reconstructs a tree from an XML string.

### YAML

- `toYaml(const std::shared_ptr<NamedObject> &obj)`: Serializes a tree to a YAML string.
- `fromYaml(const std::string &yaml)`: Reconstructs a tree from a YAML string.

### JSON

- `toJson(const std::shared_ptr<NamedObject> &obj)`: Serializes a tree to a JSON string.
- `fromJson(const std::string &json)`: Reconstructs a tree from a JSON string.

## Usage Example

```cpp
#include "quasar/named/Serialization.hpp"

using namespace quasar::named;

auto root = NamedObject::create("root");
// ... add children ...

std::string xml = serialization::toXml(root);
auto restored = serialization::fromXml(xml);
```
