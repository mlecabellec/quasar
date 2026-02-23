#include "quasar/named/NamedObject.hpp"
#include <algorithm>
#include <iostream>
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
    std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, std::chrono::seconds(1));
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
  std::shared_ptr<NamedObject> p = parent;
  while (p) {
    if (p.get() == this) {
      throw std::runtime_error("Cycle detected in parent hierarchy");
    }
    p = p->getParent();
  }

  std::shared_ptr<NamedObject> oldParent;

  if (parent) {
    // Fulfills [FE-0020.1.2.2] Added to parent's children list.
    // Attempt to add this object as a child of the new parent.
    // This will throw if there's a name collision.
    parent->addChild(getSelf());
  }

  {
    // Atomically update the parent reference.
    std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, std::chrono::seconds(1));
    if (!lock.owns_lock()) {
      throw std::runtime_error("Timeout acquiring NamedObject lock");
    }
    oldParent = m_parent.lock();
    m_parent = parent;
  }

  if (oldParent) {
    try {
      // Fulfills [FE-0020.1.3.4] removed from old parent list.
      // Remove this object from the old parent's child list.
      oldParent->removeChild(m_name);
    } catch (...) {
      // Ignore errors during removal from old parent to maintain consistency.
    }
  }
}

void NamedObject::addChild(std::shared_ptr<NamedObject> child) {
  // Fulfills [FE-0020.1.3.1] strong reference to children.
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  // Ensure that all children have unique names.
  for (const std::shared_ptr<NamedObject> &c : m_children) {
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
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  // Remove any child with the matching name.
  m_children.remove_if([&name](const std::shared_ptr<NamedObject> &c) {
    return c->getName() == name;
  });
}

std::string NamedObject::getName() const { return m_name; }

std::shared_ptr<NamedObject> NamedObject::getParent() const {
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  return m_parent.lock();
}

std::list<std::shared_ptr<NamedObject>> NamedObject::getChildren() const {
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  return m_children;
}

std::shared_ptr<NamedObject> NamedObject::getPreviousSibling() const {
  std::shared_ptr<NamedObject> p = getParent();
  if (!p)
    return nullptr;

  // Siblings are stored in the parent's child list.
  // We lock the parent to safely traverse the list.
  std::unique_lock<std::recursive_timed_mutex> lock(p->m_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring parent NamedObject lock in getPreviousSibling");
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
  std::unique_lock<std::recursive_timed_mutex> lock(p->m_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring parent NamedObject lock in getNextSibling");
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
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  if (m_children.empty())
    return nullptr;
  return m_children.front();
}

std::shared_ptr<NamedObject> NamedObject::getLastChild() const {
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  if (m_children.empty())
    return nullptr;
  return m_children.back();
}

void NamedObject::setRelated(std::shared_ptr<NamedObject> related) {
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, std::chrono::seconds(1));
  if (!lock.owns_lock()) {
    throw std::runtime_error("Timeout acquiring NamedObject lock");
  }
  m_related = related;
}

std::shared_ptr<NamedObject> NamedObject::getRelated() const {
  std::unique_lock<std::recursive_timed_mutex> lock(m_mutex, std::chrono::seconds(1));
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

} // namespace quasar::named
