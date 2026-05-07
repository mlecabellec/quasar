/**
 * @file RegistryBindings.cpp
 * @brief Lua bindings for the Quasar named object system.
 */

#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ScriptableNamedObject.hpp"
#include "quasar/scripting/LuaService.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/QueuedObserver.hpp"
#include "quasar/scripting/LuaEngine.hpp"
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
#include "quasar/named/traversal/Transformer.hpp"
#include "quasar/named/traversal/PredefinedRules.hpp"
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
#include <thread>
#include <chrono>

namespace quasar::scripting {

using namespace quasar::named;

size_t getEngineId(sol::this_state L) {
    sol::state_view lua(L);
    sol::object engineObj = lua["__quasar_engine"];
    if (engineObj.is<std::weak_ptr<LuaEngine>>()) {
        std::shared_ptr<LuaEngine> engine = engineObj.as<std::weak_ptr<LuaEngine>>().lock();
        if (engine) return engine->getId();
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
        // [CS-0010.37] Loop hard limit.
        size_t count = 0;
        for (std::shared_ptr<NamedObject> const& c : children) {
            if (++count > 1000000) break;
            proxies.emplace_back(c);
        }
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
    ut["asFloat"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedFloatingPoint<float>>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedFloatingPoint<float>> p = std::dynamic_pointer_cast<NamedFloatingPoint<float>>(obj);
        return p ? std::make_optional(LuaProxy<NamedFloatingPoint<float>>(p)) : std::nullopt;
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
    ut["asTimestamp"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedTimestamp>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedTimestamp> p = std::dynamic_pointer_cast<NamedTimestamp>(obj);
        return p ? std::make_optional(LuaProxy<NamedTimestamp>(p)) : std::nullopt;
    };
    ut["asDuration"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedDuration>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedDuration> p = std::dynamic_pointer_cast<NamedDuration>(obj);
        return p ? std::make_optional(LuaProxy<NamedDuration>(p)) : std::nullopt;
    };
    ut["asDate"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedDate>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedDate> p = std::dynamic_pointer_cast<NamedDate>(obj);
        return p ? std::make_optional(LuaProxy<NamedDate>(p)) : std::nullopt;
    };
    ut["asQuantity"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedQuantity>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedQuantity> p = std::dynamic_pointer_cast<NamedQuantity>(obj);
        return p ? std::make_optional(LuaProxy<NamedQuantity>(p)) : std::nullopt;
    };
    ut["asVariant"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedVariant>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedVariant> p = std::dynamic_pointer_cast<NamedVariant>(obj);
        return p ? std::make_optional(LuaProxy<NamedVariant>(p)) : std::nullopt;
    };
    ut["asBuffer"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedBuffer>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedBuffer> p = std::dynamic_pointer_cast<NamedBuffer>(obj);
        return p ? std::make_optional(LuaProxy<NamedBuffer>(p)) : std::nullopt;
    };
    ut["asBitBuffer"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedBitBuffer>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedBitBuffer> p = std::dynamic_pointer_cast<NamedBitBuffer>(obj);
        return p ? std::make_optional(LuaProxy<NamedBitBuffer>(p)) : std::nullopt;
    };
    ut["asActive"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<ActiveEntity>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<ActiveEntity> p = std::dynamic_pointer_cast<ActiveEntity>(obj);
        return p ? std::make_optional(LuaProxy<ActiveEntity>(p)) : std::nullopt;
    };
    ut["asMethod"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedMethod>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedMethod> p = std::dynamic_pointer_cast<NamedMethod>(obj);
        return p ? std::make_optional(LuaProxy<NamedMethod>(p)) : std::nullopt;
    };
    ut["asService"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedService>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedService> p = std::dynamic_pointer_cast<NamedService>(obj);
        return p ? std::make_optional(LuaProxy<NamedService>(p)) : std::nullopt;
    };
    ut["asMap"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedMap<NamedObject>>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedMap<NamedObject>> p = std::dynamic_pointer_cast<NamedMap<NamedObject>>(obj);
        return p ? std::make_optional(LuaProxy<NamedMap<NamedObject>>(p)) : std::nullopt;
    };
    ut["asArray"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedArray<NamedObject>>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<NamedArray<NamedObject>> p = std::dynamic_pointer_cast<NamedArray<NamedObject>>(obj);
        return p ? std::make_optional(LuaProxy<NamedArray<NamedObject>>(p)) : std::nullopt;
    };
}

void bindNamedTypes(sol::state_view lua, std::shared_ptr<LuaService> service) {
    // Phase 5: Container Hardening
    lua.new_usertype<std::vector<uint8_t>>("VectorUInt8", sol::no_constructor);

    sol::table quasarTable = lua["quasar"].get_or_create<sol::table>();
    quasarTable["CopyPolicy"] = lua.create_table_with(
        "DUPLICATE", CopyPolicy::DUPLICATE,
        "SHARE", CopyPolicy::SHARE
    );
    quasarTable["Endianness"] = lua.create_table_with(
        "BigEndian", quasar::coretypes::Endianness::BigEndian,
        "LittleEndian", quasar::coretypes::Endianness::LittleEndian
    );

    sol::table namedTable = quasarTable["named"].get_or_create<sol::table>();
    sol::table serializationTable = namedTable["serialization"].get_or_create<sol::table>();

    serializationTable["toJson"] = [](sol::object obj) {
        return quasar::named::serialization::toJson(extractNamedObject(obj));
    };
    serializationTable["fromJson"] = [](const std::string& json, sol::this_state L) {
        std::shared_ptr<NamedObject> ptr = quasar::named::serialization::fromJson(json);
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedObject>(ptr);
    };
    serializationTable["toYaml"] = [](sol::object obj) {
        return quasar::named::serialization::toYaml(extractNamedObject(obj));
    };
    serializationTable["fromYaml"] = [](const std::string& yaml, sol::this_state L) {
        std::shared_ptr<NamedObject> ptr = quasar::named::serialization::fromYaml(yaml);
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedObject>(ptr);
    };
    serializationTable["toXml"] = [](sol::object obj) {
        return quasar::named::serialization::toXml(extractNamedObject(obj));
    };
    serializationTable["fromXml"] = [](const std::string& xml, sol::this_state L) {
        std::shared_ptr<NamedObject> ptr = quasar::named::serialization::fromXml(xml);
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedObject>(ptr);
    };
    serializationTable["toBinary"] = [](sol::object obj) {
        return sol::as_table(quasar::named::serialization::toBinary(extractNamedObject(obj)));
    };
    serializationTable["fromBinary"] = [](sol::table data, sol::this_state L) {
        std::vector<uint8_t> v;
        v.reserve(data.size());
        // [CS-0010.37] Loop hard limit.
        size_t count = 0;
        for (size_t i = 1; i <= data.size(); ++i) {
            if (++count > 100000000) break; // 100MB limit
            v.push_back(data.get<uint8_t>(i));
        }
        std::shared_ptr<NamedObject> ptr = quasar::named::serialization::fromBinary(v);
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedObject>(ptr);
    };

    lua.new_usertype<ILuaProxy>("ILuaProxy", sol::no_constructor, "isAlive", &ILuaProxy::isAlive);
    lua.new_usertype<IObserver>("IObserver", sol::no_constructor);
    lua.new_usertype<quasar::named::traversal::TransformationRule>("TransformationRule", sol::no_constructor);
    
    // NamedObject
    sol::usertype<LuaProxy<NamedObject>> utNamedObject = lua.new_usertype<LuaProxy<NamedObject>>("NamedObject", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedObject>(utNamedObject);
    namedTable["createObject"] = [](const std::string& name, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedObject> ptr = NamedObject::create(name, extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedObject>(ptr);
    };

    // Scriptable
    sol::usertype<LuaProxy<ScriptableNamedObject>> utScriptable = lua.new_usertype<LuaProxy<ScriptableNamedObject>>("ScriptableNamedObject", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<ScriptableNamedObject>(utScriptable);
    utScriptable["onEvent"] = [](LuaProxy<ScriptableNamedObject> self, const std::string& ev, sol::object data) { self.lock()->onEvent(ev, data); };
    utScriptable["setLuaSelf"] = [](LuaProxy<ScriptableNamedObject> self, sol::table t) { self.lock()->setLuaSelf(t); };
    utScriptable["getLuaSelf"] = [](LuaProxy<ScriptableNamedObject> self) { return self.lock()->getLuaSelf(); };
    namedTable["createScriptable"] = [](const std::string& name, sol::object parent, sol::this_state L) {
        std::shared_ptr<ScriptableNamedObject> ptr = ScriptableNamedObject::create(name, extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<ScriptableNamedObject>(ptr);
    };

    // Long
    using NamedLong = NamedInteger<int64_t>;
    sol::usertype<LuaProxy<NamedLong>> utNamedLong = lua.new_usertype<LuaProxy<NamedLong>>("NamedLong", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedLong>(utNamedLong);
    utNamedLong["value"] = [](LuaProxy<NamedLong> self) { return self.lock()->value(); };
    utNamedLong["setValue"] = [](LuaProxy<NamedLong> self, int64_t v) { self.lock()->setValue(v); };
    namedTable["createLong"] = [](const std::string& name, int64_t v, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedLong> ptr = NamedLong::create(name, v, extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
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
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedULong>(ptr);
    };

    // Double
    using NamedDouble = NamedFloatingPoint<double>;
    sol::usertype<LuaProxy<NamedDouble>> utNamedDouble = lua.new_usertype<LuaProxy<NamedDouble>>("NamedDouble", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedDouble>(utNamedDouble);
    utNamedDouble["value"] = [](LuaProxy<NamedDouble> self) { return self.lock()->value(); };
    utNamedDouble["setValue"] = [](LuaProxy<NamedDouble> self, double v) { self.lock()->setValue(v); };
    namedTable["createDouble"] = [](const std::string& name, double v, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedDouble> ptr = NamedDouble::create(name, v, extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedDouble>(ptr);
    };

    // Float
    using NamedFloat = NamedFloatingPoint<float>;
    sol::usertype<LuaProxy<NamedFloat>> utNamedFloat = lua.new_usertype<LuaProxy<NamedFloat>>("NamedFloat", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedFloat>(utNamedFloat);
    utNamedFloat["value"] = [](LuaProxy<NamedFloat> self) { return self.lock()->value(); };
    utNamedFloat["setValue"] = [](LuaProxy<NamedFloat> self, float v) { self.lock()->setValue(v); };
    namedTable["createFloat"] = [](const std::string& name, float v, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedFloat> ptr = NamedFloat::create(name, v, extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedFloat>(ptr);
    };

    // Boolean
    sol::usertype<LuaProxy<NamedBoolean>> utNamedBoolean = lua.new_usertype<LuaProxy<NamedBoolean>>("NamedBoolean", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedBoolean>(utNamedBoolean);
    utNamedBoolean["value"] = [](LuaProxy<NamedBoolean> self) { return self.lock()->booleanValue(); };
    utNamedBoolean["setValue"] = [](LuaProxy<NamedBoolean> self, bool v) { self.lock()->setValue(v); };
    namedTable["createBoolean"] = [](const std::string& name, bool v, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedBoolean> ptr = NamedBoolean::create(name, v, extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedBoolean>(ptr);
    };

    // String
    sol::usertype<LuaProxy<NamedString>> utNamedString = lua.new_usertype<LuaProxy<NamedString>>("NamedString", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedString>(utNamedString);
    utNamedString["value"] = [](LuaProxy<NamedString> self) { return self.lock()->toString(); };
    utNamedString["setValue"] = [](LuaProxy<NamedString> self, const std::string& v) { self.lock()->setValue(v); };
    namedTable["createString"] = [](const std::string& name, const std::string& v, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedString> ptr = NamedString::create(name, v, extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedString>(ptr);
    };

    // Timestamp
    sol::usertype<LuaProxy<NamedTimestamp>> utNamedTimestamp = lua.new_usertype<LuaProxy<NamedTimestamp>>("NamedTimestamp", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedTimestamp>(utNamedTimestamp);
    utNamedTimestamp["value"] = [](LuaProxy<NamedTimestamp> self) { return self.lock()->value(); };
    namedTable["createTimestamp"] = [](const std::string& name, sol::optional<int64_t> v, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedTimestamp> ptr = NamedTimestamp::create(name, v.value_or(0), extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedTimestamp>(ptr);
    };

    // Duration
    sol::usertype<LuaProxy<NamedDuration>> utNamedDuration = lua.new_usertype<LuaProxy<NamedDuration>>("NamedDuration", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedDuration>(utNamedDuration);
    utNamedDuration["value"] = [](LuaProxy<NamedDuration> self) { return self.lock()->value(); };
    namedTable["createDuration"] = [](const std::string& name, sol::optional<int64_t> v, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedDuration> ptr = NamedDuration::create(name, v.value_or(0), extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedDuration>(ptr);
    };

    // Date
    sol::usertype<LuaProxy<NamedDate>> utNamedDate = lua.new_usertype<LuaProxy<NamedDate>>("NamedDate", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedDate>(utNamedDate);
    utNamedDate["value"] = [](LuaProxy<NamedDate> self) { return self.lock()->value(); };
    namedTable["createDate"] = [](const std::string& name, sol::optional<int64_t> v, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedDate> ptr = NamedDate::create(name, v.value_or(0), extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedDate>(ptr);
    };

    // Quantity
    sol::usertype<LuaProxy<NamedQuantity>> utNamedQuantity = lua.new_usertype<LuaProxy<NamedQuantity>>("NamedQuantity", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedQuantity>(utNamedQuantity);
    utNamedQuantity["value"] = [](LuaProxy<NamedQuantity> self) { return self.lock()->value(); };
    utNamedQuantity["setValue"] = [](LuaProxy<NamedQuantity> self, double v) { self.lock()->setValue(v); };
    namedTable["createQuantity"] = [](const std::string& name, double v, const std::string& unit, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedQuantity> ptr = NamedQuantity::create(name, v, unit, extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedQuantity>(ptr);
    };

    // Variant
    sol::usertype<LuaProxy<NamedVariant>> utNamedVariant = lua.new_usertype<LuaProxy<NamedVariant>>("NamedVariant", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedVariant>(utNamedVariant);
    utNamedVariant["getValue"] = [](LuaProxy<NamedVariant> self, sol::this_state L) -> sol::object {
        coretypes::Variant v = self.lock()->getVariant();
        if (v.getVariantType() == coretypes::VariantType::Boolean) return sol::make_object(L, v.getAs<bool>());
        if (v.getVariantType() == coretypes::VariantType::Integer) return sol::make_object(L, v.getAs<int64_t>());
        if (v.getVariantType() == coretypes::VariantType::Double) return sol::make_object(L, v.getAs<double>());
        if (v.getVariantType() == coretypes::VariantType::String) return sol::make_object(L, v.getAs<std::string>());
        if (v.getVariantType() == coretypes::VariantType::Buffer) return sol::make_object(L, sol::as_table(v.getAs<std::vector<uint8_t>>()));
        return sol::make_object(L, sol::nil);
    };
    utNamedVariant["setValue"] = [](LuaProxy<NamedVariant> self, sol::object obj) {
        if (obj.is<bool>()) self.lock()->setVariant(coretypes::Variant(obj.as<bool>()));
        else if (obj.is<int64_t>()) self.lock()->setVariant(coretypes::Variant(obj.as<int64_t>()));
        else if (obj.is<double>()) self.lock()->setVariant(coretypes::Variant(obj.as<double>()));
        else if (obj.is<std::string>()) self.lock()->setVariant(coretypes::Variant(obj.as<std::string>()));
        else if (obj.is<sol::table>()) {
            std::vector<uint8_t> v;
            sol::table t = obj.as<sol::table>();
            v.reserve(t.size());
            // [CS-0010.37] Loop hard limit.
            size_t count = 0;
            for (size_t i = 1; i <= t.size(); ++i) {
                if (++count > 1000000) break;
                v.push_back(t.get<uint8_t>(i));
            }
            self.lock()->setVariant(coretypes::Variant(v));
        }
    };
    namedTable["createVariant"] = [](const std::string& name, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedVariant> ptr = NamedVariant::create(name, coretypes::Variant(), extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedVariant>(ptr);
    };

    // Buffer
    sol::usertype<LuaProxy<NamedBuffer>> utNamedBuffer = lua.new_usertype<LuaProxy<NamedBuffer>>("NamedBuffer", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedBuffer>(utNamedBuffer);
    utNamedBuffer["getSize"] = [](LuaProxy<NamedBuffer> self) { return self.lock()->size(); };
    utNamedBuffer["read"] = [](LuaProxy<NamedBuffer> self, size_t off, size_t sz) {
        std::shared_ptr<NamedBuffer> b = self.lock();
        if (off >= b->size()) return sol::as_table(std::vector<uint8_t>());
        return sol::as_table(b->slice(off, std::min(sz, b->size() - off)).toVector());
    };
    utNamedBuffer["write"] = [](LuaProxy<NamedBuffer> self, size_t off, sol::table data) {
        std::shared_ptr<NamedBuffer> b = self.lock();
        for (size_t i = 1; i <= data.size() && (off + i - 1) < b->size(); ++i) {
            b->set(off + i - 1, data.get<uint8_t>(i));
        }
    };
    namedTable["createBuffer"] = [](const std::string& name, size_t sz, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedBuffer> ptr = NamedBuffer::create(name, sz, extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedBuffer>(ptr);
    };

    // BitBuffer
    sol::usertype<LuaProxy<NamedBitBuffer>> utNamedBitBuffer = lua.new_usertype<LuaProxy<NamedBitBuffer>>("NamedBitBuffer", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedBitBuffer>(utNamedBitBuffer);
    utNamedBitBuffer["getBitCount"] = [](LuaProxy<NamedBitBuffer> self) { return self.lock()->bitSize(); };
    utNamedBitBuffer["getBit"] = [](LuaProxy<NamedBitBuffer> self, size_t i) { return self.lock()->getBit(i); };
    utNamedBitBuffer["setBit"] = [](LuaProxy<NamedBitBuffer> self, size_t i, bool v) { self.lock()->setBit(i, v); };
    namedTable["createBitBuffer"] = [](const std::string& name, size_t bc, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedBitBuffer> ptr = NamedBitBuffer::create(name, bc, extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedBitBuffer>(ptr);
    };

    // Array
    using NamedObjectArray = NamedArray<NamedObject>;
    sol::usertype<LuaProxy<NamedObjectArray>> utNamedArray = lua.new_usertype<LuaProxy<NamedObjectArray>>("NamedArray", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedObjectArray>(utNamedArray);
    utNamedArray["size"] = [](LuaProxy<NamedObjectArray> self) { return self.lock()->size(); };
    utNamedArray["get"] = [](LuaProxy<NamedObjectArray> self, size_t i) -> std::optional<LuaProxy<NamedObject>> {
        try { return LuaProxy<NamedObject>(self.lock()->get(i - 1)); } catch(...) { return std::nullopt; }
    };
    namedTable["createArray"] = [](const std::string& name, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedObjectArray> ptr = NamedObjectArray::create(name, extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedObjectArray>(ptr);
    };

    // Map
    using NamedObjectMap = NamedMap<NamedObject>;
    sol::usertype<LuaProxy<NamedObjectMap>> utNamedObjectMap = lua.new_usertype<LuaProxy<NamedObjectMap>>("NamedMap", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedObjectMap>(utNamedObjectMap);
    utNamedObjectMap["size"] = [](LuaProxy<NamedObjectMap> self) { return self.lock()->size(); };
    utNamedObjectMap["get"] = [](LuaProxy<NamedObjectMap> self, const std::string& k) -> std::optional<LuaProxy<NamedObject>> {
        try { return LuaProxy<NamedObject>(self.lock()->get(k)); } catch(...) { return std::nullopt; }
    };
    namedTable["createMap"] = [](const std::string& name, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedObjectMap> ptr = NamedObjectMap::create(name, extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedObjectMap>(ptr);
    };

    // ActiveEntity
    sol::usertype<LuaProxy<ActiveEntity>> utActiveEntity = lua.new_usertype<LuaProxy<ActiveEntity>>("ActiveEntity", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<ActiveEntity>(utActiveEntity);
    utActiveEntity["subscribe"] = [](LuaProxy<ActiveEntity> self, std::shared_ptr<IObserver> obs) { self.lock()->subscribe(obs); };
    utActiveEntity["unsubscribe"] = [](LuaProxy<ActiveEntity> self, std::shared_ptr<IObserver> obs) { self.lock()->unsubscribe(obs); };
    utActiveEntity["getState"] = [](LuaProxy<ActiveEntity> self) { return static_cast<int>(self.lock()->getState()); };

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
    utNamedLuaMethod["execute"] = [](LuaProxy<NamedLuaMethod> self, sol::object args, sol::this_state L) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedLuaMethod> method = self.lock();
        std::shared_ptr<NamedObject> res = method->execute(extractNamedObject(args));
        if (res && !res->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), res);
        return res ? std::make_optional(LuaProxy<NamedObject>(res)) : std::nullopt;
    };
    namedTable["createLuaMethod"] = [](const std::string& name, sol::function f, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedLuaMethod> ptr = NamedLuaMethod::create(name, f, extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedLuaMethod>(ptr);
    };

    // NamedService
    sol::usertype<LuaProxy<NamedService>> utNamedService = lua.new_usertype<LuaProxy<NamedService>>("NamedService", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedService>(utNamedService);
    utNamedService["start"] = [](LuaProxy<NamedService> self) { self.lock()->start(); };
    utNamedService["stop"] = [](LuaProxy<NamedService> self, sol::this_state L) { 
        std::shared_ptr<NamedService> svc = self.lock();
        sol::state_view lua(L);
        sol::object engineObj = lua["__quasar_engine"];
        if (engineObj.is<std::weak_ptr<LuaEngine>>()) {
            std::shared_ptr<LuaEngine> engine = engineObj.as<std::weak_ptr<LuaEngine>>().lock();
            if (engine) {
                engine->getMutex().unlock();
                svc->stop();
                engine->getMutex().lock();
                return;
            }
        }
        svc->stop();
    };
    utNamedService["isRunning"] = [](LuaProxy<NamedService> self) { return self.lock()->isRunning(); };
    utNamedService["setCycleTime"] = [](LuaProxy<NamedService> self, int ms) { self.lock()->setCycleTime(std::chrono::milliseconds(ms)); };
    namedTable["createService"] = [](const std::string& name, sol::object parent, sol::this_state L) {
        std::shared_ptr<NamedService> ptr = NamedService::create(name, extractNamedObject(parent));
        if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedService>(ptr);
    };

    // Traversal
    sol::table traversalTable = namedTable["traversal"].get_or_create<sol::table>();

    traversalTable["TransformContext"] = lua.new_usertype<quasar::named::traversal::TransformContext>("TransformContext", sol::no_constructor,
        "getNode", [](const quasar::named::traversal::TransformContext& ctx) { return LuaProxy<NamedObject>(ctx.getNode()); },
        "getDepth", &quasar::named::traversal::TransformContext::getDepth,
        "getPath", &quasar::named::traversal::TransformContext::getPath
    );

    traversalTable["Transformer"] = lua.new_usertype<quasar::named::traversal::Transformer>("Transformer",
        sol::constructors<quasar::named::traversal::Transformer()>(),
        "addRule", [](sol::this_state L, quasar::named::traversal::Transformer& self, sol::object arg1, sol::optional<sol::function> arg2, sol::optional<int> arg3) {
            if (arg2.has_value()) {
                // Case: pred, gen, priority
                sol::function pred = arg1.as<sol::function>();
                sol::function gen = arg2.value();
                int priority = arg3.value_or(0);
                size_t id = getEngineId(L);
                
                const std::function<bool(const quasar::named::traversal::TransformContext&)> p = [pred](const quasar::named::traversal::TransformContext& ctx) -> bool {
                    sol::protected_function_result res = pred(ctx);
                    return res.valid() && res.get<bool>();
                };
                const std::function<std::vector<std::shared_ptr<NamedObject>>(const quasar::named::traversal::TransformContext&, quasar::named::traversal::Transformer&)> g = [gen, id](const quasar::named::traversal::TransformContext& ctx, quasar::named::traversal::Transformer& t) -> std::vector<std::shared_ptr<NamedObject>> {
                    sol::protected_function_result res = gen(ctx, t);
                    if (!res.valid()) return {};
                    std::vector<std::shared_ptr<NamedObject>> out;
                    if (res.get_type() == sol::type::table) {
                        sol::table tbl = res.get<sol::table>();
                        // [CS-0010.37] Loop hard limit.
                        size_t count = 0;
                        for (size_t i = 1; i <= tbl.size(); ++i) {
                            if (++count > 1000000) break;
                            std::shared_ptr<NamedObject> ptr = extractNamedObject(tbl[i]);
                            if (ptr) {
                                if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(id, ptr);
                                out.push_back(ptr);
                            }
                        }
                    } else {
                        std::shared_ptr<NamedObject> ptr = extractNamedObject(res.get<sol::object>());
                        if (ptr != nullptr) {
                            if (ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(id, ptr);
                            out.push_back(ptr);
                        }
                    }
                    return out;
                };
                self.addRule(quasar::named::traversal::TransformationRule(p, g, priority));
            } else {
                // Case: TransformationRule object
                self.addRule(arg1.as<quasar::named::traversal::TransformationRule>());
            }
        },
        "transform", [](quasar::named::traversal::Transformer& self, sol::object root, sol::this_state L) {
            std::vector<std::shared_ptr<NamedObject>> res = self.transform(extractNamedObject(root));
            std::vector<LuaProxy<NamedObject>> out;
            size_t id = getEngineId(L);
            // [CS-0010.37] Loop hard limit.
            size_t count = 0;
            for (const std::shared_ptr<NamedObject>& ptr : res) {
                if (++count > 1000000) break;
                if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(id, ptr);
                out.emplace_back(ptr);
            }
            return out;
        },
        "transformInPlace", [](quasar::named::traversal::Transformer& self, sol::object root) {
            self.transformInPlace(extractNamedObject(root));
        },
        "transformSubtree", [](quasar::named::traversal::Transformer& self, sol::object node, int depth, const std::string& path, sol::this_state L) {
            std::vector<std::shared_ptr<NamedObject>> res = self.transformSubtree(extractNamedObject(node), depth, path);
            std::vector<LuaProxy<NamedObject>> out;
            size_t id = getEngineId(L);
            // [CS-0010.37] Loop hard limit.
            size_t count = 0;
            for (const std::shared_ptr<NamedObject>& ptr : res) {
                if (++count > 1000000) break;
                if (ptr != nullptr && ptr->getParent() == nullptr) ObjectTracker::getInstance().trackStrong(id, ptr);
                out.emplace_back(ptr);
            }
            return out;
        }
    );

    // Predefined Rules
    sol::table predefinedTable = traversalTable["rules"].get_or_create<sol::table>();
    predefinedTable["sliceBuffer"] = [](const std::string& name, sol::table slices, CopyPolicy policy, int priority) {
        std::vector<quasar::named::traversal::SliceDefinition> v;
        // [CS-0010.37] Loop hard limit.
        size_t count = 0;
        for (size_t i = 1; i <= slices.size(); ++i) {
            if (++count > 10000) break;
            sol::table s = slices[i];
            v.push_back({s.get<std::string>("name"), s.get<size_t>("offset"), s.get<size_t>("length")});
        }
        return quasar::named::traversal::PredefinedRules::sliceBuffer(name, v, policy, priority);
    };

    predefinedTable["castToStructure"] = [](const std::string& name, sol::table mappings, int priority) {
        std::vector<quasar::named::traversal::FieldMapping> v;
        // [CS-0010.37] Loop hard limit.
        size_t count = 0;
        for (size_t i = 1; i <= mappings.size(); ++i) {
            if (++count > 10000) break;
            sol::table m = mappings[i];
            v.push_back({m.get<std::string>("name"), m.get<std::string>("type"), m.get<size_t>("offset"), m.get_or("endian", quasar::coretypes::Endianness::BigEndian)});
        }
        return quasar::named::traversal::PredefinedRules::castToStructure(name, v, priority);
    };

    // Globals
    quasarTable["track"] = [](LuaProxy<NamedObject> obj, sol::this_state L) { ObjectTracker::getInstance().trackStrong(getEngineId(L), obj.lock()); };
    quasarTable["isAlive"] = [](LuaProxy<NamedObject> obj) { return obj.isAlive(); };
    quasarTable["resolve"] = [](LuaProxy<NamedObject> root, const std::string& path) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedObject> ptr = root.lock();
        if (ptr == nullptr) return std::nullopt;
        if (path.empty()) return LuaProxy<NamedObject>(ptr);
        std::stringstream ss(path);
        std::string segment;
        std::shared_ptr<NamedObject> current = ptr;
        while (std::getline(ss, segment, '/')) {
            if (segment.empty()) continue;
            current = current->getChild(segment);
            if (current == nullptr) return std::nullopt;
        }
        return LuaProxy<NamedObject>(current);
    };
    quasarTable["createObserver"] = [weakService = std::weak_ptr<LuaService>(service)](sol::function cb, sol::optional<size_t> wm) {
        return std::make_shared<QueuedObserver>(weakService.lock(), cb, wm.value_or(1000));
    };
    quasarTable["sleep"] = [](int ms, sol::this_state L) {
        sol::state_view lua(L);
        sol::object engineObj = lua["__quasar_engine"];
        if (engineObj.is<std::weak_ptr<LuaEngine>>()) {
            std::shared_ptr<LuaEngine> engine = engineObj.as<std::weak_ptr<LuaEngine>>().lock();
            if (engine != nullptr) {
                engine->getMutex().unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
                engine->getMutex().lock();
                return;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    };
}

} // namespace quasar::scripting
