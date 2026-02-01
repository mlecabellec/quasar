#ifndef QUASAR_NAMED_TRAVERSAL_H
#define QUASAR_NAMED_TRAVERSAL_H

#include "quasar/named/NamedObject.h"
#include <functional>
#include <vector>

namespace quasar::named::traversal {

/**
 * @brief Traverses the tree depth-first (pre-order) invoking callback on each
 * node. Thread safety is caller's responsibility.
 * @param root The root object to start traversal from.
 * @param callback The callback function to invoke for each node.
 */
void forEachDepthFirst(
    const std::shared_ptr<NamedObject> &root,
    std::function<void(std::shared_ptr<NamedObject>)> callback);

/**
 * @brief Traverses the tree breadth-first invoking callback on each node.
 * Thread safety is caller's responsibility.
 * @param root The root object to start traversal from.
 * @param callback The callback function to invoke for each node.
 */
void forEachBreadthFirst(
    const std::shared_ptr<NamedObject> &root,
    std::function<void(std::shared_ptr<NamedObject>)> callback);

/**
 * @brief Finds a descendant by name (depth-first search).
 * @param root The root of the tree to search.
 * @param name The name of the object to find.
 * @return Shared pointer to the found object, or null if not found.
 */
std::shared_ptr<NamedObject>
findByName(const std::shared_ptr<NamedObject> &root, const std::string &name);

/**
 * @brief Finds all descendants of a specific type.
 * @tparam T The type to search for (must derive from NamedObject).
 * @param root The root of the tree to search.
 * @return Vector of shared pointers to found objects.
 */
template <typename T>
std::vector<std::shared_ptr<T>>
findByType(const std::shared_ptr<NamedObject> &root) {
  std::vector<std::shared_ptr<T>> result;
  forEachDepthFirst(root, [&](std::shared_ptr<NamedObject> obj) {
    if (auto casted = std::dynamic_pointer_cast<T>(obj)) {
      result.push_back(casted);
    }
  });
  return result;
}

/**
 * @brief Creates a deep copy of the tree.
 * @param root The root of the tree to copy.
 * @param newParent Optional parent for the new root.
 * @return The new root.
 * Note: Cloning specific derived types (like NamedInteger) requires support in
 * create/clone. Since we rely on factories, generic deepCopy is hard without
 * virtual clone(). We will implement structural copy for NamedObject base, but
 * for derived types we need a clone method. Spec said "Utilities shall be
 * provided for copying...". If NamedObject doesn't have virtual clone, we can't
 * deep copy derived state easily. I will implement a basic structural copy that
 * preserves names/structure but effectively degrades to NamedObject if no clone
 * facility. However, to be compliant, I should probably have added `clone` to
 * NamedObject. Let's see if I can add it now or just do best effort. I'll add a
 * virtual `clone` method to `NamedObject` and override in derived classes? That
 * would be best.
 */
std::shared_ptr<NamedObject>
deepCopy(const std::shared_ptr<NamedObject> &root,
         std::shared_ptr<NamedObject> newParent = nullptr);

} // namespace quasar::named::traversal

#endif // QUASAR_NAMED_TRAVERSAL_H
