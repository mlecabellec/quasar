/**
 * @file NamedFloatingPoint.hpp
 * @brief Template class for named floating point values.
 */

#ifndef QUASAR_NAMED_NAMEDFLOATINGPOINT_HPP
#define QUASAR_NAMED_NAMEDFLOATINGPOINT_HPP

#include "quasar/coretypes/FloatingPoint.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/IBoundPrimitive.hpp"
#include "quasar/named/CopyPolicy.hpp"

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
                           public quasar::coretypes::FloatingPoint<T>,
                           public IBoundPrimitive {
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
   * @param policy Memory policy (DUPLICATE vs SHARE).
   * @return A new NamedFloatingPoint with the same name and value, but no
   * hierarchy.
   */
  std::shared_ptr<NamedObject> clone(CopyPolicy policy = CopyPolicy::DUPLICATE) const override {
    if (policy == CopyPolicy::SHARE && m_bound) {
        std::shared_ptr<NamedFloatingPoint<T>> newObj = create(this->getName(), this->value());
        newObj->bind(m_bound_offset, m_bound_length);
        return newObj;
    }
    // Return a new instance using the same name and current value.
    return create(this->getName(), this->value());
  }

  // --- IBoundPrimitive implementation ---
  bool isBound() const override { return m_bound; }
  std::size_t getBoundOffset() const override { return m_bound_offset; }
  std::size_t getBoundLength() const override { return m_bound_length; }

  /**
   * @brief Binds this float to a specific memory offset (conceptual in Phase 1).
   */
  void bind(std::size_t offset, std::size_t length) {
      m_bound = true;
      m_bound_offset = offset;
      m_bound_length = length;
  }

  /**
   * @brief Sets the floating point value and notifies observers.
   * @param value The new value.
   */
  void setValue(T value) {
      quasar::coretypes::FloatingPoint<T>::setValue(value);
      notifyObservers(getSelf());
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
      : NamedObject(name), quasar::coretypes::FloatingPoint<T>(value),
        m_bound(false), m_bound_offset(0), m_bound_length(sizeof(T)) {
    // Both base classes are initialized with the provided arguments.
  }

private:
  bool m_bound;
  std::size_t m_bound_offset;
  std::size_t m_bound_length;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDFLOATINGPOINT_HPP

