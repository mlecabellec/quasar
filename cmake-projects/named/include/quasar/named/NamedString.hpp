/**
 * @file NamedString.hpp
 * @brief Class for named string values.
 */

#ifndef QUASAR_NAMED_NAMEDSTRING_HPP
#define QUASAR_NAMED_NAMEDSTRING_HPP

#include "quasar/coretypes/String.hpp"
#include "quasar/coretypes/String.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/CopyPolicy.hpp"

namespace quasar::named {

/**
 * @class NamedString
 * @brief A named object that holds a string value.
 * 
 * This class inherits from NamedObject for hierarchy management and 
 * coretypes::String for string value operations.
 * 
 * **Compliance**:
 * - Fulfills [FE-0030.4] support for a named String.
 * - Fulfills [FE-0020.4] Derivative of NamedObject.
 */
class NamedString : public NamedObject, public quasar::coretypes::String {
public:
  /**
   * @brief Destructor.
   */
  virtual ~NamedString() = default;

  /**
   * @brief Factory method to create a new NamedString.
   * 
   * Fulfills [FE-0020.6] static method "create".
   * 
   * @param name The name of the object.
   * @param value The initial string value.
   * @param parent Optional parent in the hierarchy.
   * @return A shared_ptr to the newly created NamedString.
   */
  static std::shared_ptr<NamedString>
  create(const std::string &name, const std::string &value,
         std::shared_ptr<NamedObject> parent = nullptr);

  /**
   * @brief Creates a standalone copy of this NamedString.
   * 
   * Fulfills [FE-0020.14] Utilities for copying parts of the tree.
   * 
   * @param policy Memory policy (DUPLICATE vs SHARE).
   * @return A new NamedString with the same name and value, but no hierarchy.
   */
  std::shared_ptr<NamedObject> clone([[maybe_unused]] CopyPolicy policy = CopyPolicy::DUPLICATE) const override {
    // Strings are typically small and simple enough to just duplicate in memory,
    // as sharing strings isn't usually a bottleneck here unless explicitly needed.
    // For Phase 1 we will just duplicate.
    return create(getName(), toString());
  }

  /**
   * @brief Sets the string value and notifies observers.
   * @param value The new value.
   */
  void setValue(const std::string& value) {
      quasar::coretypes::String::setValue(value);
      notifyObservers(getSelf());
  }

  /**
   * @brief Returns the type of the object.

   * @return "NamedString"
   */
  std::string getType() const override;

  /**
   * @brief Constructs a NamedString instance.
   * @param name The name of the object.
   * @param value The initial string value.
   */
  NamedString(const std::string &name, const std::string &value);
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDSTRING_HPP

