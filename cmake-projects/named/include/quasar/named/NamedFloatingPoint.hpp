/**
 * @file NamedFloatingPoint.hpp
 * @brief Template class for named floating point values.
 */

#ifndef QUASAR_NAMED_NAMEDFLOATINGPOINT_HPP
#define QUASAR_NAMED_NAMEDFLOATINGPOINT_HPP

#include "quasar/coretypes/FloatingPoint.hpp"
#include "quasar/named/NamedObject.hpp"

namespace quasar::named {

/**
 * @class NamedFloatingPoint
 * @brief A named object that holds a floating point value.
 *
 * This class combines hierarchical management from NamedObject with floating
 * point value handling from coretypes::FloatingPoint.
 * 
 * **Compliance**:
 * - Fulfills [FE-0020.4] Derivative class for FloatingPoint core type.
 * - Fulfills [CS-0010.34] auto forbidden.
 *
 * @tparam T The underlying floating point type (e.g., float, double).
 */
template <typename T>
class NamedFloatingPoint : public NamedObject,
                           public quasar::coretypes::FloatingPoint<T> {
public:
  /**
   * @brief Destructor.
   */
  virtual ~NamedFloatingPoint() = default;

  /**
   * @brief Factory method to create a NamedFloatingPoint.
   * 
   * Fulfills [FE-0020.6] static method "create".
   *
   * @param name The name of the floating point object.
   * @param value The initial floating point value.
   * @param parent Optional parent in the hierarchy.
   * @return Shared pointer to the new NamedFloatingPoint instance.
   */
  static std::shared_ptr<NamedFloatingPoint<T>>
  create(const std::string &name, T value,
         std::shared_ptr<NamedObject> parent = nullptr) {
    // Instantiate the NamedFloatingPoint.
    std::shared_ptr<NamedFloatingPoint<T>> obj =
        std::make_shared<NamedFloatingPoint<T>>(name, value);

    // Initialize self-reference for getSelf().
    obj->setSelf(obj);

    // Attach to parent if provided.
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  /**
   * @brief Creates a clone of this NamedFloatingPoint.
   * 
   * Fulfills [FE-0020.14] Utilities for copying parts of the tree.
   * 
   * @return A new NamedFloatingPoint with the same name and value, but no
   * hierarchy.
   */
  std::shared_ptr<NamedObject> clone() const override {
    // Return a new instance using the same name and current value.
    return create(this->getName(), this->value());
  }

  /**
   * @brief Returns the type of the object.
   * @return "NamedFloatingPoint"
   */
  std::string getType() const override { return "NamedFloatingPoint"; }

  /**
   * @brief Constructs a NamedFloatingPoint instance.
   * @param name The name of the object.
   * @param value The initial value.
   */
  NamedFloatingPoint(const std::string &name, T value)
      : NamedObject(name), quasar::coretypes::FloatingPoint<T>(value) {
    // Both base classes are initialized with the provided arguments.
  }
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDFLOATINGPOINT_HPP

