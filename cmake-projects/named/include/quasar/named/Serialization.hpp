/**
 * @file Serialization.hpp
 * @brief Utilities for serializing NamedObject hierarchies to various formats (XML, YAML, JSON).
 */

#ifndef QUASAR_NAMED_SERIALIZATION_HPP
#define QUASAR_NAMED_SERIALIZATION_HPP

#include "quasar/named/NamedObject.hpp"
#include <memory>
#include <string>

/**
 * @namespace quasar::named::serialization
 * @brief Namespace for object serialization and deserialization.
 * 
 * **Compliance**:
 * - Fulfills [FE-0020.9] Capabilities to convert NamedObject to/from JSON, BSON, YAML, and XML.
 * - Fulfills [FE-0020.9.1] Implemented as separate utilities/visitors.
 */
namespace quasar::named::serialization {

/**
 * @brief Serializes a NamedObject hierarchy into an XML string.
 * 
 * Fulfills [FE-0020.9.4] XML conversion.
 * 
 * @param obj The root object of the hierarchy to serialize.
 * @return A string containing the XML representation.
 */
std::string toXml(const std::shared_ptr<NamedObject> &obj);

/**
 * @brief Deserializes a NamedObject hierarchy from an XML string.
 * 
 * Fulfills [FE-0020.9.4] XML conversion.
 * 
 * @param xml The XML string to parse.
 * @return Shared pointer to the root of the reconstructed hierarchy.
 * @throws std::runtime_error if parsing fails.
 */
std::shared_ptr<NamedObject> fromXml(const std::string &xml);

/**
 * @brief Serializes a NamedObject hierarchy into a YAML string.
 * 
 * Fulfills [FE-0020.9.3] YAML conversion.
 * 
 * @param obj The root object of the hierarchy to serialize.
 * @return A string containing the YAML representation.
 */
std::string toYaml(const std::shared_ptr<NamedObject> &obj);

/**
 * @brief Deserializes a NamedObject hierarchy from a YAML string.
 * 
 * Fulfills [FE-0020.9.3] YAML conversion.
 * 
 * @param yaml The YAML string to parse.
 * @return Shared pointer to the root of the reconstructed hierarchy.
 * @throws std::runtime_error if parsing fails.
 */
std::shared_ptr<NamedObject> fromYaml(const std::string &yaml);

/**
 * @brief Serializes a NamedObject hierarchy into a JSON string.
 * 
 * Fulfills [FE-0020.9.2] JSON conversion.
 * 
 * @param obj The root object of the hierarchy to serialize.
 * @return A string containing the JSON representation.
 */
std::string toJson(const std::shared_ptr<NamedObject> &obj);

/**
 * @brief Deserializes a NamedObject hierarchy from a JSON string.
 * 
 * Fulfills [FE-0020.9.2] JSON conversion.
 * 
 * @param json The JSON string to parse.
 * @return Shared pointer to the root of the reconstructed hierarchy.
 * @throws std::runtime_error if parsing fails.
 */
std::shared_ptr<NamedObject> fromJson(const std::string &json);

} // namespace quasar::named::serialization

#endif // QUASAR_NAMED_SERIALIZATION_HPP
