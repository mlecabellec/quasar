#include "quasar/named/NamedObject.h"
#include <regex>
#include <iostream>
#include <algorithm>

namespace quasar::named {

bool NamedObject::isValidName(const std::string& name) {
    static const std::regex pattern("^[a-zA-Z_][a-zA-Z0-9_]*$");
    return std::regex_match(name, pattern);
}

NamedObject::NamedObject(const std::string& name) : m_name(name) {
    if (name.empty()) {
        throw std::runtime_error("Name cannot be empty");
    }
    if (!isValidName(name)) {
        throw std::runtime_error("Invalid name format: " + name);
    }
}

NamedObject::~NamedObject() {
}

std::shared_ptr<NamedObject> NamedObject::create(const std::string& name, std::shared_ptr<NamedObject> parent) {
    struct Helper : public NamedObject {
        Helper(const std::string& n) : NamedObject(n) {}
    };
    
    auto obj = std::make_shared<Helper>(name);
    
    if (parent) {
        obj->setParent(parent);
    }
    
    return obj;
}

void NamedObject::setParent(std::shared_ptr<NamedObject> parent) {
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        auto currentParent = m_parent.lock();
        if (currentParent == parent) {
            return;
        }
    }

    if (parent.get() == this) {
        throw std::runtime_error("Cannot set self as parent");
    }

    // Check for cycles
    auto p = parent;
    while(p) {
        if (p.get() == this) {
            throw std::runtime_error("Cycle detected in parent hierarchy");
        }
        p = p->getParent();
    }

    std::shared_ptr<NamedObject> oldParent;
    
    if (parent) {
        // Add to new parent. This might throw if name duplicate.
        parent->addChild(shared_from_this());
    }
    
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        oldParent = m_parent.lock();
        m_parent = parent;
    }
    
    if (oldParent) {
        try {
            oldParent->removeChild(m_name);
        } catch(...) {
            // Should not happen if logic is consistent
        }
    }
}

void NamedObject::addChild(std::shared_ptr<NamedObject> child) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    for (const auto& c : m_children) {
        if (c->getName() == child->getName()) {
            if (c == child) return; // Already child
            throw std::runtime_error("Name not unique in parent: " + child->getName());
        }
    }
    m_children.push_back(child);
}

void NamedObject::removeChild(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_children.remove_if([&name](const std::shared_ptr<NamedObject>& c) {
        return c->getName() == name;
    });
}

std::string NamedObject::getName() const {
    return m_name;
}

std::shared_ptr<NamedObject> NamedObject::getParent() const {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_parent.lock();
}

std::list<std::shared_ptr<NamedObject>> NamedObject::getChildren() const {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_children;
}

std::shared_ptr<NamedObject> NamedObject::getPreviousSibling() const {
    auto p = getParent();
    if (!p) return nullptr;
    
    // We need to access parent's children. 
    // We should lock parent.
    // However, we can't access m_mutex of p directly if it's private?
    // It is private. But we are NamedObject, so we can access private members of other NamedObjects.
    std::lock_guard<std::recursive_mutex> lock(p->m_mutex);
    auto& siblings = p->m_children;
    auto it = std::find(siblings.begin(), siblings.end(), shared_from_this());
    
    if (it != siblings.begin() && it != siblings.end()) {
        return *std::prev(it);
    }
    return nullptr;
}

std::shared_ptr<NamedObject> NamedObject::getNextSibling() const {
    auto p = getParent();
    if (!p) return nullptr;
    
    std::lock_guard<std::recursive_mutex> lock(p->m_mutex);
    auto& siblings = p->m_children;
    auto it = std::find(siblings.begin(), siblings.end(), shared_from_this());
    
    if (it != siblings.end() && std::next(it) != siblings.end()) {
        return *std::next(it);
    }
    return nullptr;
}

std::shared_ptr<NamedObject> NamedObject::getFirstChild() const {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (m_children.empty()) return nullptr;
    return m_children.front();
}

std::shared_ptr<NamedObject> NamedObject::getLastChild() const {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (m_children.empty()) return nullptr;
    return m_children.back();
}

void NamedObject::setRelated(std::shared_ptr<NamedObject> related) {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    m_related = related;
}

std::shared_ptr<NamedObject> NamedObject::getRelated() const {
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    return m_related.lock();
}

bool NamedObject::operator==(const NamedObject& other) const {
    return m_name == other.m_name;
}

bool NamedObject::operator<(const NamedObject& other) const {
    return m_name < other.m_name;
}

std::shared_ptr<NamedObject> NamedObject::clone() const {
    return NamedObject::create(m_name);
}

} // namespace quasar::named
