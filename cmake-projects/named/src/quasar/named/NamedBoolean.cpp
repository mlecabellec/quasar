#include "quasar/named/NamedBoolean.hpp"

namespace quasar::named {

std::shared_ptr<NamedBoolean> NamedBoolean::create(const std::string &name,
                                                   bool value,
                                                   std::shared_ptr<NamedObject> parent) {
  std::shared_ptr<NamedBoolean> obj = std::make_shared<NamedBoolean>(name, value);
  obj->setSelf(obj);
  if (parent) {
    obj->setParent(parent);
  }
  return obj;
}

std::string NamedBoolean::getType() const {
  return "NamedBoolean";
}

NamedBoolean::NamedBoolean(const std::string &name, bool value)
    : NamedObject(name), quasar::coretypes::Boolean(value),
      m_bound(false), m_bound_offset(0), m_bound_length(sizeof(bool)) {}

} // namespace quasar::named
