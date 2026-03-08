/**
 * @file NamedDuration.hpp
 * @brief Named representation of a Duration.
 */

#ifndef QUASAR_NAMED_NAMEDDURATION_HPP
#define QUASAR_NAMED_NAMEDDURATION_HPP

#include "quasar/coretypes/Duration.hpp"
#include "quasar/named/NamedObject.hpp"

namespace quasar::named {

class NamedDuration : public NamedObject, public quasar::coretypes::Duration {
public:
  virtual ~NamedDuration() = default;

  static std::shared_ptr<NamedDuration> create(const std::string &name, int64_t us_duration, std::shared_ptr<NamedObject> parent = nullptr) {
    auto obj = std::make_shared<NamedDuration>(name, us_duration);
    obj->setSelf(obj);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  std::shared_ptr<NamedObject> clone() const override {
    return NamedDuration::create(getName(), value());
  }

  std::string getType() const override { return "NamedDuration"; }

  NamedDuration(const std::string &name, int64_t us)
      : NamedObject(name), quasar::coretypes::Duration(us) {}
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDDURATION_HPP
