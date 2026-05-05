#include "quasar/named/Traversal.hpp"
#include "quasar/named/NamedConfig.hpp"
#include <algorithm>
#include <queue>
#include <stack>

namespace quasar::named::traversal {

void forEachDepthFirst(
    const std::shared_ptr<NamedObject> &root,
    std::function<void(std::shared_ptr<NamedObject>)> callback) {
  // Fulfills [FE-0020.12.1] Depth-first traversal.
  if (!root)
    return;

  // Use a stack for iterative depth-first traversal.
  std::stack<std::shared_ptr<NamedObject>> stack;
  stack.push(root);

  std::size_t iterations = 0;
  while (!stack.empty()) {
    // [CS-0010.37] Loop hard limit.
    if (++iterations > config::HARD_LIMIT_ITERATIONS) {
        throw std::runtime_error("Hard limit reached in forEachDepthFirst");
    }
    
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<NamedObject> current = stack.top();
    stack.pop();

    // Execute the user callback on the current node.
    callback(current);

    // Push children onto the stack in reverse order so they are processed
    // in the original forward order (left-to-right).
    std::list<std::shared_ptr<NamedObject>> children = current->getChildren();
    for (std::list<std::shared_ptr<NamedObject>>::reverse_iterator it =
             children.rbegin();
         it != children.rend(); ++it) {
      stack.push(*it);
    }
  }
}

void forEachBreadthFirst(
    const std::shared_ptr<NamedObject> &root,
    std::function<void(std::shared_ptr<NamedObject>)> callback) {
  // Fulfills [FE-0020.12.1] Breadth-first traversal.
  if (!root)
    return;

  // Use a queue for iterative breadth-first (level-order) traversal.
  std::queue<std::shared_ptr<NamedObject>> queue;
  queue.push(root);

  std::size_t iterations = 0;
  while (!queue.empty()) {
    // [CS-0010.37] Loop hard limit.
    if (++iterations > config::HARD_LIMIT_ITERATIONS) {
        throw std::runtime_error("Hard limit reached in forEachBreadthFirst");
    }
    
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<NamedObject> current = queue.front();
    queue.pop();

    // Execute the user callback.
    callback(current);

    // Enqueue all children for processing in the next levels.
    std::list<std::shared_ptr<NamedObject>> children = current->getChildren();
    for (std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
      const std::shared_ptr<NamedObject> &child = *it;
      queue.push(child);
    }
  }
}

std::shared_ptr<NamedObject>
findByName(const std::shared_ptr<NamedObject> &root, const std::string &name) {
  // Fulfills [FE-0020.13] Search tree by name.
  // Simple iterative search using a stack.
  std::stack<std::shared_ptr<NamedObject>> stack;
  if (root)
    stack.push(root);

  std::size_t iterations = 0;
  while (!stack.empty()) {
    // [CS-0010.37] Loop hard limit.
    if (++iterations > config::HARD_LIMIT_ITERATIONS) {
        throw std::runtime_error("Hard limit reached in findByName");
    }
    
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<NamedObject> current = stack.top();
    stack.pop();

    // Return the node if its name matches the search criteria.
    if (current->getName() == name)
      return current;

    // Continue search in children.
    std::list<std::shared_ptr<NamedObject>> children = current->getChildren();
    for (std::list<std::shared_ptr<NamedObject>>::reverse_iterator it =
             children.rbegin();
         it != children.rend(); ++it) {
      stack.push(*it);
    }
  }
  return nullptr;
}

std::shared_ptr<NamedObject> deepCopy(const std::shared_ptr<NamedObject> &root,
                                      std::shared_ptr<NamedObject> newParent, int maxDepth) {
  // Fulfills [FE-0020.14] Copying parts of the tree.
  if (!root)
    return nullptr;
  if (maxDepth <= 0) throw std::runtime_error("Maximum recursion depth exceeded in deepCopy");

  // Clone the current node (state only, no children/parent yet).
  std::shared_ptr<NamedObject> newRoot = root->clone();
  
  // Link to the provided parent in the new tree.
  if (newParent) {
    newRoot->setParent(newParent);
  }

  // Recursively process and attach all children.
  std::list<std::shared_ptr<NamedObject>> children = root->getChildren();
  for (std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
    const std::shared_ptr<NamedObject> &child = *it;
    deepCopy(child, newRoot, maxDepth - 1);
  }

  return newRoot;
}

} // namespace quasar::named::traversal

