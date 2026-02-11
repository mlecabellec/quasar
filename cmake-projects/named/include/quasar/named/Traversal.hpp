/**
 * @file Traversal.hpp
 * @brief Utilities for traversing and searching NamedObject hierarchies.
 */

#ifndef QUASAR_NAMED_TRAVERSAL_HPP
#define QUASAR_NAMED_TRAVERSAL_HPP

#include "quasar/named/NamedObject.hpp"
#include <functional>
#include <vector>

/**
 * @namespace quasar::named::traversal
 * @brief Namespace for tree traversal algorithms.
 */
namespace quasar::named::traversal {

/**
 * @brief Traverses the tree depth-first (pre-order) invoking a callback on each node.
 * 
 * Thread safety: The caller is responsible for ensuring that the tree structure 
 * is not modified by other threads during traversal.
 * 
 * @param root The root object to start traversal from.
 * @param callback The function to invoke for each visited node.
 */
void forEachDepthFirst(
    const std::shared_ptr<NamedObject> &root,
    std::function<void(std::shared_ptr<NamedObject>)> callback);

/**
 * @brief Traverses the tree breadth-first invoking a callback on each node.
 * 
 * Thread safety: The caller is responsible for ensuring that the tree structure 
 * is not modified by other threads during traversal.
 * 
 * @param root The root object to start traversal from.
 * @param callback The function to invoke for each visited node.
 */
void forEachBreadthFirst(
    const std::shared_ptr<NamedObject> &root,
    std::function<void(std::shared_ptr<NamedObject>)> callback);

/**
 * @brief Finds a descendant by its name using depth-first search.
 * 
 * @param root The root of the tree to search.
 * @param name The name of the object to find.
 * @return Shared pointer to the found object, or nullptr if not found.
 */
std::shared_ptr<NamedObject>
findByName(const std::shared_ptr<NamedObject> &root, const std::string &name);

/**
 * @brief Finds all descendants (including the root) that match a specific type.
 * 
 * @tparam T The type to search for (must derive from NamedObject).
 * @param root The root of the tree to search.
 * @return A vector of shared pointers to all found objects of type T.
 */
template <typename T>
std::vector<std::shared_ptr<T>>
findByType(const std::shared_ptr<NamedObject> &root) {
  std::vector<std::shared_ptr<T>> result;
  // Use depth-first traversal to find matching nodes.
  forEachDepthFirst(root, [&](std::shared_ptr<NamedObject> obj) {
    if (auto casted = std::dynamic_pointer_cast<T>(obj)) {
      result.push_back(casted);
    }
  });
  return result;
}

/**
 * @brief Creates a complete deep copy of an object hierarchy.
 * 
 * This function recursively clones each node in the tree using the virtual `clone()` method.
 * 
 * @param root The root of the tree to copy.
 * @param newParent Optional parent to attach the newly created root to.
 * @return The root of the new copied hierarchy.
 */
std::shared_ptr<NamedObject>
deepCopy(const std::shared_ptr<NamedObject> &root,
         std::shared_ptr<NamedObject> newParent = nullptr);

} // namespace quasar::named::traversal

#endif // QUASAR_NAMED_TRAVERSAL_HPP
