/**
 * @file NamedBoolean.hpp
 * @brief Class for named boolean values.
 */

#ifndef QUASAR_NAMED_NAMEDBOOLEAN_HPP
#define QUASAR_NAMED_NAMEDBOOLEAN_HPP

#include "quasar/coretypes/Boolean.hpp"
#include "quasar/named/NamedObject.hpp"

namespace quasar::named {

/**
 * @class NamedBoolean
 * @brief A named object that holds a boolean value.
 * 
 * Inherits from NamedObject for hierarchy and coretypes::Boolean for boolean state.
 * 
 * **Compliance**:
 * - Fulfills [FE-0020.4] Derivated class for Boolean core type.
 */
class NamedBoolean : public NamedObject, public quasar::coretypes::Boolean {
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
   * @return A new NamedBoolean with the same name and value, but no hierarchy.
   */
  std::shared_ptr<NamedObject> clone() const override {
    return create(getName(), booleanValue());
  }

  /**
   * @brief Returns the type of the object.
   * @return "NamedBoolean"
   */
  std::string getType() const override;

  /**
   * @brief Constructs a NamedBoolean instance.
   * @param name The name of the object.
   * @param value The initial boolean value.
   */
  NamedBoolean(const std::string &name, bool value);
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDBOOLEAN_HPP
