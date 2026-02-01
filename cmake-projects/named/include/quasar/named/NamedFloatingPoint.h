#ifndef QUASAR_NAMED_NAMEDFLOATINGPOINT_H
#define QUASAR_NAMED_NAMEDFLOATINGPOINT_H

#include "quasar/coretypes/FloatingPoint.hpp"
#include "quasar/named/NamedObject.h"

namespace quasar::named {

/**
 * @brief A named floating point value.
 * Inherits from NamedObject and coretypes::FloatingPoint.
 * @tparam T The underlying floating point type.
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
   * @brief Creates a NamedFloatingPoint.
   * @param name The name.
   * @param value The initial value.
   * @param parent The optional parent.
   * @return Shared pointer to created NamedFloatingPoint.
   */
  static std::shared_ptr<NamedFloatingPoint<T>>
  create(const std::string &name, T value,
         std::shared_ptr<NamedObject> parent = nullptr) {
    auto obj = std::make_shared<NamedFloatingPoint<T>>(name, value);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  std::shared_ptr<NamedObject> clone() const override {
    return create(this->getName(), this->value());
  }

  /**
   * @brief Constructs a NamedFloatingPoint.
   * @param name The name.
   * @param value The initial value.
   */
  NamedFloatingPoint(const std::string &name, T value)
      : NamedObject(name), quasar::coretypes::FloatingPoint<T>(value) {}
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDFLOATINGPOINT_H
