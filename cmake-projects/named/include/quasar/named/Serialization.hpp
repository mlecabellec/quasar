#ifndef QUASAR_NAMED_SERIALIZATION_HPP
#define QUASAR_NAMED_SERIALIZATION_HPP

#include "quasar/named/NamedObject.hpp"
#include <memory>
#include <string>

namespace quasar::named::serialization {

/**
 * @brief Serializes a NamedObject to XML string.
 * @param obj The object.
 * @return XML string.
 */
std::string toXml(const std::shared_ptr<NamedObject> &obj);

/**
 * @brief Deserializes a NamedObject from XML string.
 * @param xml The XML string.
 * @return Shared pointer to deserialized object.
 */
std::shared_ptr<NamedObject> fromXml(const std::string &xml);

/**
 * @brief Serializes a NamedObject to YAML string.
 * @param obj The object.
 * @return YAML string.
 */
std::string toYaml(const std::shared_ptr<NamedObject> &obj);

/**
 * @brief Deserializes a NamedObject from YAML string.
 * @param yaml The YAML string.
 * @return Shared pointer to deserialized object.
 */
std::shared_ptr<NamedObject> fromYaml(const std::string &yaml);

/**
 * @brief Serializes a NamedObject to JSON string.
 * @param obj The object.
 * @return JSON string.
 */
std::string toJson(const std::shared_ptr<NamedObject> &obj);

/**
 * @brief Deserializes a NamedObject from JSON string.
 * @param json The JSON string.
 * @return Shared pointer to deserialized object.
 */
std::shared_ptr<NamedObject> fromJson(const std::string &json);

} // namespace quasar::named::serialization

#endif // QUASAR_NAMED_SERIALIZATION_HPP
