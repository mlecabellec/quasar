#include "quasar/named/Traversal.h"
#include <queue>
#include <stack>
#include <algorithm>

namespace quasar::named::traversal {

void forEachDepthFirst(const std::shared_ptr<NamedObject>& root, std::function<void(std::shared_ptr<NamedObject>)> callback) {
    if (!root) return;
    
    std::stack<std::shared_ptr<NamedObject>> stack;
    stack.push(root);
    
    while (!stack.empty()) {
        auto current = stack.top();
        stack.pop();
        
        callback(current);
        
        auto children = current->getChildren();
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            stack.push(*it);
        }
    }
}

void forEachBreadthFirst(const std::shared_ptr<NamedObject>& root, std::function<void(std::shared_ptr<NamedObject>)> callback) {
    if (!root) return;
    
    std::queue<std::shared_ptr<NamedObject>> queue;
    queue.push(root);
    
    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop();
        
        callback(current);
        
        for (const auto& child : current->getChildren()) {
            queue.push(child);
        }
    }
}

std::shared_ptr<NamedObject> findByName(const std::shared_ptr<NamedObject>& root, const std::string& name) {
    std::stack<std::shared_ptr<NamedObject>> stack;
    if (root) stack.push(root);
    
    while (!stack.empty()) {
        auto current = stack.top();
        stack.pop();
        
        if (current->getName() == name) return current;
        
        auto children = current->getChildren();
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            stack.push(*it);
        }
    }
    return nullptr;
}

std::shared_ptr<NamedObject> deepCopy(const std::shared_ptr<NamedObject>& root, std::shared_ptr<NamedObject> newParent) {
    if (!root) return nullptr;
    
    auto newRoot = root->clone();
    if (newParent) {
        newRoot->setParent(newParent);
    }
    
    for (const auto& child : root->getChildren()) {
        deepCopy(child, newRoot);
    }
    
    return newRoot;
}

} // namespace quasar::named::traversal
