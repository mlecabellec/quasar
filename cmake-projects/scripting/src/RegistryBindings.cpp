#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ScriptableNamedObject.hpp"
#include "quasar/scripting/LuaService.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/QueuedObserver.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedQuantity.hpp"
#include "quasar/named/NamedVariant.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedTimestamp.hpp"
#include "quasar/named/NamedDuration.hpp"
#include "quasar/named/NamedDate.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedBitBuffer.hpp"
#include "quasar/named/NamedArray.hpp"
#include "quasar/named/NamedMap.hpp"
#include "quasar/named/ActiveEntity.hpp"
#include "quasar/named/NamedMethod.hpp"
#include "quasar/named/NamedService.hpp"
#include "quasar/named/Serialization.hpp"
#include "quasar/scripting/NamedLuaMethod.hpp"
#include <sstream>
#include <iostream>
#include <optional>
#include <map>
#include <cstdint>
#include <vector>
#include <list>
#include <memory>
#include <algorithm>

namespace quasar::scripting {

using namespace quasar::named;

size_t getEngineId(sol::this_state L) {
    sol::state_view lua(L);
    sol::object engineObj = lua["__quasar_engine"];
    if (engineObj.is<LuaEngine*>()) {
        return engineObj.as<LuaEngine*>()->getId();
    }
    return 0;
}

std::shared_ptr<NamedObject> extractNamedObject(sol::object obj) {
    if (!obj.valid() || obj.is<sol::nil_t>()) return nullptr;
    if (obj.is<ILuaProxy>()) return obj.as<ILuaProxy&>().lockAsNamedObject();
    return nullptr;
}

template<typename T, typename U>
static void bindNamedObjectMethods(U& ut) {
    ut["getName"] = [](LuaProxy<T> self) { return self.lock()->getName(); };
    ut["setName"] = [](LuaProxy<T> self, const std::string& name) { self.lock()->setName(name); };
    ut["getType"] = [](LuaProxy<T> self) { return self.lock()->getType(); };
    ut["isAlive"] = [](LuaProxy<T> self) { return self.isAlive(); };
    ut["getParent"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedObject> p = self.lock()->getParent();
        return p ? std::make_optional(LuaProxy<NamedObject>(p)) : std::nullopt;
    };
    ut["getChild"] = [](LuaProxy<T> self, const std::string& name) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedObject> c = self.lock()->getChild(name);
        return c ? std::make_optional(LuaProxy<NamedObject>(c)) : std::nullopt;
    };
    ut["getChildren"] = [](LuaProxy<T> self) {
        std::list<std::shared_ptr<NamedObject>> children = self.lock()->getChildren();
        std::vector<LuaProxy<NamedObject>> proxies;
        proxies.reserve(children.size());
        for (std::shared_ptr<NamedObject> const& c : children) proxies.emplace_back(c);
        return proxies;
    };
    
    ut["asLong"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedInteger<int64_t>>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedInteger<int64_t>> p = std::dynamic_pointer_cast<NamedInteger<int64_t>>(obj);
        return p ? std::make_optional(LuaProxy<NamedInteger<int64_t>>(p)) : std::nullopt;
    };
    ut["asULong"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedInteger<uint64_t>>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedInteger<uint64_t>> p = std::dynamic_pointer_cast<NamedInteger<uint64_t>>(obj);
        return p ? std::make_optional(LuaProxy<NamedInteger<uint64_t>>(p)) : std::nullopt;
    };
    ut["asDouble"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedFloatingPoint<double>>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedFloatingPoint<double>> p = std::dynamic_pointer_cast<NamedFloatingPoint<double>>(obj);
        return p ? std::make_optional(LuaProxy<NamedFloatingPoint<double>>(p)) : std::nullopt;
    };
    ut["asBoolean"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedBoolean>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedBoolean> p = std::dynamic_pointer_cast<NamedBoolean>(obj);
        return p ? std::make_optional(LuaProxy<NamedBoolean>(p)) : std::nullopt;
    };
    ut["asString"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedString>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedString> p = std::dynamic_pointer_cast<NamedString>(obj);
        return p ? std::make_optional(LuaProxy<NamedString>(p)) : std::nullopt;
    };
}

void bindNamedTypes(sol::state_view lua, std::shared_ptr<LuaService> service) {
    // Phase 5: Container Hardening
    lua.new_usertype<std::vector<uint8_t>>("VectorUInt8", sol::no_constructor);

    sol::table quasarTable = lua["quasar"].get_or_create<sol::table>();
    sol::table namedTable = quasarTable["named"].get_or_create<sol::table>();
    sol::table serializationTable = namedTable["serialization"].get_or_create<sol::table>();

    serializationTable["fromJson"] = [](const std::string& json, sol::this_state L) {
        std::shared_ptr<NamedObject> ptr = quasar::named::serialization::fromJson(json);
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedObject>(ptr);
    };

    lua.new_usertype<ILuaProxy>("ILuaProxy", sol::no_constructor, "isAlive", &ILuaProxy::isAlive);
    
    // NamedObject
    sol::usertype<LuaProxy<NamedObject>> utNamedObject = lua.new_usertype<LuaProxy<NamedObject>>("NamedObject", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedObject>(utNamedObject);
    namedTable["createObject"] = [](const std::string& name, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedObject> ptr = NamedObject::create(name, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedObject>(ptr);
    };

    // Scriptable
    sol::usertype<LuaProxy<ScriptableNamedObject>> utScriptable = lua.new_usertype<LuaProxy<ScriptableNamedObject>>("ScriptableNamedObject", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<ScriptableNamedObject>(utScriptable);
    utScriptable["onEvent"] = [](LuaProxy<ScriptableNamedObject> self, const std::string& ev, sol::object data) { self.lock()->onEvent(ev, data); };
    utScriptable["setLuaSelf"] = [](LuaProxy<ScriptableNamedObject> self, sol::table t) { self.lock()->setLuaSelf(t); };
    utScriptable["getLuaSelf"] = [](LuaProxy<ScriptableNamedObject> self) { return self.lock()->getLuaSelf(); };

    // Long
    using NamedLong = NamedInteger<int64_t>;
    sol::usertype<LuaProxy<NamedLong>> utNamedLong = lua.new_usertype<LuaProxy<NamedLong>>("NamedLong", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedLong>(utNamedLong);
    utNamedLong["value"] = [](LuaProxy<NamedLong> self) { return self.lock()->value(); };
    utNamedLong["setValue"] = [](LuaProxy<NamedLong> self, int64_t v) { self.lock()->setValue(v); };
    namedTable["createLong"] = [](const std::string& name, int64_t v, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedLong> ptr = NamedLong::create(name, v, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedLong>(ptr);
    };

    // ULong
    using NamedULong = NamedInteger<uint64_t>;
    sol::usertype<LuaProxy<NamedULong>> utNamedULong = lua.new_usertype<LuaProxy<NamedULong>>("NamedULong", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedULong>(utNamedULong);
    utNamedULong["value"] = [](LuaProxy<NamedULong> self) { return static_cast<int64_t>(self.lock()->value()); };
    utNamedULong["setValue"] = [](LuaProxy<NamedULong> self, int64_t v) { self.lock()->setValue(static_cast<uint64_t>(v)); };
    namedTable["createULong"] = [](const std::string& name, int64_t v, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedULong> ptr = NamedULong::create(name, static_cast<uint64_t>(v), extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedULong>(ptr);
    };

    // Boolean
    sol::usertype<LuaProxy<NamedBoolean>> utNamedBoolean = lua.new_usertype<LuaProxy<NamedBoolean>>("NamedBoolean", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedBoolean>(utNamedBoolean);
    utNamedBoolean["value"] = [](LuaProxy<NamedBoolean> self) { return self.lock()->booleanValue(); };
    utNamedBoolean["setValue"] = [](LuaProxy<NamedBoolean> self, bool v) { self.lock()->setValue(v); };
    namedTable["createBoolean"] = [](const std::string& name, bool v, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedBoolean> ptr = NamedBoolean::create(name, v, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedBoolean>(ptr);
    };

    // String
    sol::usertype<LuaProxy<NamedString>> utNamedString = lua.new_usertype<LuaProxy<NamedString>>("NamedString", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedString>(utNamedString);
    utNamedString["value"] = [](LuaProxy<NamedString> self) { return self.lock()->toString(); };
    utNamedString["setValue"] = [](LuaProxy<NamedString> self, const std::string& v) { self.lock()->setValue(v); };
    namedTable["createString"] = [](const std::string& name, const std::string& v, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedString> ptr = NamedString::create(name, v, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedString>(ptr);
    };

    // Quantity
    sol::usertype<LuaProxy<NamedQuantity>> utNamedQuantity = lua.new_usertype<LuaProxy<NamedQuantity>>("NamedQuantity", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedQuantity>(utNamedQuantity);
    utNamedQuantity["value"] = [](LuaProxy<NamedQuantity> self) { return self.lock()->value(); };
    utNamedQuantity["setValue"] = [](LuaProxy<NamedQuantity> self, double v) { self.lock()->setValue(v); };
    namedTable["createQuantity"] = [](const std::string& name, double v, const std::string& unit, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedQuantity> ptr = NamedQuantity::create(name, v, unit, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedQuantity>(ptr);
    };

    // Variant
    sol::usertype<LuaProxy<NamedVariant>> utNamedVariant = lua.new_usertype<LuaProxy<NamedVariant>>("NamedVariant", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedVariant>(utNamedVariant);
    utNamedVariant["get"] = [](LuaProxy<NamedVariant> self) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedObject> ptr = self.lock()->get();
        return ptr ? std::make_optional(LuaProxy<NamedObject>(ptr)) : std::nullopt;
    };
    utNamedVariant["set"] = [](LuaProxy<NamedVariant> self, sol::object obj) {
        self.lock()->set(extractNamedObject(obj));
    };
    namedTable["createVariant"] = [](const std::string& name, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedVariant> ptr = NamedVariant::create(name, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedVariant>(ptr);
    };

    // Methods
    sol::usertype<LuaProxy<NamedMethod>> utNamedMethod = lua.new_usertype<LuaProxy<NamedMethod>>("NamedMethod", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedMethod>(utNamedMethod);
    utNamedMethod["execute"] = [](LuaProxy<NamedMethod> self, sol::object args, sol::this_state L) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedMethod> method = self.lock();
        std::shared_ptr<NamedObject> res = method->execute(extractNamedObject(args));
        if (res && !res->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), res);
        return res ? std::make_optional(LuaProxy<NamedObject>(res)) : std::nullopt;
    };

    sol::usertype<LuaProxy<NamedLuaMethod>> utNamedLuaMethod = lua.new_usertype<LuaProxy<NamedLuaMethod>>("NamedLuaMethod", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedLuaMethod>(utNamedLuaMethod);
    namedTable["createLuaMethod"] = [](const std::string& name, sol::function f, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedLuaMethod> ptr = NamedLuaMethod::create(name, f, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedLuaMethod>(ptr);
    };

    // Globals
    quasarTable["track"] = [](LuaProxy<NamedObject> obj, sol::this_state L) { ObjectTracker::getInstance().trackStrong(getEngineId(L), obj.lock()); };
    quasarTable["isAlive"] = [](LuaProxy<NamedObject> obj) { return obj.isAlive(); };
    quasarTable["resolve"] = [](LuaProxy<NamedObject> root, const std::string& path) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedObject> ptr = root.lock();
        if (!ptr) return std::nullopt;
        if (path.empty()) return LuaProxy<NamedObject>(ptr);
        std::stringstream ss(path);
        std::string segment;
        std::shared_ptr<NamedObject> current = ptr;
        while (std::getline(ss, segment, '/')) {
            if (segment.empty()) continue;
            current = current->getChild(segment);
            if (!current) return std::nullopt;
        }
        return LuaProxy<NamedObject>(current);
    };
}

} // namespace quasar::scripting
