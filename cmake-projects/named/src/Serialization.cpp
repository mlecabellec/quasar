#include "quasar/named/Serialization.hpp"
#include "quasar/named/NamedBitBuffer.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedString.hpp"

#include <iostream>
#include <stdexcept>
#include <tinyxml2.h>
#include <yaml-cpp/yaml.h>
#include <jsoncons/json.hpp>

namespace quasar::named::serialization {

using namespace tinyxml2;

// --- Helper Factory ---
std::shared_ptr<NamedObject> createFromTypeAndValue(const std::string& name, const std::string& type, const std::string& valueStr, std::shared_ptr<NamedObject> parent) {
    if (type == "Boolean") {
        return NamedBoolean::create(name, valueStr == "true", parent);
    } else if (type == "Integer") {
        // Default to Int (int32) or Long (int64) if needed?
        // Let's use Int for now or Long if huge?
        // Simple heuristic: fit in int?
        try {
            return NamedInteger<int32_t>::create(name, std::stoi(valueStr), parent);
        } catch (...) {
             return NamedInteger<int64_t>::create(name, std::stoll(valueStr), parent);
        }
    } else if (type == "FloatingPoint") {
        return NamedFloatingPoint<double>::create(name, std::stod(valueStr), parent);
    } else if (type == "Buffer") {
        return NamedBuffer::create(name, quasar::coretypes::Buffer::fromString(valueStr).toVector(), parent);
    } else if (type == "BitBuffer") {
        // BitBuffer from string? coretypes::BitBuffer doesn't strictly have fromString for bits?
        // Assuming hex string same as buffer for now, but size logic is tricky.
        // Let's assume it's serialized as Buffer hex.
        // We need bitCount. 
        // Current serialization just calls toString() which is hex.
        // We lose valid bit count info if not stored.
        // We will create based on bytes * 8.
        auto buf = quasar::coretypes::Buffer::fromString(valueStr);
        auto nbb = NamedBitBuffer::create(name, buf.size() * 8, parent);
        // Copy data
        for(size_t i=0; i<buf.size(); ++i) nbb->set(i, buf.get(i));
        return nbb;
    } else if (type == "String") {
        return NamedString::create(name, valueStr, parent);
    } else {
        return NamedObject::create(name, parent);
    }
}

// --- XML ---

void serializeToXml(XMLElement *element,
                    const std::shared_ptr<NamedObject> &obj) {
  element->SetAttribute("name", obj->getName().c_str());

  if (auto b = dynamic_cast<const quasar::coretypes::Boolean *>(obj.get())) {
    element->SetAttribute("type", "Boolean");
    element->SetText(b->toString().c_str());
  } else if (auto n = dynamic_cast<const quasar::coretypes::Number *>(obj.get())) {
    element->SetAttribute("type", n->getType().c_str()); // Integer or FloatingPoint
    element->SetText(n->toString().c_str());
  } else if (auto bb = dynamic_cast<const quasar::coretypes::BitBuffer *>(obj.get())) {
    element->SetAttribute("type", "BitBuffer");
    element->SetText(bb->toString().c_str());
  } else if (auto buf = dynamic_cast<const quasar::coretypes::Buffer *>(obj.get())) {
    element->SetAttribute("type", "Buffer");
    element->SetText(buf->toString().c_str());
  } else if (auto s = dynamic_cast<const quasar::coretypes::String *>(obj.get())) {
    element->SetAttribute("type", "String");
    element->SetText(s->toString().c_str());
  } else {
    element->SetAttribute("type", "Object");
  }

  for (const std::shared_ptr<NamedObject> &child : obj->getChildren()) {
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

void deserializeFromXml(XMLElement *element, std::shared_ptr<NamedObject> parent) {
    const char* name = element->Attribute("name");
    const char* type = element->Attribute("type");
    const char* text = element->GetText();
    std::string valueStr = text ? text : "";
    std::string objName = name ? name : "unnamed";
    std::string objType = type ? type : "Object";

    std::shared_ptr<NamedObject> obj = createFromTypeAndValue(objName, objType, valueStr, parent);

    XMLElement *child = element->FirstChildElement("NamedObject");
    while (child) {
        deserializeFromXml(child, obj);
        child = child->NextSiblingElement("NamedObject");
    }
}

std::shared_ptr<NamedObject> fromXml(const std::string &xml) {
  XMLDocument doc;
  if (doc.Parse(xml.c_str()) != XML_SUCCESS) {
      throw std::runtime_error("Failed to parse XML");
  }
  XMLElement *root = doc.FirstChildElement("NamedObject");
  if (!root) throw std::runtime_error("Invalid XML: missing root NamedObject");
  
  // Root has no parent initially
  const char* name = root->Attribute("name");
  const char* type = root->Attribute("type");
  const char* text = root->GetText();
  std::string valueStr = text ? text : "";
  std::string objName = name ? name : "unnamed";
  std::string objType = type ? type : "Object";

  std::shared_ptr<NamedObject> obj = createFromTypeAndValue(objName, objType, valueStr, nullptr);
  
  XMLElement *child = root->FirstChildElement("NamedObject");
  while (child) {
      deserializeFromXml(child, obj);
      child = child->NextSiblingElement("NamedObject");
  }
  return obj;
}

// --- YAML ---

YAML::Node serializeToYaml(const std::shared_ptr<NamedObject> &obj) {
  YAML::Node node;
  node["name"] = obj->getName();

  if (const quasar::coretypes::Boolean *b = dynamic_cast<const quasar::coretypes::Boolean *>(obj.get())) {
    node["type"] = "Boolean";
    node["value"] = b->toString();
  } else if (const quasar::coretypes::Number *n = dynamic_cast<const quasar::coretypes::Number *>(obj.get())) {
    node["type"] = n->getType();
    node["value"] = n->toString();
  } else if (const quasar::coretypes::BitBuffer *bb = dynamic_cast<const quasar::coretypes::BitBuffer *>(obj.get())) {
    node["type"] = "BitBuffer";
    node["value"] = bb->toString();
  } else if (const quasar::coretypes::Buffer *buf = dynamic_cast<const quasar::coretypes::Buffer *>(obj.get())) {
    node["type"] = "Buffer";
    node["value"] = buf->toString();
  } else if (const quasar::coretypes::String *s = dynamic_cast<const quasar::coretypes::String *>(obj.get())) {
    node["type"] = "String";
    node["value"] = s->toString();
  } else {
    node["type"] = "Object";
  }

  for (const std::shared_ptr<NamedObject> &child : obj->getChildren()) {
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

void deserializeFromYaml(const YAML::Node& node, std::shared_ptr<NamedObject> parent) {
    std::string name = node["name"].as<std::string>();
    std::string type = node["type"].as<std::string>();
    std::string value = node["value"] ? node["value"].as<std::string>() : "";

    std::shared_ptr<NamedObject> obj = createFromTypeAndValue(name, type, value, parent);

    if (node["children"]) {
        for (const auto& child : node["children"]) {
            deserializeFromYaml(child, obj);
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
      for (const auto& child : root["children"]) {
          deserializeFromYaml(child, obj);
      }
  }
  return obj;
}

// --- JSON ---

using jsoncons::json;

json serializeToJson(const std::shared_ptr<NamedObject> &obj) {
    json j;
    j["name"] = obj->getName();

    if (auto b = dynamic_cast<const quasar::coretypes::Boolean *>(obj.get())) {
        j["type"] = "Boolean";
        j["value"] = b->toString();
    } else if (auto n = dynamic_cast<const quasar::coretypes::Number *>(obj.get())) {
        j["type"] = n->getType();
        j["value"] = n->toString();
    } else if (auto bb = dynamic_cast<const quasar::coretypes::BitBuffer *>(obj.get())) {
        j["type"] = "BitBuffer";
        j["value"] = bb->toString();
    } else if (auto buf = dynamic_cast<const quasar::coretypes::Buffer *>(obj.get())) {
        j["type"] = "Buffer";
        j["value"] = buf->toString();
    } else if (auto s = dynamic_cast<const quasar::coretypes::String *>(obj.get())) {
        j["type"] = "String";
        j["value"] = s->toString();
    } else {
        j["type"] = "Object";
    }

    if (!obj->getChildren().empty()) {
        json children = json::array();
        for (const std::shared_ptr<NamedObject> &child : obj->getChildren()) {
            children.push_back(serializeToJson(child));
        }
        j["children"] = children;
    }
    return j;
}

std::string toJson(const std::shared_ptr<NamedObject> &obj) {
  json j = serializeToJson(obj);
  return j.to_string();
}

void deserializeFromJson(const json& j, std::shared_ptr<NamedObject> parent) {
    std::string name = j["name"].as<std::string>();
    std::string type = j["type"].as<std::string>();
    std::string value = j.contains("value") ? j["value"].as<std::string>() : "";

    std::shared_ptr<NamedObject> obj = createFromTypeAndValue(name, type, value, parent);

    if (j.contains("children")) {
        for (const auto& child : j["children"].array_range()) {
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
        for (const auto& child : j["children"].array_range()) {
            deserializeFromJson(child, obj);
        }
    }
    return obj;
}

} // namespace quasar::named::serialization