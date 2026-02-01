#ifndef QUASAR_NAMED_NAMEDINTEGER_H
#define QUASAR_NAMED_NAMEDINTEGER_H

#include "quasar/coretypes/Integer.hpp"
#include "quasar/named/NamedObject.h"

namespace quasar::named {

/**
 * @brief A named integer value.
 * Inherits from NamedObject and coretypes::Integer.
 * @tparam T The underlying integer type.
 */
template <typename T>
class NamedInteger : public NamedObject, public quasar::coretypes::Integer<T> {
public:
  /**
   * @brief Destructor.
   */
  virtual ~NamedInteger() = default;

  /**
   * @brief Creates a NamedInteger.
   * @param name The name.
   * @param value The initial value.
   * @param parent The optional parent.
   * @return Shared pointer to created NamedInteger.
   */
  static std::shared_ptr<NamedInteger<T>>
  create(const std::string &name, T value,
         std::shared_ptr<NamedObject> parent = nullptr) {
    auto obj = std::make_shared<NamedInteger<T>>(name, value);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  std::shared_ptr<NamedObject> clone() const override {
    return create(this->getName(), this->value());
  }

  /**
   * @brief Constructs a NamedInteger.
   * @param name The name.
   * @param value The initial value.
   */
  NamedInteger(const std::string &name, T value)
      : NamedObject(name), quasar::coretypes::Integer<T>(value) {}
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDINTEGER_H
