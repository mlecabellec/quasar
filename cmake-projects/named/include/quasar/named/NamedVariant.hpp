/**
 * @file NamedVariant.hpp
 * @brief Class for holding a named dynamically typed value.
 */

#ifndef QUASAR_NAMED_NAMEDVARIANT_HPP
#define QUASAR_NAMED_NAMEDVARIANT_HPP

#include "quasar/named/NamedObject.hpp"
#include "quasar/coretypes/Variant.hpp"
#include "quasar/named/NamedConfig.hpp"
#include <stdexcept>
#include <string>
#include <mutex>
#include "quasar/named/CopyPolicy.hpp"

namespace quasar::named {

/**
 * @class NamedVariant
 * @brief A NamedObject that encapsulates a type-safe Variant value.
 *
 * This class combines the hierarchical capabilities of NamedObject with 
 * the dynamic value management of coretypes::Variant. It replaces the previous 
 * implementation that wrapped a child NamedObject, adhering to the 
 * primitive-inheritance model used by NamedInteger and NamedBoolean.
 *
 * **Compliance**:
 * - Fulfills [FE-0110.2.2] Named Variants.
 * - Fulfills [CS-0010.34] auto forbidden.
 * - Fulfills [CS-0010.45] Doxygen documentation.
 */
class NamedVariant : public NamedObject, public coretypes::Variant {
public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~NamedVariant() = default;

  /**
   * @brief Factory method.
   * @param name The name of the object.
   * @param variant Initial variant value.
   * @param parent Optional parent in the hierarchy.
   * @return Shared pointer to the new variant instance.
   */
  static std::shared_ptr<NamedVariant> create(const std::string &name, 
                                            const coretypes::Variant& variant = coretypes::Variant(),
                                            std::shared_ptr<NamedObject> parent = nullptr) {
    // [CS-0010.44] Explicit construction and initialization.
    std::shared_ptr<NamedVariant> obj = std::make_shared<NamedVariant>(name, variant);
    obj->setSelf(obj);
    if (parent != nullptr) {
      obj->setParent(parent);
    }
    return obj;
  }

  /**
   * @brief Returns the type name of the object.
   * @return "NamedVariant"
   */
  std::string getType() const override { return "NamedVariant"; }

  /**
   * @brief Sets the variant value and notifies observers.
   * @param v The new Variant value.
   * @throws std::runtime_error if mutex acquisition times out.
   */
  void setVariant(const coretypes::Variant& v) {
      // [CS-0010.21] RAII for mutex.
      std::unique_lock<std::recursive_timed_mutex> lock(m_varMutex, config::DEFAULT_LOCK_TIMEOUT);
      if (!lock.owns_lock()) {
           throw std::runtime_error("Timeout acquiring NamedVariant lock");
      }
      
      // [CS-0010.44] Updating the underlying Variant value.
      coretypes::Variant::operator=(v);
      
      // [CS-0010.44] Notifying observers of the state change.
      notifyObservers(getSelf());
  }

  /**
   * @brief Returns a copy of the currently held variant value.
   * @return The held coretypes::Variant.
   * @throws std::runtime_error if mutex acquisition times out.
   */
  coretypes::Variant getVariant() const {
      // [CS-0010.21] RAII for mutex.
      std::unique_lock<std::recursive_timed_mutex> lock(m_varMutex, config::DEFAULT_LOCK_TIMEOUT);
      if (!lock.owns_lock()) {
           throw std::runtime_error("Timeout acquiring NamedVariant lock");
      }
      return static_cast<const coretypes::Variant&>(*this);
  }

  /**
   * @brief Creates a standalone copy of the variant object.
   * @param policy Copy policy (DUPLICATE vs SHARE).
   * @return Shared pointer to the cloned object.
   */
  std::shared_ptr<NamedObject> clone([[maybe_unused]] CopyPolicy policy = CopyPolicy::DUPLICATE) const override {
    // [CS-0010.44] Creating a new NamedVariant with identical name and value.
    return create(getName(), getVariant());
  }

  /**
   * @brief Protected constructor to enforce factory usage.
   * @param name The name of the object.
   * @param variant Initial variant value.
   */
  NamedVariant(const std::string &name, const coretypes::Variant& variant) 
      : NamedObject(name), coretypes::Variant(variant) {}

private:
  /** @brief Mutex for protection of the variant value. */
  mutable std::recursive_timed_mutex m_varMutex;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDVARIANT_HPP
