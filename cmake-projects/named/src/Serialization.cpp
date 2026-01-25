#include "quasar/named/Serialization.h"
#include "quasar/named/NamedBoolean.h"
#include "quasar/named/NamedInteger.h"
#include "quasar/named/NamedFloatingPoint.h"
#include "quasar/named/NamedBuffer.h"
#include "quasar/named/NamedBitBuffer.h"

#include <tinyxml2.h>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <stdexcept>

namespace quasar::named::serialization {

using namespace tinyxml2;

void serializeToXml(XMLElement* element, const std::shared_ptr<NamedObject>& obj) {
    element->SetAttribute("name", obj->getName().c_str());

    if (auto b = dynamic_cast<const quasar::coretypes::Boolean*>(obj.get())) {
        element->SetAttribute("type", "Boolean");
        element->SetText(b->toString().c_str());
    } else if (auto n = dynamic_cast<const quasar::coretypes::Number*>(obj.get())) {
        element->SetAttribute("type", "Number");
        element->SetText(n->toString().c_str());
    } else if (auto bb = dynamic_cast<const quasar::coretypes::BitBuffer*>(obj.get())) {
        element->SetAttribute("type", "BitBuffer");
        element->SetText(bb->toString().c_str());
    } else if (auto buf = dynamic_cast<const quasar::coretypes::Buffer*>(obj.get())) {
        element->SetAttribute("type", "Buffer");
        element->SetText(buf->toString().c_str());
    } else {
        element->SetAttribute("type", "Object");
    }

    for (const auto& child : obj->getChildren()) {
        XMLElement* childElem = element->GetDocument()->NewElement("NamedObject");
        serializeToXml(childElem, child);
        element->InsertEndChild(childElem);
    }
}

std::string toXml(const std::shared_ptr<NamedObject>& obj) {
    XMLDocument doc;
    XMLElement* root = doc.NewElement("NamedObject");
    doc.InsertFirstChild(root);
    serializeToXml(root, obj);
    
    XMLPrinter printer;
    doc.Accept(&printer);
    return printer.CStr();
}

std::shared_ptr<NamedObject> fromXml(const std::string& xml) {
    (void)xml;
    throw std::runtime_error("XML Deserialization not fully implemented");
}

YAML::Node serializeToYaml(const std::shared_ptr<NamedObject>& obj) {
    YAML::Node node;
    node["name"] = obj->getName();
    
    if (auto b = dynamic_cast<const quasar::coretypes::Boolean*>(obj.get())) {
        node["type"] = "Boolean";
        node["value"] = b->booleanValue();
    } else if (auto n = dynamic_cast<const quasar::coretypes::Number*>(obj.get())) {
        node["type"] = "Number";
        node["value"] = n->toString(); 
    } else if (auto bb = dynamic_cast<const quasar::coretypes::BitBuffer*>(obj.get())) {
        node["type"] = "BitBuffer";
        node["value"] = bb->toString();
    } else if (auto buf = dynamic_cast<const quasar::coretypes::Buffer*>(obj.get())) {
        node["type"] = "Buffer";
        node["value"] = buf->toString();
    } else {
        node["type"] = "Object";
    }
    
    for (const auto& child : obj->getChildren()) {
        node["children"].push_back(serializeToYaml(child));
    }
    return node;
}

std::string toYaml(const std::shared_ptr<NamedObject>& obj) {
    YAML::Node root = serializeToYaml(obj);
    YAML::Emitter out;
    out << root;
    return out.c_str();
}

std::shared_ptr<NamedObject> fromYaml(const std::string& yaml) {
    (void)yaml;
    throw std::runtime_error("YAML Deserialization not fully implemented");
}

std::string toJson(const std::shared_ptr<NamedObject>& obj) {
    (void)obj;
    throw std::runtime_error("JSON Serialization requires jsoncons (not found)");
}

std::shared_ptr<NamedObject> fromJson(const std::string& json) {
    (void)json;
    throw std::runtime_error("JSON Deserialization requires jsoncons (not found)");
}

} // namespace quasar::named::serialization
