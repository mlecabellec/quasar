#ifndef QUASAR_NAMED_NAMEDSTRING_HPP
#define QUASAR_NAMED_NAMEDSTRING_HPP

#include "quasar/coretypes/String.hpp"
#include "quasar/named/NamedObject.hpp"

namespace quasar::named {

/**
 * @brief A named string value.
 * Inherits from NamedObject and coretypes::String.
 */
class NamedString : public NamedObject, public quasar::coretypes::String {
public:
  /**
   * @brief Destructor.
   */
  virtual ~NamedString() = default;

  /**
   * @brief Creates a new NamedString.
   * @param name The name of the object.
   * @param value The string value.
   * @param parent The optional parent of the object.
   * @return A shared_ptr to the created object.
   */
  static std::shared_ptr<NamedString>
  create(const std::string &name, const std::string &value,
         std::shared_ptr<NamedObject> parent = nullptr);

  std::shared_ptr<NamedObject> clone() const override {
    return create(getName(), toString());
  }

  /**
   * @brief Constructs a NamedString.
   * @param name The name.
   * @param value The initial value.
   */
  NamedString(const std::string &name, const std::string &value);
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDSTRING_HPP
