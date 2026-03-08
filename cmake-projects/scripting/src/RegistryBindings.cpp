#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/scripting/ScriptableNamedObject.hpp"
#include "quasar/scripting/LuaService.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedQuantity.hpp"
#include "quasar/named/NamedVariant.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedTimestamp.hpp"
#include "quasar/named/NamedDuration.hpp"
#include "quasar/named/NamedDate.hpp"
#include <sstream>
#include <iostream>

namespace quasar::scripting {

using namespace quasar::named;

/**
 * @brief Helper to resolve a path in a NamedObject hierarchy.
 */
std::shared_ptr<NamedObject> resolvePath(std::shared_ptr<NamedObject> root, const std::string& path) {
    if (!root || path.empty()) return root;
    
    std::stringstream ss(path);
    std::string segment;
    std::shared_ptr<NamedObject> current = root;
    
    while (std::getline(ss, segment, '/')) {
        if (segment.empty()) continue; 
        current = current->getChild(segment);
        if (!current) return nullptr;
    }
    
    return current;
}

void bindNamedTypes(sol::state_view lua) {
    // --- NamedObject ---
    sol::usertype<NamedObject> utNamedObject = lua.new_usertype<NamedObject>("NamedObject", sol::no_constructor);
    utNamedObject["getName"] = &NamedObject::getName;
    utNamedObject["setName"] = &NamedObject::setName;
    utNamedObject["getParent"] = &NamedObject::getParent;
    utNamedObject["setParent"] = &NamedObject::setParent;
    utNamedObject["getChildren"] = &NamedObject::getChildren;
    utNamedObject["getChild"] = &NamedObject::getChild;
    utNamedObject["getType"] = &NamedObject::getType;
    utNamedObject["getSelf"] = &NamedObject::getSelf;
    utNamedObject["clone"] = &NamedObject::clone;
    utNamedObject["deepCopy"] = sol::overload(
        static_cast<std::shared_ptr<NamedObject>(NamedObject::*)() const>(&NamedObject::deepCopy)
    );
    // Safe casting helpers
    utNamedObject["asLong"] = [](std::shared_ptr<NamedObject> obj) { return std::dynamic_pointer_cast<NamedInteger<int64_t>>(obj); };
    utNamedObject["asDouble"] = [](std::shared_ptr<NamedObject> obj) { return std::dynamic_pointer_cast<NamedFloatingPoint<double>>(obj); };
    utNamedObject["asQuantity"] = [](std::shared_ptr<NamedObject> obj) { return std::dynamic_pointer_cast<NamedQuantity>(obj); };
    utNamedObject["asVariant"] = [](std::shared_ptr<NamedObject> obj) { return std::dynamic_pointer_cast<NamedVariant>(obj); };
    utNamedObject["asScriptable"] = [](std::shared_ptr<NamedObject> obj) { return std::dynamic_pointer_cast<ScriptableNamedObject>(obj); };

    // --- ScriptableNamedObject ---
    sol::usertype<ScriptableNamedObject> utScriptable = lua.new_usertype<ScriptableNamedObject>("ScriptableNamedObject", sol::base_classes, sol::bases<NamedObject>());
    utScriptable["new"] = [](const std::string& name, sol::optional<std::shared_ptr<NamedObject>> parent) {
        return std::static_pointer_cast<NamedObject>(ScriptableNamedObject::create(name, parent.value_or(nullptr)));
    };
    utScriptable["create"] = [](const std::string& name, sol::optional<std::shared_ptr<NamedObject>> parent) {
        return std::static_pointer_cast<NamedObject>(ScriptableNamedObject::create(name, parent.value_or(nullptr)));
    };
    utScriptable["setLuaSelf"] = &ScriptableNamedObject::setLuaSelf;
    utScriptable["getLuaSelf"] = &ScriptableNamedObject::getLuaSelf;
    utScriptable["onEvent"] = &ScriptableNamedObject::onEvent;

    // --- NamedLong ---
    using NamedLong = NamedInteger<int64_t>;
    sol::usertype<NamedLong> utNamedLong = lua.new_usertype<NamedLong>("NamedLong", sol::base_classes, sol::bases<NamedObject>());
    utNamedLong["new"] = [](const std::string& name, int64_t value, sol::optional<std::shared_ptr<NamedObject>> parent) {
        return std::static_pointer_cast<NamedObject>(NamedLong::create(name, value, parent.value_or(nullptr)));
    };
    utNamedLong["create"] = [](const std::string& name, int64_t value, sol::optional<std::shared_ptr<NamedObject>> parent) {
        return std::static_pointer_cast<NamedObject>(NamedLong::create(name, value, parent.value_or(nullptr)));
    };
    utNamedLong["value"] = &NamedLong::value;
    utNamedLong[sol::meta_function::to_string] = static_cast<std::string(NamedLong::*)() const>(&NamedLong::toString);

    // --- NamedDouble ---
    using NamedDouble = NamedFloatingPoint<double>;
    sol::usertype<NamedDouble> utNamedDouble = lua.new_usertype<NamedDouble>("NamedDouble", sol::base_classes, sol::bases<NamedObject>());
    utNamedDouble["new"] = [](const std::string& name, double value, sol::optional<std::shared_ptr<NamedObject>> parent) {
        return std::static_pointer_cast<NamedObject>(NamedDouble::create(name, value, parent.value_or(nullptr)));
    };
    utNamedDouble["create"] = [](const std::string& name, double value, sol::optional<std::shared_ptr<NamedObject>> parent) {
        return std::static_pointer_cast<NamedObject>(NamedDouble::create(name, value, parent.value_or(nullptr)));
    };
    utNamedDouble["value"] = &NamedDouble::value;
    utNamedDouble[sol::meta_function::to_string] = static_cast<std::string(NamedDouble::*)() const>(&NamedDouble::toString);

    // --- NamedQuantity ---
    sol::usertype<NamedQuantity> utNamedQuantity = lua.new_usertype<NamedQuantity>("NamedQuantity", sol::base_classes, sol::bases<NamedObject>());
    utNamedQuantity["new"] = sol::overload(
        [](const std::string& name, double value, const quasar::coretypes::Unit& unit, sol::optional<std::shared_ptr<NamedObject>> parent) {
            return std::static_pointer_cast<NamedObject>(NamedQuantity::create(name, value, unit, parent.value_or(nullptr)));
        },
        [](const std::string& name, double value, const std::string& unitSymbol, sol::optional<std::shared_ptr<NamedObject>> parent) {
            return std::static_pointer_cast<NamedObject>(NamedQuantity::create(name, value, unitSymbol, parent.value_or(nullptr)));
        }
    );
    utNamedQuantity["create"] = sol::overload(
        [](const std::string& name, double value, const quasar::coretypes::Unit& unit, sol::optional<std::shared_ptr<NamedObject>> parent) {
            return std::static_pointer_cast<NamedObject>(NamedQuantity::create(name, value, unit, parent.value_or(nullptr)));
        },
        [](const std::string& name, double value, const std::string& unitSymbol, sol::optional<std::shared_ptr<NamedObject>> parent) {
            return std::static_pointer_cast<NamedObject>(NamedQuantity::create(name, value, unitSymbol, parent.value_or(nullptr)));
        }
    );
    utNamedQuantity["value"] = &NamedQuantity::value;
    utNamedQuantity["getUnitSymbol"] = &NamedQuantity::getUnitSymbol;
    utNamedQuantity[sol::meta_function::to_string] = static_cast<std::string(NamedQuantity::*)() const>(&NamedQuantity::toString);

    // --- NamedVariant ---
    sol::usertype<NamedVariant> utNamedVariant = lua.new_usertype<NamedVariant>("NamedVariant", sol::base_classes, sol::bases<NamedObject>());
    utNamedVariant["new"] = [](const std::string& name, sol::optional<std::shared_ptr<NamedObject>> parent) {
        return std::static_pointer_cast<NamedObject>(NamedVariant::create(name, parent.value_or(nullptr)));
    };
    utNamedVariant["create"] = [](const std::string& name, sol::optional<std::shared_ptr<NamedObject>> parent) {
        return std::static_pointer_cast<NamedObject>(NamedVariant::create(name, parent.value_or(nullptr)));
    };
    utNamedVariant["set"] = [](NamedVariant& self, std::shared_ptr<NamedObject> obj) {
        self.set(obj);
    };
    utNamedVariant["get"] = &NamedVariant::get;
    utNamedVariant["getType"] = &NamedVariant::getType;

    // --- LuaService ---
    sol::usertype<LuaService> utLuaService = lua.new_usertype<LuaService>("LuaService", sol::base_classes, sol::bases<named::NamedObject, ScriptComponent>());
    utLuaService["new"] = [](const std::string& name, sol::optional<std::shared_ptr<named::NamedObject>> parent) {
        return std::static_pointer_cast<named::NamedObject>(LuaService::create(name, parent.value_or(nullptr)));
    };
    utLuaService["create"] = [](const std::string& name, sol::optional<std::shared_ptr<named::NamedObject>> parent) {
        return std::static_pointer_cast<named::NamedObject>(LuaService::create(name, parent.value_or(nullptr)));
    };
    utLuaService["loadScript"] = &LuaService::loadScript;
    utLuaService["onInit"] = &LuaService::onInit;
    utLuaService["onUpdate"] = &LuaService::onUpdate;
    utLuaService["onShutdown"] = &LuaService::onShutdown;
    utLuaService["asService"] = [](std::shared_ptr<named::NamedObject> obj) { return std::dynamic_pointer_cast<LuaService>(obj); };

    // --- Global quasar table ---
    sol::table quasarTable = lua.create_named_table("quasar");
    quasarTable["resolve"] = &resolvePath;
    
    // Lifecycle Tracking
    quasarTable["track"] = [](std::shared_ptr<named::NamedObject> obj) {
        ObjectTracker::getInstance().track(obj);
    };
    quasarTable["isAlive"] = [](std::shared_ptr<named::NamedObject> obj) {
        return ObjectTracker::getInstance().isAlive(obj);
    };
    quasarTable["cleanupTracker"] = []() {
        ObjectTracker::getInstance().cleanup();
    };
}

} // namespace quasar::scripting
