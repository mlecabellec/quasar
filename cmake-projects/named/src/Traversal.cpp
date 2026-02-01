#include "quasar/named/Traversal.hpp"
#include <algorithm>
#include <queue>
#include <stack>

namespace quasar::named::traversal {

void forEachDepthFirst(
    const std::shared_ptr<NamedObject> &root,
    std::function<void(std::shared_ptr<NamedObject>)> callback) {
  if (!root)
    return;

  std::stack<std::shared_ptr<NamedObject>> stack;
  stack.push(root);

  while (!stack.empty()) {
    std::shared_ptr<NamedObject> current = stack.top();
    stack.pop();

    callback(current);

    // Push children in reverse order to process them in correct order.
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
  if (!root)
    return;

  std::queue<std::shared_ptr<NamedObject>> queue;
  queue.push(root);

  while (!queue.empty()) {
    std::shared_ptr<NamedObject> current = queue.front();
    queue.pop();

    callback(current);

    // Enqueue children.
    for (const std::shared_ptr<NamedObject> &child : current->getChildren()) {
      queue.push(child);
    }
  }
}

std::shared_ptr<NamedObject>
findByName(const std::shared_ptr<NamedObject> &root, const std::string &name) {
  std::stack<std::shared_ptr<NamedObject>> stack;
  if (root)
    stack.push(root);

  while (!stack.empty()) {
    std::shared_ptr<NamedObject> current = stack.top();
    stack.pop();

    // Check current node.
    if (current->getName() == name)
      return current;

    // Push children.
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
                                      std::shared_ptr<NamedObject> newParent) {
  if (!root)
    return nullptr;

  std::shared_ptr<NamedObject> newRoot = root->clone();
  if (newParent) {
    newRoot->setParent(newParent);
  }

  // Recursively copy children.
  for (const std::shared_ptr<NamedObject> &child : root->getChildren()) {
    deepCopy(child, newRoot);
  }

  return newRoot;
}

} // namespace quasar::named::traversal
