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
 * - Fulfills [FE-0110.3.1] Temporal Types: Timestamp.
 */
class NamedTimestamp : public NamedObject, public quasar::coretypes::Timestamp {
public:
  /** @brief Virtual destructor. */
  virtual ~NamedTimestamp() = default;

  /**
   * @brief Factory method.
   * @param name Object name.
   * @param us_since_epoch Initial value in microseconds since epoch.
   * @param parent Optional parent.
   * @return Shared pointer to the new object.
   */
  static std::shared_ptr<NamedTimestamp> create(const std::string &name, int64_t us_since_epoch, std::shared_ptr<NamedObject> parent = nullptr) {
    std::shared_ptr<NamedTimestamp> obj = std::make_shared<NamedTimestamp>(name, us_since_epoch);
    obj->setSelf(obj);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  /**
   * @brief Factory method for current time.
   * @param name Object name.
   * @param parent Optional parent.
   * @return Shared pointer to the new object.
   */
  static std::shared_ptr<NamedTimestamp> now(const std::string &name, std::shared_ptr<NamedObject> parent = nullptr) {
    return create(name, quasar::coretypes::Timestamp::now().value(), parent);
  }

  /**
   * @brief Standalone copy.
   * @return Cloned object.
   */
  std::shared_ptr<NamedObject> clone() const override {
    return NamedTimestamp::create(getName(), value());
  }

  /** @brief Returns "NamedTimestamp". */
  std::string getType() const override { return "NamedTimestamp"; }

  /**
   * @brief Constructor.
   * @param name Object name.
   * @param us Microseconds since epoch.
   */
  NamedTimestamp(const std::string &name, int64_t us)
      : NamedObject(name), quasar::coretypes::Timestamp(us) {}
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDTIMESTAMP_HPP

