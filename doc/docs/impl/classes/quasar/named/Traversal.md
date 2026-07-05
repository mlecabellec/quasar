# Traversal

The `traversal` namespace provides algorithms for visiting, searching, and copying `NamedObject` hierarchies.

## Algorithms

### Tree Traversal

- **Depth-First Search (DFS)**: `forEachDepthFirst(root, callback)` - Visits nodes in pre-order.
- **Breadth-First Search (BFS)**: `forEachBreadthFirst(root, callback)` - Visits nodes level by level.

### Search

- `findByName(root, name)`: Finds a descendant by its unique name using DFS.
- `findByType<T>(root)`: Returns a vector of all descendants (including root) that match type `T`.

### Operations

- `deepCopy(root, newParent = nullptr)`: Recursively clones the entire hierarchy. Use this to create a fully independent duplicate of a tree.

## Thread Safety

Callers must ensure the tree structure is not modified by other threads during traversal.

## Usage Example

```cpp
#include "quasar/named/Traversal.hpp"

using namespace quasar::named;
using namespace quasar::named::traversal;

auto root = // ... a complex tree ...

// Find all integers in the tree
auto allInts = findByType<NamedInteger<int>>(root);

// DFS print
forEachDepthFirst(root, [](auto obj) {
    std::cout << obj->getName() << std::endl;
});
```
