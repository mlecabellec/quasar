#include "quasar/named/Serialization.hpp"
#include "quasar/named/NamedBitBuffer.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedInteger.hpp"

#include <iostream>
#include <stdexcept>
#include <tinyxml2.h>
#include <yaml-cpp/yaml.h>

namespace quasar::named::serialization {

using namespace tinyxml2;

void serializeToXml(XMLElement *element,
                    const std::shared_ptr<NamedObject> &obj) {
  element->SetAttribute("name", obj->getName().c_str());

  if (auto b = dynamic_cast<const quasar::coretypes::Boolean *>(obj.get())) {
    // Handle Boolean type.
    element->SetAttribute("type", "Boolean");
    element->SetText(b->toString().c_str());
  } else if (auto n =
                 dynamic_cast<const quasar::coretypes::Number *>(obj.get())) {
    // Handle Number types (Integer, FloatingPoint).
    element->SetAttribute("type", "Number");
    element->SetText(n->toString().c_str());
  } else if (auto bb = dynamic_cast<const quasar::coretypes::BitBuffer *>(
                 obj.get())) {
    element->SetAttribute("type", "BitBuffer");
    element->SetText(bb->toString().c_str());
  } else if (auto buf =
                 dynamic_cast<const quasar::coretypes::Buffer *>(obj.get())) {
    // Handle Buffer type.
    element->SetAttribute("type", "Buffer");
    element->SetText(buf->toString().c_str());
  } else {
    // Default object type.
    element->SetAttribute("type", "Object");
  }

  for (const std::shared_ptr<NamedObject> &child : obj->getChildren()) {
    // Recursively serialize children.
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

  // Convert to string.
  XMLPrinter printer;
  doc.Accept(&printer);
  return printer.CStr();
}

std::shared_ptr<NamedObject> fromXml(const std::string &xml) {
  (void)xml;
  throw std::runtime_error("XML Deserialization not fully implemented");
}

YAML::Node serializeToYaml(const std::shared_ptr<NamedObject> &obj) {
  YAML::Node node;
  node["name"] = obj->getName();

  if (const quasar::coretypes::Boolean *b =
          dynamic_cast<const quasar::coretypes::Boolean *>(obj.get())) {
    // Handle Boolean.
    node["type"] = "Boolean";
    node["value"] = b->booleanValue();
  } else if (const quasar::coretypes::Number *n =
                 dynamic_cast<const quasar::coretypes::Number *>(obj.get())) {
    // Handle Number.
    node["type"] = "Number";
    node["value"] = n->toString();
  } else if (const quasar::coretypes::BitBuffer *bb =
                 dynamic_cast<const quasar::coretypes::BitBuffer *>(
                     obj.get())) {
    node["type"] = "BitBuffer";
    node["value"] = bb->toString();
  } else if (const quasar::coretypes::Buffer *buf =
                 dynamic_cast<const quasar::coretypes::Buffer *>(obj.get())) {
    // Handle Buffer.
    node["type"] = "Buffer";
    node["value"] = buf->toString();
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

std::shared_ptr<NamedObject> fromYaml(const std::string &yaml) {
  (void)yaml;
  throw std::runtime_error("YAML Deserialization not fully implemented");
}

std::string toJson(const std::shared_ptr<NamedObject> &obj) {
  (void)obj;
  throw std::runtime_error("JSON Serialization requires jsoncons (not found)");
}

std::shared_ptr<NamedObject> fromJson(const std::string &json) {
  (void)json;
  throw std::runtime_error(
      "JSON Deserialization requires jsoncons (not found)");
}

} // namespace quasar::named::serialization
