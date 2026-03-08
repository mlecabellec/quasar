/**
 * @file NamedTimestamp.hpp
 * @brief Named representation of a Timestamp.
 */

#ifndef QUASAR_NAMED_NAMEDTIMESTAMP_HPP
#define QUASAR_NAMED_NAMEDTIMESTAMP_HPP

#include "quasar/coretypes/Timestamp.hpp"
#include "quasar/named/NamedObject.hpp"

namespace quasar::named {

/**
 * @class NamedTimestamp
 * @brief A high-precision timestamp wrapped as a NamedObject.
 *
 * **Compliance**:
 * - Fulfills [TSK-20260303-002.5] Temporal Types: Timestamp.
 */
class NamedTimestamp : public NamedObject, public quasar::coretypes::Timestamp {
public:
  virtual ~NamedTimestamp() = default;

  static std::shared_ptr<NamedTimestamp> create(const std::string &name, int64_t us_since_epoch, std::shared_ptr<NamedObject> parent = nullptr) {
    auto obj = std::make_shared<NamedTimestamp>(name, us_since_epoch);
    obj->setSelf(obj);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  static std::shared_ptr<NamedTimestamp> now(const std::string &name, std::shared_ptr<NamedObject> parent = nullptr) {
    return create(name, quasar::coretypes::Timestamp::now().value(), parent);
  }

  std::shared_ptr<NamedObject> clone() const override {
    return NamedTimestamp::create(getName(), value());
  }

  std::string getType() const override { return "NamedTimestamp"; }

  NamedTimestamp(const std::string &name, int64_t us)
      : NamedObject(name), quasar::coretypes::Timestamp(us) {}
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDTIMESTAMP_HPP
