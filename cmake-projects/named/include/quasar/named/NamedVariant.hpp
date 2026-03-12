/**
 * @file NamedVariant.hpp
 * @brief Class for holding a dynamically typed NamedObject.
 */

#ifndef QUASAR_NAMED_NAMEDVARIANT_HPP
#define QUASAR_NAMED_NAMEDVARIANT_HPP

#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedConfig.hpp"
#include <stdexcept>
#include <string>
#include <mutex>
#include "quasar/named/CopyPolicy.hpp"

namespace quasar::named {

/**
 * @class NamedVariant
 * @brief A NamedObject that dynamically wraps exactly one child NamedObject.
 *
 * Provides a dynamic type holder within the NamedObject hierarchy.
 *
 * **Compliance**:
 * - Fulfills [FE-0110.2.2] Named Variants.
 * - Fulfills [CS-0010.34] auto forbidden.
 */
class NamedVariant : public NamedObject {
public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~NamedVariant() = default;

  /**
   * @brief Factory method.
   * @param name The name.
   * @param parent Optional parent.
   * @return Shared pointer to the new variant.
   */
  static std::shared_ptr<NamedVariant> create(const std::string &name, std::shared_ptr<NamedObject> parent = nullptr) {
    std::shared_ptr<NamedVariant> obj = std::make_shared<NamedVariant>(name);
    obj->setSelf(obj);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  /**
   * @brief Returns the type of the object.
   * @return "NamedVariant"
   */
  std::string getType() const override { return "NamedVariant"; }

  /**
   * @brief Sets the active variant object.
   * Replaces any existing child. The new child will be renamed to "value".
   * @param obj The object to set.
   */
  void set(std::shared_ptr<NamedObject> obj) {
    if (!obj) throw std::invalid_argument("Cannot set null variant object");

    std::unique_lock<std::recursive_timed_mutex> lock(m_varMutex, config::DEFAULT_LOCK_TIMEOUT);
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
   * @return The object.
   */
  std::shared_ptr<NamedObject> get() const {
    std::unique_lock<std::recursive_timed_mutex> lock(m_varMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedVariant lock");
    return m_currentObject;
  }

  /**
   * @brief Type check against the held object.
   * @tparam T The type to check for.
   * @return true if matches.
   */
  template <typename T>
  bool holds() const {
    std::shared_ptr<NamedObject> obj = get();
    if (!obj) return false;
    return obj->is<T>();
  }

  /**
   * @brief Casts the held object.
   * @tparam T The type to cast to.
   * @return Shared pointer to the object.
   */
  template <typename T>
  std::shared_ptr<T> getAs() const {
    std::shared_ptr<NamedObject> obj = get();
    if (!obj) throw std::runtime_error("Variant is empty");
    return obj->as<T>();
  }

  /**
   * @brief Clones the variant.
   * @return Cloned object.
   */
  std::shared_ptr<NamedObject> clone([[maybe_unused]] CopyPolicy policy = CopyPolicy::DUPLICATE) const override {
    std::shared_ptr<NamedVariant> cloned = NamedVariant::create(getName());
    std::shared_ptr<NamedObject> current = get();
    if (current) {
        cloned->set(current->clone());
    }
    return cloned;
  }

  /**
   * @brief Internal helper to add a child.
   * @param child The child.
   */
  void addChild(std::shared_ptr<NamedObject> child) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_varMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedVariant lock");
    
    NamedObject::addChild(child);
    m_currentObject = child;
  }

  /**
   * @brief Internal helper to remove a child by name.
   * @param name The name.
   */
  void removeChild(const std::string &name) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_varMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedVariant lock");
    
    std::string nameCopy = name;
    if (m_currentObject && m_currentObject->getName() == nameCopy) {
        NamedObject::removeChild(nameCopy);
        m_currentObject = nullptr;
    }
  }

  /**
   * @brief Internal helper to replace a child.
   * @param oldChild The old child.
   * @param newChild The new child.
   */
  void replaceChild(std::shared_ptr<NamedObject> oldChild, std::shared_ptr<NamedObject> newChild) override {
    std::unique_lock<std::recursive_timed_mutex> lock(m_varMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) throw std::runtime_error("Timeout acquiring NamedVariant lock");
    
    NamedObject::replaceChild(oldChild, newChild);
    m_currentObject = newChild;
  }

  /**
   * @brief Constructor.
   * @param name The name.
   */
  explicit NamedVariant(const std::string &name) : NamedObject(name) {}

private:
  /** @brief Currently held object. */
  std::shared_ptr<NamedObject> m_currentObject;
  /** @brief Mutex for protection. */
  mutable std::recursive_timed_mutex m_varMutex;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDVARIANT_HPP

