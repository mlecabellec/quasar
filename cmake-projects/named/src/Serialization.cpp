#include "quasar/named/Serialization.hpp"
#include "quasar/named/NamedBitBuffer.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedString.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <jsoncons/json.hpp>
#include <stdexcept>
#include <tinyxml2.h>
#include <yaml-cpp/yaml.h>

namespace quasar::named::serialization {

using namespace tinyxml2;

// --- Helper Factory ---
/**
 * @brief Reconstructs a specific NamedObject derivative based on type
 * information.
 */
std::shared_ptr<NamedObject>
createFromTypeAndValue(const std::string &name, const std::string &type,
                       const std::string &valueStr,
                       std::shared_ptr<NamedObject> parent) {
  if (type == "Boolean") {
    return NamedBoolean::create(name, valueStr == "true", parent);
  } else if (type == "Integer") {
    // Attempt to parse as 32-bit integer, fallback to 64-bit if it overflows.
    try {
      return NamedInteger<int32_t>::create(name, std::stoi(valueStr), parent);
    } catch (...) {
      return NamedInteger<int64_t>::create(name, std::stoll(valueStr), parent);
    }
  } else if (type == "FloatingPoint") {
    return NamedFloatingPoint<double>::create(name, std::stod(valueStr),
                                              parent);
  } else if (type == "Buffer") {
    // Reconstruct buffer from its string representation (typically hex).
    return NamedBuffer::create(
        name, quasar::coretypes::Buffer::fromString(valueStr).toVector(),
        parent);
  } else if (type == "BitBuffer") {
    // BitBuffer reconstruction. Currently assumes byte-aligned data from
    // string.
    quasar::coretypes::Buffer buf =
        quasar::coretypes::Buffer::fromString(valueStr);
    std::shared_ptr<NamedBitBuffer> nbb =
        NamedBitBuffer::create(name, buf.size() * 8, parent);
    // Copy data byte-by-byte.
    for (size_t i = 0; i < buf.size(); ++i)
      nbb->set(i, buf.get(i));
    return nbb;
  } else if (type == "String") {
    return NamedString::create(name, valueStr, parent);
  } else {
    // Fallback to base NamedObject for unknown or generic types.
    return NamedObject::create(name, parent);
  }
}

// --- XML ---

/**
 * @brief Recursive helper for XML serialization.
 */
void serializeToXml(XMLElement *element,
                    const std::shared_ptr<NamedObject> &obj) {
  element->SetAttribute("name", obj->getName().c_str());

  // Use dynamic_cast to identify the underlying core type and serialize its
  // value.
  const quasar::coretypes::Boolean *b =
      dynamic_cast<const quasar::coretypes::Boolean *>(obj.get());
  if (b) {
    element->SetAttribute("type", "Boolean");
    element->SetText(b->toString().c_str());
  } else {
    const quasar::coretypes::Number *n =
        dynamic_cast<const quasar::coretypes::Number *>(obj.get());
    if (n) {
      element->SetAttribute("type", n->getType().c_str());
      element->SetText(n->toString().c_str());
    } else {
      const quasar::coretypes::BitBuffer *bb =
          dynamic_cast<const quasar::coretypes::BitBuffer *>(obj.get());
      if (bb) {
        element->SetAttribute("type", "BitBuffer");
        element->SetText(bb->toString().c_str());
      } else {
        const quasar::coretypes::Buffer *buf =
            dynamic_cast<const quasar::coretypes::Buffer *>(obj.get());
        if (buf) {
          element->SetAttribute("type", "Buffer");
          element->SetText(buf->toString().c_str());
        } else {
          const quasar::coretypes::String *s =
              dynamic_cast<const quasar::coretypes::String *>(obj.get());
          if (s) {
            element->SetAttribute("type", "String");
            element->SetText(s->toString().c_str());
          } else {
            element->SetAttribute("type", "Object");
          }
        }
      }
    }
  }

  // Recursively serialize all children as nested elements.
  for (const std::shared_ptr<NamedObject> &child : obj->getChildren()) {
    XMLElement *childElem = element->GetDocument()->NewElement("NamedObject");
    serializeToXml(childElem, child);
    element->InsertEndChild(childElem);
  }
}

std::string toXml(const std::shared_ptr<NamedObject> &obj) {
  // Fulfills [FE-0020.9.4] XML conversion.
  XMLDocument doc;
  XMLElement *root = doc.NewElement("NamedObject");
  doc.InsertFirstChild(root);
  serializeToXml(root, obj);

  XMLPrinter printer;
  doc.Accept(&printer);
  return printer.CStr();
}

/**
 * @brief Recursive helper for XML deserialization.
 */
void deserializeFromXml(XMLElement *element,
                        std::shared_ptr<NamedObject> parent) {
  // Fulfills [FE-0020.9.4] XML conversion.
  const char *name = element->Attribute("name");
  const char *type = element->Attribute("type");
  const char *text = element->GetText();
  std::string valueStr = text ? text : "";
  std::string objName = name ? name : "unnamed";
  std::string objType = type ? type : "Object";

  // Create the object and automatically link it to the parent.
  std::shared_ptr<NamedObject> obj =
      createFromTypeAndValue(objName, objType, valueStr, parent);

  // Process all child elements.
  XMLElement *child = element->FirstChildElement("NamedObject");
  while (child) {
    deserializeFromXml(child, obj);
    child = child->NextSiblingElement("NamedObject");
  }
}

std::shared_ptr<NamedObject> fromXml(const std::string &xml) {
  // Fulfills [FE-0020.9.4] XML conversion.
  XMLDocument doc;
  if (doc.Parse(xml.c_str()) != XML_SUCCESS) {
    throw std::runtime_error("Failed to parse XML");
  }
  XMLElement *root = doc.FirstChildElement("NamedObject");
  if (!root)
    throw std::runtime_error("Invalid XML: missing root NamedObject");

  // Root processing.
  const char *name = root->Attribute("name");
  const char *type = root->Attribute("type");
  const char *text = root->GetText();
  std::string valueStr = text ? text : "";
  std::string objName = name ? name : "unnamed";
  std::string objType = type ? type : "Object";

  std::shared_ptr<NamedObject> obj =
      createFromTypeAndValue(objName, objType, valueStr, nullptr);

  XMLElement *child = root->FirstChildElement("NamedObject");
  while (child) {
    deserializeFromXml(child, obj);
    child = child->NextSiblingElement("NamedObject");
  }
  return obj;
}

// --- YAML ---

/**
 * @brief Recursive helper for YAML serialization.
 */
YAML::Node serializeToYaml(const std::shared_ptr<NamedObject> &obj) {
  // Fulfills [FE-0020.9.3] YAML conversion via yaml-cpp.
  YAML::Node node;
  node["name"] = obj->getName();

  // Extract type and value.
  if (const quasar::coretypes::Boolean *b =
          dynamic_cast<const quasar::coretypes::Boolean *>(obj.get())) {
    node["type"] = "Boolean";
    node["value"] = b->toString();
  } else if (const quasar::coretypes::Number *n =
                 dynamic_cast<const quasar::coretypes::Number *>(obj.get())) {
    node["type"] = n->getType();
    node["value"] = n->toString();
  } else if (const quasar::coretypes::BitBuffer *bb =
                 dynamic_cast<const quasar::coretypes::BitBuffer *>(
                     obj.get())) {
    node["type"] = "BitBuffer";
    node["value"] = bb->toString();
  } else if (const quasar::coretypes::Buffer *buf =
                 dynamic_cast<const quasar::coretypes::Buffer *>(obj.get())) {
    node["type"] = "Buffer";
    node["value"] = buf->toString();
  } else if (const quasar::coretypes::String *s =
                 dynamic_cast<const quasar::coretypes::String *>(obj.get())) {
    node["type"] = "String";
    node["value"] = s->toString();
  } else {
    node["type"] = "Object";
  }

  // Serialize children into a list.
  for (const std::shared_ptr<NamedObject> &child : obj->getChildren()) {
    node["children"].push_back(serializeToYaml(child));
  }
  return node;
}

std::string toYaml(const std::shared_ptr<NamedObject> &obj) {
  // Fulfills [FE-0020.9.3] YAML conversion.
  YAML::Node root = serializeToYaml(obj);
  YAML::Emitter out;
  out << root;
  return out.c_str();
}

/**
 * @brief Recursive helper for YAML deserialization.
 */
void deserializeFromYaml(const YAML::Node &node,
                         std::shared_ptr<NamedObject> parent) {
  // Fulfills [FE-0020.9.3] YAML conversion.
  std::string name = node["name"].as<std::string>();
  std::string type = node["type"].as<std::string>();
  std::string value = node["value"] ? node["value"].as<std::string>() : "";

  std::shared_ptr<NamedObject> obj =
      createFromTypeAndValue(name, type, value, parent);

  if (node["children"]) {
    for (const YAML::Node &child : node["children"]) {
      deserializeFromYaml(child, obj);
    }
  }
}

std::shared_ptr<NamedObject> fromYaml(const std::string &yaml) {
  // Fulfills [FE-0020.9.3] YAML conversion.
  YAML::Node root = YAML::Load(yaml);
  if (!root.IsDefined())
    throw std::runtime_error("Invalid YAML");

  std::string name = root["name"].as<std::string>();
  std::string type = root["type"].as<std::string>();
  std::string value = root["value"] ? root["value"].as<std::string>() : "";

  std::shared_ptr<NamedObject> obj =
      createFromTypeAndValue(name, type, value, nullptr);

  if (root["children"]) {
    for (const YAML::Node &child : root["children"]) {
      deserializeFromYaml(child, obj);
    }
  }
  return obj;
}

// --- JSON ---

using jsoncons::json;

/**
 * @brief Recursive helper for JSON serialization.
 */
json serializeToJson(const std::shared_ptr<NamedObject> &obj) {
  // Fulfills [FE-0020.9.2] JSON conversion via jsoncons.
  json j;
  j["name"] = obj->getName();

  // Map object properties to JSON fields.
  const quasar::coretypes::Boolean *b =
      dynamic_cast<const quasar::coretypes::Boolean *>(obj.get());
  if (b) {
    j["type"] = "Boolean";
    j["value"] = b->toString();
  } else {
    const quasar::coretypes::Number *n =
        dynamic_cast<const quasar::coretypes::Number *>(obj.get());
    if (n) {
      j["type"] = n->getType();
      j["value"] = n->toString();
    } else {
      const quasar::coretypes::BitBuffer *bb =
          dynamic_cast<const quasar::coretypes::BitBuffer *>(obj.get());
      if (bb) {
        j["type"] = "BitBuffer";
        j["value"] = bb->toString();
      } else {
        const quasar::coretypes::Buffer *buf =
            dynamic_cast<const quasar::coretypes::Buffer *>(obj.get());
        if (buf) {
          j["type"] = "Buffer";
          j["value"] = buf->toString();
        } else {
          const quasar::coretypes::String *s =
              dynamic_cast<const quasar::coretypes::String *>(obj.get());
          if (s) {
            j["type"] = "String";
            j["value"] = s->toString();
          } else {
            j["type"] = "Object";
          }
        }
      }
    }
  }

  // Process children as a JSON array.
  if (!obj->getChildren().empty()) {
    json children_json_array = json::array();
    for (const std::shared_ptr<NamedObject> &child : obj->getChildren()) {
      children_json_array.push_back(serializeToJson(child));
    }
    j["children"] = children_json_array;
  }
  return j;
}

std::string toJson(const std::shared_ptr<NamedObject> &obj) {
  // Fulfills [FE-0020.9.2] JSON conversion.
  json j = serializeToJson(obj);
  return j.to_string();
}

/**
 * @brief Recursive helper for JSON deserialization.
 */
void deserializeFromJson(const json &j, std::shared_ptr<NamedObject> parent) {
  // Fulfills [FE-0020.9.2] JSON conversion.
  std::string name = j["name"].as<std::string>();
  std::string type = j["type"].as<std::string>();
  std::string value = j.contains("value") ? j["value"].as<std::string>() : "";

  std::shared_ptr<NamedObject> obj =
      createFromTypeAndValue(name, type, value, parent);

  if (j.contains("children")) {
    for (const auto &child : j["children"].array_range()) {
      deserializeFromJson(child, obj);
    }
  }
}

std::shared_ptr<NamedObject> fromJson(const std::string &jsonStr) {
  // Fulfills [FE-0020.9.2] JSON conversion.
  json j = json::parse(jsonStr);
  std::string name = j["name"].as<std::string>();
  std::string type = j["type"].as<std::string>();
  std::string value = j.contains("value") ? j["value"].as<std::string>() : "";

  std::shared_ptr<NamedObject> obj =
      createFromTypeAndValue(name, type, value, nullptr);

  if (j.contains("children")) {
    for (const auto &child : j["children"].array_range()) {
      deserializeFromJson(child, obj);
    }
  }
  return obj;
}

} // namespace quasar::named::serialization