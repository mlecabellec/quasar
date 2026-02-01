#include "quasar/named/NamedBoolean.h"

namespace quasar::named {

NamedBoolean::NamedBoolean(const std::string &name, bool value)
    : NamedObject(name), quasar::coretypes::Boolean(value) {}

std::shared_ptr<NamedBoolean>
NamedBoolean::create(const std::string &name, bool value,
                     std::shared_ptr<NamedObject> parent) {
  // Create new instance.
  std::shared_ptr<NamedBoolean> obj =
      std::make_shared<NamedBoolean>(name, value);
  if (parent) {
    // Set parent if provided.
    obj->setParent(parent);
  }
  return obj;
}

} // namespace quasar::named
