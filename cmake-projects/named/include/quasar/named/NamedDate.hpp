/**
 * @file NamedDate.hpp
 * @brief Named representation of a Date.
 */

#ifndef QUASAR_NAMED_NAMEDDATE_HPP
#define QUASAR_NAMED_NAMEDDATE_HPP

#include "quasar/coretypes/Date.hpp"
#include "quasar/named/NamedObject.hpp"

namespace quasar::named {

class NamedDate : public NamedObject, public quasar::coretypes::Date {
public:
  virtual ~NamedDate() = default;

  static std::shared_ptr<NamedDate> create(const std::string &name, int64_t days_since_epoch, std::shared_ptr<NamedObject> parent = nullptr) {
    auto obj = std::make_shared<NamedDate>(name, days_since_epoch);
    obj->setSelf(obj);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  static std::shared_ptr<NamedDate> now(const std::string &name, std::shared_ptr<NamedObject> parent = nullptr) {
    return create(name, quasar::coretypes::Date::now().value(), parent);
  }

  std::shared_ptr<NamedObject> clone() const override {
    return NamedDate::create(getName(), value());
  }

  std::string getType() const override { return "NamedDate"; }

  NamedDate(const std::string &name, int64_t days)
      : NamedObject(name), quasar::coretypes::Date(days) {}
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDDATE_HPP
