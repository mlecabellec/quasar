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
std::shared_ptr<NamedObject>
createFromTypeAndValue(const std::string &name, const std::string &type,
                       const std::string &valueStr,
                       std::shared_ptr<NamedObject> parent) {
  if (type == "NamedBoolean" || type == "Boolean") {
    return NamedBoolean::create(name, valueStr == "true", parent);
  } else if (type == "NamedInteger" || type == "Integer") {
    try {
      return NamedInteger<int32_t>::create(name, std::stoi(valueStr), parent);
    } catch (...) {
      return NamedInteger<int64_t>::create(name, std::stoll(valueStr), parent);
    }
  } else if (type == "NamedFloatingPoint" || type == "FloatingPoint") {
    return NamedFloatingPoint<double>::create(name, std::stod(valueStr), parent);
  } else if (type == "NamedBuffer" || type == "Buffer") {
    return NamedBuffer::create(name, quasar::coretypes::Buffer::fromString(valueStr).toVector(), parent);
  } else if (type == "NamedBitBuffer" || type == "BitBuffer") {
    quasar::coretypes::Buffer buf = quasar::coretypes::Buffer::fromString(valueStr);
    std::shared_ptr<NamedBitBuffer> nbb = NamedBitBuffer::create(name, buf.size() * 8, parent);
    for (size_t i = 0; i < buf.size(); i++) nbb->set(i, buf.get(i));
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
std::string getValueAsString(const std::shared_ptr<NamedObject>& obj) {
    if (auto b = dynamic_cast<const quasar::coretypes::Boolean*>(obj.get())) return b->toString();
    if (auto n = dynamic_cast<const quasar::coretypes::Number*>(obj.get())) return n->toString();
    if (auto bb = dynamic_cast<const quasar::coretypes::BitBuffer*>(obj.get())) return bb->toString();
    if (auto buf = dynamic_cast<const quasar::coretypes::Buffer*>(obj.get())) return buf->toString();
    if (auto s = dynamic_cast<const quasar::coretypes::String*>(obj.get())) return s->toString();
    return "";
}

// --- XML ---
void serializeToXml(XMLElement *element, const std::shared_ptr<NamedObject> &obj) {
  element->SetAttribute("name", obj->getName().c_str());
  element->SetAttribute("type", obj->getType().c_str());
  std::string val = getValueAsString(obj);
  if (!val.empty()) element->SetText(val.c_str());

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
  const char *name = element->Attribute("name");
  const char *type = element->Attribute("type");
  const char *text = element->GetText();
  std::shared_ptr<NamedObject> obj = createFromTypeAndValue(name ? name : "unnamed", type ? type : "Object", text ? text : "", parent);
  XMLElement *child = element->FirstChildElement("NamedObject");
  while (child) {
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
  node["type"] = obj->getType();
  std::string val = getValueAsString(obj);
  if (!val.empty()) node["value"] = val;
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

void deserializeFromYaml(const YAML::Node &node, std::shared_ptr<NamedObject> parent) {
  std::string name = node["name"].as<std::string>();
  std::string type = node["type"].as<std::string>();
  std::string value = node["value"] ? node["value"].as<std::string>() : "";
  std::shared_ptr<NamedObject> obj = createFromTypeAndValue(name, type, value, parent);
  if (node["children"]) {
    for (const YAML::Node &child : node["children"]) deserializeFromYaml(child, obj);
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
    for (const YAML::Node &child : root["children"]) deserializeFromYaml(child, obj);
  }
  return obj;
}

// --- JSON ---
using jsoncons::json;

json serializeToJson(const std::shared_ptr<NamedObject> &obj) {
  json j;
  j["name"] = obj->getName();
  j["type"] = obj->getType();
  std::string val = getValueAsString(obj);
  if (!val.empty()) j["value"] = val;
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
  return serializeToJson(obj).to_string();
}

void deserializeFromJson(const json &j, std::shared_ptr<NamedObject> parent) {
  std::string name = j["name"].as<std::string>();
  std::string type = j["type"].as<std::string>();
  std::string value = j.contains("value") ? j["value"].as<std::string>() : "";
  std::shared_ptr<NamedObject> obj = createFromTypeAndValue(name, type, value, parent);
  if (j.contains("children")) {
    for (const auto &child : j["children"].array_range()) deserializeFromJson(child, obj);
  }
}

std::shared_ptr<NamedObject> fromJson(const std::string &jsonStr) {
  json j = json::parse(jsonStr);
  std::string name = j["name"].as<std::string>();
  std::string type = j["type"].as<std::string>();
  std::string value = j.contains("value") ? j["value"].as<std::string>() : "";
  std::shared_ptr<NamedObject> obj = createFromTypeAndValue(name, type, value, nullptr);
  if (j.contains("children")) {
    for (const auto &child : j["children"].array_range()) deserializeFromJson(child, obj);
  }
  return obj;
}

} // namespace quasar::named::serialization