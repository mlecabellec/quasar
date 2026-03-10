/**
 * @file NamedDate.hpp
 * @brief Named representation of a Date.
 */

#ifndef QUASAR_NAMED_NAMEDDATE_HPP
#define QUASAR_NAMED_NAMEDDATE_HPP

#include "quasar/coretypes/Date.hpp"
#include "quasar/named/NamedObject.hpp"

namespace quasar::named {

/**
 * @class NamedDate
 * @brief A NamedObject that holds a Date value.
 * @compliance [FE-0110.3.1] Temporal Types: Date.
 */
class NamedDate : public NamedObject, public quasar::coretypes::Date {
public:
  /** @brief Virtual destructor. */
  virtual ~NamedDate() = default;

  /**
   * @brief Factory method.
   * @param name Object name.
   * @param days_since_epoch Initial value.
   * @param parent Optional parent.
   * @return Shared pointer to the new object.
   */
  static std::shared_ptr<NamedDate> create(const std::string &name, int64_t days_since_epoch, std::shared_ptr<NamedObject> parent = nullptr) {
    std::shared_ptr<NamedDate> obj = std::make_shared<NamedDate>(name, days_since_epoch);
    obj->setSelf(obj);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  /**
   * @brief Factory method for current date.
   * @param name Object name.
   * @param parent Optional parent.
   * @return Shared pointer to the new object.
   */
  static std::shared_ptr<NamedDate> now(const std::string &name, std::shared_ptr<NamedObject> parent = nullptr) {
    return create(name, quasar::coretypes::Date::now().value(), parent);
  }

  /**
   * @brief Standalone copy.
   * @return Cloned object.
   */
  std::shared_ptr<NamedObject> clone() const override {
    return NamedDate::create(getName(), value());
  }

  /** @brief Returns "NamedDate". */
  std::string getType() const override { return "NamedDate"; }

  /**
   * @brief Constructor.
   * @param name Object name.
   * @param days Days since epoch.
   */
  NamedDate(const std::string &name, int64_t days)
      : NamedObject(name), quasar::coretypes::Date(days) {}
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDDATE_HPP

