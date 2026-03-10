/**
 * @file NamedDuration.hpp
 * @brief Named representation of a Duration.
 */

#ifndef QUASAR_NAMED_NAMEDDURATION_HPP
#define QUASAR_NAMED_NAMEDDURATION_HPP

#include "quasar/coretypes/Duration.hpp"
#include "quasar/named/NamedObject.hpp"

namespace quasar::named {

/**
 * @class NamedDuration
 * @brief A NamedObject that holds a Duration value.
 * @compliance [FE-0110.3.1] Temporal Types: Duration.
 */
class NamedDuration : public NamedObject, public quasar::coretypes::Duration {
public:
  /** @brief Virtual destructor. */
  virtual ~NamedDuration() = default;

  /**
   * @brief Factory method.
   * @param name Object name.
   * @param us_duration Initial value in microseconds.
   * @param parent Optional parent.
   * @return Shared pointer to the new object.
   */
  static std::shared_ptr<NamedDuration> create(const std::string &name, int64_t us_duration, std::shared_ptr<NamedObject> parent = nullptr) {
    std::shared_ptr<NamedDuration> obj = std::make_shared<NamedDuration>(name, us_duration);
    obj->setSelf(obj);
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  /**
   * @brief Standalone copy.
   * @return Cloned object.
   */
  std::shared_ptr<NamedObject> clone() const override {
    return NamedDuration::create(getName(), value());
  }

  /** @brief Returns "NamedDuration". */
  std::string getType() const override { return "NamedDuration"; }

  /**
   * @brief Constructor.
   * @param name Object name.
   * @param us Microseconds.
   */
  NamedDuration(const std::string &name, int64_t us)
      : NamedObject(name), quasar::coretypes::Duration(us) {}
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDDURATION_HPP

