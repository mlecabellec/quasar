#include "quasar/named/NamedString.hpp"

namespace quasar::named {

NamedString::NamedString(const std::string &name, const std::string &value)
    : NamedObject(name), quasar::coretypes::String(value) {
    // Both base classes are initialized with the respective name and value.
}

std::shared_ptr<NamedString>
NamedString::create(const std::string &name, const std::string &value,
                    std::shared_ptr<NamedObject> parent) {
  // Create a new NamedString instance using make_shared.
  std::shared_ptr<NamedString> obj =
      std::make_shared<NamedString>(name, value);
  
  // Set the self-reference for getSelf().
  obj->setSelf(obj);
  
  // Link to parent if one is provided.
  if (parent) {
    obj->setParent(parent);
  }
  return obj;
}

} // namespace quasar::named
