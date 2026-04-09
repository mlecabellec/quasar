#include "quasar/named/Serialization.hpp"
#include "quasar/named/NamedBitBuffer.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedTimestamp.hpp"
#include "quasar/named/NamedDuration.hpp"
#include "quasar/named/NamedDate.hpp"
#include "quasar/named/NamedQuantity.hpp"
#include "quasar/named/NamedArray.hpp"
#include "quasar/named/NamedMap.hpp"
#include "quasar/named/NamedSet.hpp"
#include "quasar/named/NamedVariant.hpp"
#include "quasar/named/NamedConfig.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/bson/bson.hpp>
#include <stdexcept>
#include <tinyxml2.h>
#include <yaml-cpp/yaml.h>

namespace quasar::named::serialization {

using namespace tinyxml2;
using jsoncons::json;

// --- Helper Factory ---
/**
 * @brief Creates a NamedObject from type and value strings.
 * @param name Object name.
 * @param type Object type name.
 * @param valueStr String representation of the value.
 * @param parent Optional parent.
 * @return Shared pointer to the created object.
 */
std::shared_ptr<NamedObject>
createFromTypeAndValue(const std::string &name, const std::string &type,
                       const std::string &valueStr,
                       std::shared_ptr<NamedObject> parent) {
  // [CS-0010.44] Block comments for dispatch logic.
  if (type == "NamedBoolean" || type == "Boolean") {
    return NamedBoolean::create(name, valueStr == "true", parent);
  } else if (type == "NamedInteger" || type == "Integer") {
    return NamedInteger<int64_t>::create(name, std::stoll(valueStr), parent);
  } else if (type == "NamedFloatingPoint" || type == "FloatingPoint") {
    return NamedFloatingPoint<double>::create(name, std::stod(valueStr), parent);
  } else if (type == "NamedBuffer" || type == "Buffer") {
    return NamedBuffer::create(name, quasar::coretypes::Buffer::fromString(valueStr).toVector(), parent);
  } else if (type == "NamedBitBuffer" || type == "BitBuffer") {
    quasar::coretypes::Buffer buf = quasar::coretypes::Buffer::fromString(valueStr);
    std::shared_ptr<NamedBitBuffer> nbb = NamedBitBuffer::create(name, buf.size() * 8, parent);
    // [CS-0010.37] Loop hard limit.
    std::size_t iterations = 0;
    for (size_t i = 0; i < buf.size(); i++) {
        if (++iterations > config::HARD_LIMIT_ITERATIONS) {
            throw std::runtime_error("Hard limit reached in createFromTypeAndValue (BitBuffer)");
        }
        nbb->set(i, buf.get(i));
    }
    return nbb;
  } else if (type == "NamedString" || type == "String") {
    return NamedString::create(name, valueStr, parent);
  } else if (type == "NamedTimestamp") {
    return NamedTimestamp::create(name, std::stoll(valueStr), parent);
  } else if (type == "NamedDuration") {
    return NamedDuration::create(name, std::stoll(valueStr), parent);
  } else if (type == "NamedDate") {
    return NamedDate::create(name, std::stoll(valueStr), parent);
  } else if (type == "NamedQuantity") {
    size_t spacePos = valueStr.find(' ');
    if (spacePos != std::string::npos) {
        double val = std::stod(valueStr.substr(0, spacePos));
        std::string unitSym = valueStr.substr(spacePos + 1);
        return NamedQuantity::create(name, val, quasar::coretypes::Units::fromSymbol(unitSym), parent);
    }
    return NamedQuantity::create(name, std::stod(valueStr), quasar::coretypes::Units::Dimensionless, parent);
  } else if (type == "NamedArray") {
    return NamedArray<NamedObject>::create(name, parent);
  } else if (type == "NamedMap") {
    return NamedMap<NamedObject>::create(name, parent);
  } else if (type == "NamedSet") {
    return NamedSet<NamedObject>::create(name, parent);
  } else if (type == "NamedVariant") {
    return NamedVariant::create(name, parent);
  }
  return NamedObject::create(name, parent);
}

// --- Value Helper ---
/**
 * @brief Serializes the value of a NamedObject to a string.
 * @param obj The object.
 * @return String representation.
 */
std::string getValueAsString(const std::shared_ptr<NamedObject>& obj) {
    // [CS-0010.34] auto forbidden.
    const quasar::coretypes::Boolean* b = dynamic_cast<const quasar::coretypes::Boolean*>(obj.get());
    if (b) return b->toString();
    
    const quasar::coretypes::Number* n = dynamic_cast<const quasar::coretypes::Number*>(obj.get());
    if (n) return n->toString();
    
    const quasar::coretypes::BitBuffer* bb = dynamic_cast<const quasar::coretypes::BitBuffer*>(obj.get());
    if (bb) return bb->toString();
    
    const quasar::coretypes::Buffer* buf = dynamic_cast<const quasar::coretypes::Buffer*>(obj.get());
    if (buf) return buf->toString();
    
    const quasar::coretypes::String* s = dynamic_cast<const quasar::coretypes::String*>(obj.get());
    if (s) return s->toString();
    
    return "";
}

// --- XML ---
/** @compliance [FE-0020.9.4] XML conversion. */
void serializeToXml(XMLElement *element, const std::shared_ptr<NamedObject> &obj) {
  // [CS-0010.15] Null pointer check.
  if (!element) throw std::invalid_argument("XMLElement is null in serializeToXml");
  
  element->SetAttribute("name", obj->getName().c_str());
  element->SetAttribute("type", obj->getType().c_str());
  std::string val = getValueAsString(obj);
  if (!val.empty()) element->SetText(val.c_str());

  // [CS-0010.34] auto forbidden.
  std::list<std::shared_ptr<NamedObject>> children = obj->getChildren();
  for (std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
    const std::shared_ptr<NamedObject> &child = *it;
    XMLElement *childElem = element->GetDocument()->NewElement("NamedObject");
    serializeToXml(childElem, child);
    element->InsertEndChild(childElem);
  }
}

std::string toXml(const std::shared_ptr<NamedObject> &obj) {
  XMLDocument doc;
  XMLElement *root = doc.NewElement("NamedObject");
  doc.InsertFirstChild(root);
  serializeToXml(root, obj);
  XMLPrinter printer;
  doc.Accept(&printer);
  return printer.CStr();
}

/** @compliance [FE-0020.9.4] XML conversion. */
void deserializeFromXml(XMLElement *element, std::shared_ptr<NamedObject> parent) {
  // [CS-0010.15] Null pointer check.
  if (!element) return;
  
  const char *name = element->Attribute("name");
  const char *type = element->Attribute("type");
  const char *text = element->GetText();
  std::shared_ptr<NamedObject> obj = createFromTypeAndValue(name ? name : "unnamed", type ? type : "Object", text ? text : "", parent);
  
  XMLElement *child = element->FirstChildElement("NamedObject");
  // [CS-0010.37] Loop hard limit.
  std::size_t iterations = 0;
  while (child) {
    if (++iterations > config::HARD_LIMIT_ITERATIONS) {
        throw std::runtime_error("Hard limit reached in deserializeFromXml");
    }
    deserializeFromXml(child, obj);
    child = child->NextSiblingElement("NamedObject");
  }
}

std::shared_ptr<NamedObject> fromXml(const std::string &xml) {
  XMLDocument doc;
  if (doc.Parse(xml.c_str()) != XML_SUCCESS) throw std::runtime_error("Failed to parse XML");
  XMLElement *root = doc.FirstChildElement("NamedObject");
  if (!root) throw std::runtime_error("Invalid XML: missing root NamedObject");
  const char *name = root->Attribute("name");
  const char *type = root->Attribute("type");
  const char *text = root->GetText();
  std::shared_ptr<NamedObject> obj = createFromTypeAndValue(name ? name : "unnamed", type ? type : "Object", text ? text : "", nullptr);
  XMLElement *child = root->FirstChildElement("NamedObject");
  
  // [CS-0010.37] Loop hard limit.
  std::size_t iterations = 0;
  while (child) {
    if (++iterations > config::HARD_LIMIT_ITERATIONS) {
        throw std::runtime_error("Hard limit reached in fromXml");
    }
    deserializeFromXml(child, obj);
    child = child->NextSiblingElement("NamedObject");
  }
  return obj;
}

// --- YAML ---
/** @compliance [FE-0020.9.3] YAML conversion. */
YAML::Node serializeToYaml(const std::shared_ptr<NamedObject> &obj) {
  YAML::Node node;
  node["name"] = obj->getName();
  node["type"] = obj->getType();
  std::string val = getValueAsString(obj);
  if (!val.empty()) node["value"] = val;
  
  // [CS-0010.34] auto forbidden.
  std::list<std::shared_ptr<NamedObject>> children = obj->getChildren();
  for (std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
    const std::shared_ptr<NamedObject> &child = *it;
    node["children"].push_back(serializeToYaml(child));
  }
  return node;
}

std::string toYaml(const std::shared_ptr<NamedObject> &obj) {
  YAML::Node root = serializeToYaml(obj);
  YAML::Emitter out;
  out << root;
  return out.c_str();
}

/** @compliance [FE-0020.9.3] YAML conversion. */
void deserializeFromYaml(const YAML::Node &node, std::shared_ptr<NamedObject> parent) {
  std::string name = node["name"].as<std::string>();
  std::string type = node["type"].as<std::string>();
  std::string value = node["value"] ? node["value"].as<std::string>() : "";
  std::shared_ptr<NamedObject> obj = createFromTypeAndValue(name, type, value, parent);
  if (node["children"]) {
    // [CS-0010.34] auto forbidden.
    for (YAML::const_iterator it = node["children"].begin(); it != node["children"].end(); ++it) {
        deserializeFromYaml(*it, obj);
    }
  }
}

std::shared_ptr<NamedObject> fromYaml(const std::string &yaml) {
  YAML::Node root = YAML::Load(yaml);
  if (!root.IsDefined()) throw std::runtime_error("Invalid YAML");
  std::string name = root["name"].as<std::string>();
  std::string type = root["type"].as<std::string>();
  std::string value = root["value"] ? root["value"].as<std::string>() : "";
  std::shared_ptr<NamedObject> obj = createFromTypeAndValue(name, type, value, nullptr);
  if (root["children"]) {
    // [CS-0010.34] auto forbidden.
    for (YAML::const_iterator it = root["children"].begin(); it != root["children"].end(); ++it) {
        deserializeFromYaml(*it, obj);
    }
  }
  return obj;
}

// --- JSON ---
/** @compliance [FE-0020.9.2] JSON conversion. */
json serializeToJson(const std::shared_ptr<NamedObject> &obj) {
  json j;
  j["name"] = obj->getName();
  j["type"] = obj->getType();
  std::string val = getValueAsString(obj);
  if (!val.empty()) j["value"] = val;
  
  std::list<std::shared_ptr<NamedObject>> children = obj->getChildren();
  if (!children.empty()) {
    json json_children = json::array();
    // [CS-0010.34] auto forbidden.
    for (std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
      json_children.push_back(serializeToJson(*it));
    }
    j["children"] = json_children;
  }
  return j;
}

std::string toJson(const std::shared_ptr<NamedObject> &obj) {
  return serializeToJson(obj).to_string();
}

/** @compliance [FE-0020.9.2] JSON conversion. */
void deserializeFromJson(const json &j, std::shared_ptr<NamedObject> parent) {
  std::string name = j["name"].as<std::string>();
  std::string type = j["type"].as<std::string>();
  std::string value = j.contains("value") ? j["value"].as<std::string>() : "";
  std::shared_ptr<NamedObject> obj = createFromTypeAndValue(name, type, value, parent);
  if (j.contains("children")) {
    // [CS-0010.34] auto forbidden.
    for (const json& child : j["children"].array_range()) {
        deserializeFromJson(child, obj);
    }
  }
}

std::shared_ptr<NamedObject> fromJson(const std::string &jsonStr) {
  json j = json::parse(jsonStr);
  std::string name = j["name"].as<std::string>();
  std::string type = j["type"].as<std::string>();
  std::string value = j.contains("value") ? j["value"].as<std::string>() : "";
  std::shared_ptr<NamedObject> obj = createFromTypeAndValue(name, type, value, nullptr);
  if (j.contains("children")) {
    // [CS-0010.34] auto forbidden.
    for (const json& child : j["children"].array_range()) {
        deserializeFromJson(child, obj);
    }
  }
  return obj;
}

// --- BSON ---

/**
 * @brief Helper to serialize a NamedObject to a json object suitable for BSON.
 * @param obj The object to serialize.
 * @return A json object.
 * @compliance [FE-0020.9.2] BSON conversion.
 */
json serializeToBinaryJson(const std::shared_ptr<NamedObject> &obj) {
  json j;
  j["name"] = obj->getName();
  j["type"] = obj->getType();
  
  const quasar::coretypes::BitBuffer* bb = dynamic_cast<const quasar::coretypes::BitBuffer*>(obj.get());
  if (bb) {
      std::vector<uint8_t> vec = bb->toVector();
      j["value"] = jsoncons::byte_string(vec.data(), vec.size());
      j["bitSize"] = static_cast<uint64_t>(bb->bitSize());
  } else {
      const quasar::coretypes::Buffer* buf = dynamic_cast<const quasar::coretypes::Buffer*>(obj.get());
      if (buf) {
          std::vector<uint8_t> vec = buf->toVector();
          j["value"] = jsoncons::byte_string(vec.data(), vec.size());
      } else {
          std::string val = getValueAsString(obj);
          if (!val.empty()) j["value"] = val;
      }
  }
  
  std::list<std::shared_ptr<NamedObject>> children = obj->getChildren();
  if (!children.empty()) {
    json json_children = json::array();
    for (std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
      const std::shared_ptr<NamedObject> &child = *it;
      json_children.push_back(serializeToBinaryJson(child));
    }
    j["children"] = json_children;
  }
  return j;
}

/**
 * @brief Helper to reconstruct a NamedObject hierarchy from a json object (BSON context).
 * @param j The json object.
 * @param parent Optional parent.
 * @return Shared pointer to the created object.
 * @compliance [FE-0020.9.2] BSON conversion.
 */
std::shared_ptr<NamedObject> createFromBinaryJson(const json &j, std::shared_ptr<NamedObject> parent) {
  std::string name = j["name"].as<std::string>();
  std::string type = j["type"].as<std::string>();
  
  std::shared_ptr<NamedObject> obj;
  if (type == "NamedBitBuffer" || type == "BitBuffer") {
      size_t bitSize = j["bitSize"].as<size_t>();
      jsoncons::byte_string bytes = j["value"].as<jsoncons::byte_string>();
      std::shared_ptr<NamedBitBuffer> nbb = NamedBitBuffer::create(name, bitSize, parent);
      for (size_t i = 0; i < bytes.size(); ++i) {
          nbb->set(i, bytes[i]);
      }
      obj = nbb;
  } else if (type == "NamedBuffer" || type == "Buffer") {
      jsoncons::byte_string bytes = j["value"].as<jsoncons::byte_string>();
      std::vector<uint8_t> data(bytes.begin(), bytes.end());
      obj = NamedBuffer::create(name, data, parent);
  } else {
      std::string value = j.contains("value") ? (j["value"].is_string() ? j["value"].as<std::string>() : "") : "";
      obj = createFromTypeAndValue(name, type, value, parent);
  }

  if (j.contains("children")) {
    for (const json& child : j["children"].array_range()) {
        createFromBinaryJson(child, obj);
    }
  }
  return obj;
}

std::vector<uint8_t> toBinary(const std::shared_ptr<NamedObject> &obj) {
  json j = serializeToBinaryJson(obj);
  std::vector<uint8_t> buffer;
  jsoncons::bson::encode_bson(j, buffer);
  return buffer;
}

std::shared_ptr<NamedObject> fromBinary(const std::vector<uint8_t> &data) {
  json j = jsoncons::bson::decode_bson<json>(data);
  return createFromBinaryJson(j, nullptr);
}

} // namespace quasar::named::serialization
