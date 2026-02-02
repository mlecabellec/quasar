#include "quasar/named/NamedString.hpp"

namespace quasar::named {

NamedString::NamedString(const std::string &name, const std::string &value)
    : NamedObject(name), quasar::coretypes::String(value) {}

std::shared_ptr<NamedString>
NamedString::create(const std::string &name, const std::string &value,
                    std::shared_ptr<NamedObject> parent) {
  std::shared_ptr<NamedString> obj =
      std::make_shared<NamedString>(name, value);
  obj->setSelf(obj);
  if (parent) {
    obj->setParent(parent);
  }
  return obj;
}

} // namespace quasar::named
