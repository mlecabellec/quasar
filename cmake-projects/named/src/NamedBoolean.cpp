#include "quasar/named/NamedBoolean.hpp"

namespace quasar::named {

NamedBoolean::NamedBoolean(const std::string &name, bool value)
    : NamedObject(name), quasar::coretypes::Boolean(value) {
    // Fulfills [FE-0020.4] Derivated class for Boolean core type.
    // Both base classes are initialized with the respective name and value.
}

std::shared_ptr<NamedBoolean>
NamedBoolean::create(const std::string &name, bool value,
                     std::shared_ptr<NamedObject> parent) {
  // Fulfills [FE-0020.6] static method "create".
  // Create a new NamedBoolean instance.
  std::shared_ptr<NamedBoolean> obj =
      std::make_shared<NamedBoolean>(name, value);
  
  // Set the self-reference for getSelf().
  obj->setSelf(obj);
  
  // Link to parent if one is provided.
  if (parent) {
    obj->setParent(parent);
  }
  return obj;
}

} // namespace quasar::named
