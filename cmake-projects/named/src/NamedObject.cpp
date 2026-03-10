#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedConfig.hpp"
#include <algorithm>
#include <regex>

namespace quasar::named {

bool NamedObject::isValidName(const std::string &name) {
  // Fulfills [FE-0020.1.1.3] Name shall match regex.
  // Regex to match a valid C-style identifier.
  static const std::regex pattern("^[a-zA-Z_][a-zA-Z0-9_]*$");
  return std::regex_match(name, pattern);
}

NamedObject::NamedObject(const std::string &name) : m_name(name) {
  // Fulfills [FE-0020.1.1] "name" property initialized at construction.
  // Validate name according to naming rules.
  if (name.empty()) {
    // Fulfills [FE-0020.1.1.1] Name shall be non empty.
    throw std::runtime_error("Name cannot be empty");
  }
  if (!isValidName(name)) {
    throw std::runtime_error("Invalid name format: " + name);
  }
}

NamedObject::~NamedObject() {
  // Fulfills [FE-0020.1.3.2] Parent destruction releases children.
  // std::list<shared_ptr> handles this automatically.
}

std::shared_ptr<NamedObject>
NamedObject::create(const std::string &name,
                    std::shared_ptr<NamedObject> parent) {
  // Fulfills [FE-0020.6] static method "create".
  // Local helper class to allow make_shared with protected constructor.
  struct Helper : public NamedObject {
    explicit Helper(const std::string &n) : NamedObject(n) {}
  };

  // Create the object and initialize its weak reference to self.
  std::shared_ptr<Helper> obj = std::make_shared<Helper>(name);
  obj->setSelf(obj);

  // If a parent is provided, establish the hierarchy.
  if (parent) {
    obj->setParent(parent);
  }

  return obj;
}

std::shared_ptr<NamedObject> NamedObject::getSelf() const {
  // Lock the weak pointer to obtain a shared_ptr.
  return m_self.lock();
}

void NamedObject::setParent(std::shared_ptr<NamedObject> parent) {
  // Fulfills [FE-0020.5] Operations shall be thread safe.
  {
    // Check if the requested parent is already set.
    std::unique_lock<std::recursive_timed_mutex> lock(m_mutex,
                                                      config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
      throw std::runtime_error("Timeout acquiring NamedObject lock");
    }
    std::shared_ptr<NamedObject> currentParent = m_parent.lock();
    if (currentParent == parent) {
      return;
    }
  }

  // Prevent an object from being its own parent.
  if (parent.get() == this) {
    throw std::runtime_error("Cannot set self as parent");
  }

  // Verify that setting the parent doesn't create a cycle in the tree.
  // [CS-0010.37] Hard limit on loops.
  std::shared_ptr<NamedObject> p = parent;
  std::size_t iterations = 0;
  while (p) {
    if (++iterations > config::HARD_LIMIT_ITERATIONS) {
      throw std::runtime_error("Hard limit reached in parent hierarchy check");
    }
    if (p.get() == this) {
      throw std::runtime_error("Cycle detected in parent hierarchy");
    }
    p = p->getParent();
  }

  std::shared_ptr<NamedObject> oldParent;

  // Add this object as a child of the new parent.
  if (parent) {
    // Fulfills [FE-0020.1.2.2] Added to parent's children list.
    // Attempt to add this object as a child of the new parent.
    // This will throw if there's a name collision.
    parent->addChild(getSelf());
  }

  // Atomically update the parent reference.
  {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mutex,
                                                      config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
      throw std::runtime_error("Timeout acquiring NamedObject lock");
    }
    oldParent = m_parent.lock();
    m_parent = parent;
  }

  // Remove this object from the old parent's child list.
  if (oldParent) {
    try {
      // Fulfills [FE-0020.1.3.4] removed from old parent list.
      oldParent->removeChild(m_name);
    } catch (...) {
      // Ignore errors during removal from old parent to maintain consistency.
    }
  }
}

void NamedObject::addChild(std::shared_ptr<NamedObject> child) {
  // Fulfills [FE-0020.1.3.1] strong reference to children.
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex,
                                                    config::DEFAULT_LOCK_TIMEOUT);
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  
  // Ensure that all children have unique names.
  // [CS-0010.34] auto forbidden.
  for (std::list<std::shared_ptr<NamedObject>>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
    const std::shared_ptr<NamedObject> &c = *it;
    if (c->getName() == child->getName()) {
      if (c == child)
        return; // The object is already a child.
      // Fulfills [FE-0020.1.1.2] Unique name within parent.
      // Fulfills [FE-0020.1.2.3] Fail if name not unique in new parent.
      throw std::runtime_error("Name not unique in parent: " +
                               child->getName());
    }
  }
  m_children.push_back(child);
}

void NamedObject::removeChild(const std::string &name) {
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex,
                                                    config::DEFAULT_LOCK_TIMEOUT);
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  
  // Take a local copy of the name string to avoid dangling reference if the 
  // child is destroyed during removal.
  std::string nameCopy = name;

  // Remove any child with the matching name.
  m_children.remove_if([&nameCopy](const std::shared_ptr<NamedObject> &c) {
    return c->getName() == nameCopy;
  });
}

void NamedObject::replaceChild(std::shared_ptr<NamedObject> oldChild,
                               std::shared_ptr<NamedObject> newChild) {
  std::unique_lock<std::recursive_timed_mutex> lock(
      m_mutex, config::DEFAULT_LOCK_TIMEOUT);
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }

  // Find the old child in the list.
  std::list<std::shared_ptr<NamedObject>>::iterator it =
      std::find(m_children.begin(), m_children.end(), oldChild);

  if (it != m_children.end()) {
    // Replace it with the new child.
    *it = newChild;
  } else {
    throw std::runtime_error("Old child not found in parent: " +
                             oldChild->getName());
  }
}

std::string NamedObject::getName() const { return m_name; }

void NamedObject::setName(const std::string &name) {
  // Validate the new name according to naming rules.
  if (name.empty()) {
    throw std::runtime_error("Name cannot be empty");
  }
  if (!isValidName(name)) {
    throw std::runtime_error("Invalid name format: " + name);
  }

  // Acquire local lock to modify name.
  std::unique_lock<std::recursive_timed_mutex> lock(
      m_mutex, config::DEFAULT_LOCK_TIMEOUT);
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }

  if (m_name == name) {
    return;
  }

  // Need to check for name uniqueness in the parent before renaming.
  std::shared_ptr<NamedObject> parent = m_parent.lock();
  if (parent) {
    std::unique_lock<std::recursive_timed_mutex> parentLock(
        parent->m_mutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!parentLock.owns_lock()) {
      throw std::runtime_error("Timeout acquiring parent lock for rename");
    }
    // [CS-0010.34] auto forbidden.
    for (std::list<std::shared_ptr<NamedObject>>::iterator it = parent->m_children.begin(); it != parent->m_children.end(); ++it) {
      const std::shared_ptr<NamedObject> &child = *it;
      if (child.get() != this && child->getName() == name) {
        throw std::runtime_error("Name not unique in parent: " + name);
      }
    }
    // Rename is safe
    m_name = name;
  } else {
    // No parent, just rename
    m_name = name;
  }
}


std::string NamedObject::getType() const { return "NamedObject"; }

void NamedObject::replaceInTree(std::shared_ptr<NamedObject> replacement) {
  // Fulfills [FE-0110.1.1] Tree Substitution.
  if (!replacement) {
    throw std::runtime_error("Replacement cannot be null");
  }

  std::unique_lock<std::recursive_timed_mutex> lock(
      m_mutex, config::DEFAULT_LOCK_TIMEOUT);
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }

  // Update parent reference if it exists.
  std::shared_ptr<NamedObject> parent = m_parent.lock();
  if (parent) {
    // Replace this in parent's child list.
    parent->replaceChild(getSelf(), replacement);
  }

  // Hand over all children.
  std::list<std::shared_ptr<NamedObject>> children;
  {
    // Snapshot children and clear local list.
    children = m_children;
    m_children.clear();
  }

  // Re-parent each child to the replacement.
  // [CS-0010.34] auto forbidden.
  for (std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
    const std::shared_ptr<NamedObject> &child = *it;
    child->setParent(replacement);
  }

  // Update replacement's parent.
  replacement->setParent(parent);

  // Clear our own parent reference now that we are detached.
  {
      std::unique_lock<std::recursive_timed_mutex> lock2(
          m_mutex, config::DEFAULT_LOCK_TIMEOUT);
      if (lock2.owns_lock()) {
          m_parent.reset();
      }
  }
}

std::shared_ptr<NamedObject> NamedObject::getParent() const {
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex,
                                                    config::DEFAULT_LOCK_TIMEOUT);
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  return m_parent.lock();
}

std::list<std::shared_ptr<NamedObject>> NamedObject::getChildren() const {
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex,
                                                    config::DEFAULT_LOCK_TIMEOUT);
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  return m_children;
}

std::shared_ptr<NamedObject> NamedObject::getChild(const std::string &name) const {
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex,
                                                    config::DEFAULT_LOCK_TIMEOUT);
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  // Search for child by name.
  // [CS-0010.34] auto forbidden.
  for (std::list<std::shared_ptr<NamedObject>>::const_iterator it = m_children.begin(); it != m_children.end(); ++it) {
    const std::shared_ptr<NamedObject> &child = *it;
    if (child->getName() == name) {
      return child;
    }
  }
  return nullptr;
}

std::shared_ptr<NamedObject> NamedObject::getPreviousSibling() const {
  std::shared_ptr<NamedObject> p = getParent();
  if (!p)
    return nullptr;

  // Siblings are stored in the parent's child list.
  // We lock the parent to safely traverse the list.
  std::unique_lock<std::recursive_timed_mutex> lock(p->m_mutex,
                                                    config::DEFAULT_LOCK_TIMEOUT);
  if (!lock.owns_lock()) {
    throw std::runtime_error(
        "Timeout acquiring parent NamedObject lock in getPreviousSibling");
  }
  std::list<std::shared_ptr<NamedObject>> &siblings = p->m_children;
  std::list<std::shared_ptr<NamedObject>>::iterator it =
      std::find(siblings.begin(), siblings.end(), getSelf());

  // If found and not the first element, return the previous one.
  if (it != siblings.begin() && it != siblings.end()) {
    return *std::prev(it);
  }
  return nullptr;
}

std::shared_ptr<NamedObject> NamedObject::getNextSibling() const {
  std::shared_ptr<NamedObject> p = getParent();
  if (!p)
    return nullptr;

  // Siblings are stored in the parent's child list.
  // We lock the parent to safely traverse the list.
  std::unique_lock<std::recursive_timed_mutex> lock(p->m_mutex,
                                                    config::DEFAULT_LOCK_TIMEOUT);
  if (!lock.owns_lock()) {
    throw std::runtime_error(
        "Timeout acquiring parent NamedObject lock in getNextSibling");
  }
  const std::list<std::shared_ptr<NamedObject>> &siblings = p->m_children;
  std::list<std::shared_ptr<NamedObject>>::const_iterator it =
      std::find(siblings.begin(), siblings.end(), getSelf());

  // If found and not the last element, return the next one.
  if (it != siblings.end() && std::next(it) != siblings.end()) {
    return *std::next(it);
  }
  return nullptr;
}

std::shared_ptr<NamedObject> NamedObject::getFirstChild() const {
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex,
                                                    config::DEFAULT_LOCK_TIMEOUT);
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  if (m_children.empty())
    return nullptr;
  return m_children.front();
}

std::shared_ptr<NamedObject> NamedObject::getLastChild() const {
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex,
                                                    config::DEFAULT_LOCK_TIMEOUT);
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  if (m_children.empty())
    return nullptr;
  return m_children.back();
}

void NamedObject::setRelated(std::shared_ptr<NamedObject> related) {
  // Fulfills [FE-0020.7] Support a "related" property.
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex,
                                                    config::DEFAULT_LOCK_TIMEOUT);
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  m_related = related;
}

std::shared_ptr<NamedObject> NamedObject::getRelated() const {
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex,
                                                    config::DEFAULT_LOCK_TIMEOUT);
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  return m_related.lock();
}

bool NamedObject::operator==(const NamedObject &other) const {
  // Simple equality based on the object's name.
  return m_name == other.m_name;
}

bool NamedObject::operator<(const NamedObject &other) const {
  // Lexicographical order based on the object's name.
  return m_name < other.m_name;
}

std::shared_ptr<NamedObject> NamedObject::clone() const {
  // Default implementation creates a new NamedObject with the same name.
  return NamedObject::create(m_name);
}

std::shared_ptr<NamedObject> NamedObject::deepCopy() const {
  // Fulfills [FE-0020.14.1] deep copy mechanism.
  // Initiates a deep copy down the hierarchy starting with no parent context.
  return deepCopy(nullptr, nullptr);
}

std::shared_ptr<NamedObject> NamedObject::deepCopy(
    [[maybe_unused]] std::shared_ptr<NamedObject> originalParent,
    std::shared_ptr<NamedObject> newParent) const {

  // Clone the current node (shallow copy, unattached).
  std::shared_ptr<NamedObject> clonedObj = clone();

  // Attach to the copy's new parent if provided.
  if (newParent) {
    clonedObj->setParent(newParent);
  }

  // Snap children concurrently safely.
  std::list<std::shared_ptr<NamedObject>> children;
  {
    std::unique_lock<std::recursive_timed_mutex> lock(m_mutex,
                                                      config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
      throw std::runtime_error(
          "Timeout acquiring NamedObject lock during deepCopy");
    }
    children = m_children;
  }

  // Iterate over children and recursively copy them. Deep copy attaches
  // themselves automatically.
  // [CS-0010.34] auto forbidden.
  for (std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
    const std::shared_ptr<NamedObject> &child = *it;
    child->deepCopy(getSelf(), clonedObj);
  }

  return clonedObj;
}

} // namespace quasar::named

