#ifndef QUASAR_NAMED_SERIALIZATION_H
#define QUASAR_NAMED_SERIALIZATION_H

#include "quasar/named/NamedObject.h"
#include <string>
#include <memory>

namespace quasar::named::serialization {

std::string toXml(const std::shared_ptr<NamedObject>& obj);
std::shared_ptr<NamedObject> fromXml(const std::string& xml);

std::string toYaml(const std::shared_ptr<NamedObject>& obj);
std::shared_ptr<NamedObject> fromYaml(const std::string& yaml);

std::string toJson(const std::shared_ptr<NamedObject>& obj);
std::shared_ptr<NamedObject> fromJson(const std::string& json);

} // namespace quasar::named::serialization

#endif // QUASAR_NAMED_SERIALIZATION_H
