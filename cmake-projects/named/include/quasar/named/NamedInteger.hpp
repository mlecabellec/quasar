/**
 * @file NamedInteger.hpp
 * @brief Template class for named integer values.
 */

#ifndef QUASAR_NAMED_NAMEDINTEGER_HPP
#define QUASAR_NAMED_NAMEDINTEGER_HPP

#include "quasar/coretypes/Integer.hpp"
#include "quasar/named/NamedObject.hpp"

namespace quasar::named {

/**
 * @class NamedInteger
 * @brief A named object that holds an integer value.
 *
 * This class combines the hierarchical capabilities of NamedObject with the
 * integer value management of coretypes::Integer.
 * 
 * **Compliance**:
 * - Fulfills [FE-0020.4] Derivated class for Integer core type.
 *
 * @tparam T The underlying integer type (e.g., int, uint32_t, int64_t).
 */
template <typename T>
class NamedInteger : public NamedObject, public quasar::coretypes::Integer<T> {
public:
  /**
   * @brief Destructor.
   */
  virtual ~NamedInteger() = default;

  /**
   * @brief Factory method to create a NamedInteger.
   * 
   * Fulfills [FE-0020.6] static method "create".
   *
   * @param name The name of the integer object.
   * @param value The initial integer value.
   * @param parent Optional parent in the hierarchy.
   * @return Shared pointer to the new NamedInteger instance.
   */
  static std::shared_ptr<NamedInteger<T>>
  create(const std::string &name, T value,
         std::shared_ptr<NamedObject> parent = nullptr) {
    // Instantiate the NamedInteger.
    std::shared_ptr<NamedInteger<T>> obj =
        std::make_shared<NamedInteger<T>>(name, value);

    // Initialize self-reference for getSelf().
    obj->setSelf(obj);

    // Attach to parent if provided.
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  /**
   * @brief Creates a clone of this NamedInteger.
   * 
   * Fulfills [FE-0020.14] Utilities for copying parts of the tree.
   * 
   * @return A new NamedInteger with the same name and value, but no hierarchy.
   */
  std::shared_ptr<NamedObject> clone() const override {
    // Return a new instance using the same name and current value.
    return create(this->getName(), this->value());
  }

  /**
   * @brief Constructs a NamedInteger.
   * @param name The name of the object.
   * @param value The initial value.
   */
  NamedInteger(const std::string &name, T value)
      : NamedObject(name), quasar::coretypes::Integer<T>(value) {
    // Both base classes are initialized with the provided arguments.
  }
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDINTEGER_HPP
