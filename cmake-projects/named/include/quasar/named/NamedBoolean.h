#ifndef QUASAR_NAMED_NAMEDBOOLEAN_H
#define QUASAR_NAMED_NAMEDBOOLEAN_H

#include "quasar/coretypes/Boolean.hpp"
#include "quasar/named/NamedObject.h"

namespace quasar::named {

/**
 * @brief A named boolean value.
 * Inherits from NamedObject and coretypes::Boolean.
 */
class NamedBoolean : public NamedObject, public quasar::coretypes::Boolean {
public:
  /**
   * @brief Destructor.
   */
  virtual ~NamedBoolean() = default;

  /**
   * @brief Creates a new NamedBoolean.
   * @param name The name of the object.
   * @param value The boolean value.
   * @param parent The optional parent of the object.
   * @return A shared_ptr to the created object.
   */
  static std::shared_ptr<NamedBoolean>
  create(const std::string &name, bool value,
         std::shared_ptr<NamedObject> parent = nullptr);

  std::shared_ptr<NamedObject> clone() const override {
    return create(getName(), booleanValue());
  }

  /**
   * @brief Constructs a NamedBoolean.
   * @param name The name.
   * @param value The initial value.
   */
  NamedBoolean(const std::string &name, bool value);
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDBOOLEAN_H
