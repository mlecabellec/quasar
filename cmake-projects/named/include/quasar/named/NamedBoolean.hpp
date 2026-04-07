/**
 * @file NamedBoolean.hpp
 * @brief Class for named boolean values.
 */

#ifndef QUASAR_NAMED_NAMEDBOOLEAN_HPP
#define QUASAR_NAMED_NAMEDBOOLEAN_HPP

#include "quasar/coretypes/Boolean.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/IBoundPrimitive.hpp"
#include "quasar/named/CopyPolicy.hpp"

namespace quasar::named {

/**
 * @class NamedBoolean
 * @brief A named object that holds a boolean value.
 * 
 * Inherits from NamedObject for hierarchy and coretypes::Boolean for boolean state.
 * 
 * **Compliance**:
 * - Fulfills [FE-0020.4] Derivative class for Boolean core type.
 */
class NamedBoolean : public NamedObject, public quasar::coretypes::Boolean, public IBoundPrimitive {
public:
  /**
   * @brief Destructor.
   */
  virtual ~NamedBoolean() = default;

  /**
   * @brief Factory method to create a new NamedBoolean.
   * 
   * Fulfills [FE-0020.6] static method "create".
   * 
   * @param name The name of the object.
   * @param value The initial boolean value.
   * @param parent Optional parent in the hierarchy.
   * @return A shared_ptr to the newly created NamedBoolean.
   */
  static std::shared_ptr<NamedBoolean>
  create(const std::string &name, bool value,
         std::shared_ptr<NamedObject> parent = nullptr);

  /**
   * @brief Creates a standalone copy of this NamedBoolean.
   * 
   * Fulfills [FE-0020.14] Utilities for copying parts of the tree.
   * 
   * @param policy Memory policy (DUPLICATE vs SHARE).
   * @return A new NamedBoolean with the same name and value, but no hierarchy.
   */
  std::shared_ptr<NamedObject> clone(CopyPolicy policy = CopyPolicy::DUPLICATE) const override {
    if (policy == CopyPolicy::SHARE && m_bound) {
        std::shared_ptr<NamedBoolean> newObj = create(getName(), booleanValue());
        newObj->bind(m_bound_offset, m_bound_length);
        return newObj;
    }
    return create(getName(), booleanValue());
  }

  // --- IBoundPrimitive implementation ---
  bool isBound() const override { return m_bound; }
  std::size_t getBoundOffset() const override { return m_bound_offset; }
  std::size_t getBoundLength() const override { return m_bound_length; }

  /**
   * @brief Binds this boolean to a specific memory offset (conceptual in Phase 1).
   */
  void bind(std::size_t offset, std::size_t length) {
      m_bound = true;
      m_bound_offset = offset;
      m_bound_length = length;
  }

  /**
   * @brief Sets the boolean value and notifies observers if changed.
   * @param value The new value.
   */
  void setValue(bool value) {
      if (quasar::coretypes::Boolean::booleanValue() != value) {
          quasar::coretypes::Boolean::setValue(value);
          notifyObservers(getSelf());
      }
  }


  /**
   * @brief Returns the type of the object.

   * @return "NamedBoolean"
   */
  std::string getType() const override;

  NamedBoolean(const std::string &name, bool value);

private:
  bool m_bound;
  std::size_t m_bound_offset;
  std::size_t m_bound_length;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDBOOLEAN_HPP

