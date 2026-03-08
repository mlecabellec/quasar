/**
 * @file NamedVariant.hpp
 * @brief Class for holding a dynamically typed NamedObject.
 */

#ifndef QUASAR_NAMED_NAMEDVARIANT_HPP
#define QUASAR_NAMED_NAMEDVARIANT_HPP

#include "quasar/named/NamedObject.hpp"
#include <stdexcept>
#include <string>
#include <mutex>

namespace quasar::named {

/**
 * @class NamedVariant
 * @brief A NamedObject that dynamically wraps exactly one child NamedObject.
 *
 * Provides a dynamic type holder within the NamedObject hierarchy.
 *
 * **Compliance**:
 * - Fulfills [TSK-20260303-002.4] Named Variants.
 */
class NamedVariant : public NamedObject {
public:
  virtual ~NamedVariant() = default;

  /**
   * @brief Factory method.
   */
  static std::shared_ptr<NamedVariant> create(const std::string &name, std::shared_ptr<NamedObject> parent = nullptr) {
    auto obj = std::make_shared<NamedVariant>(name);
    obj->setSelf(obj);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  std::string getType() const override { return "NamedVariant"; }

  /**
   * @brief Sets the active variant object.
   * Replaces any existing child. The new child will be renamed to "value".
   */
  void set(std::shared_ptr<NamedObject> obj) {
    if (!obj) throw std::invalid_argument("Cannot set null variant object");

    std::unique_lock<std::recursive_timed_mutex> lock(m_varMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedVariant lock");

    obj->setName("value");

    if (m_currentObject) {
       replaceChild(m_currentObject, obj);
    } else {
       addChild(obj);
    }
  }

  /**
   * @brief Returns the currently held object.
   */
  std::shared_ptr<NamedObject> get() const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_varMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedVariant lock");
    return m_currentObject;
  }

  /**
   * @brief Type check against the held object.
   */
  template <typename T>
  bool holds() const {
    auto obj = get();
    if (!obj) return false;
    return obj->is<T>();
  }

  /**
   * @brief Casts the held object.
   */
  template <typename T>
  std::shared_ptr<T> getAs() const {
    auto obj = get();
    if (!obj) throw std::runtime_error("Variant is empty");
    return obj->as<T>();
  }

  std::shared_ptr<NamedObject> clone() const override {
    auto cloned = NamedVariant::create(getName());
    auto current = get();
    if (current) {
        cloned->set(current->clone());
    }
    return cloned;
  }

  void addChild(std::shared_ptr<NamedObject> child) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_varMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedVariant lock");
    
    NamedObject::addChild(child);
    m_currentObject = child;
  }

  void removeChild(const std::string &name) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_varMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedVariant lock");
    
    std::string nameCopy = name;
    if (m_currentObject && m_currentObject->getName() == nameCopy) {
        NamedObject::removeChild(nameCopy);
        m_currentObject = nullptr;
    }
  }

  void replaceChild(std::shared_ptr<NamedObject> oldChild, std::shared_ptr<NamedObject> newChild) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_varMutex, std::chrono::milliseconds(100));
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedVariant lock");
    
    NamedObject::replaceChild(oldChild, newChild);
    m_currentObject = newChild;
  }

  // Constructor
  NamedVariant(const std::string &name) : NamedObject(name) {}

private:
  std::shared_ptr<NamedObject> m_currentObject;
  mutable std::recursive_timed_mutex m_varMutex;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDVARIANT_HPP
