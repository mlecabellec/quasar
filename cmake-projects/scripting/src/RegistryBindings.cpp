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
#include <vector>
#include <list>
#include <memory>

namespace quasar::scripting {

using namespace quasar::named;

std::shared_ptr<NamedObject> extractNamedObject(sol::object obj) {
    if (!obj.valid() || obj.is<sol::nil_t>()) return nullptr;
    
    // Attempt to extract as a generic ILuaProxy
    if (obj.is<ILuaProxy>()) {
        return obj.as<ILuaProxy&>().lockAsNamedObject();
    }
    
    // Fallback for explicit proxy types
    if (obj.is<LuaProxy<NamedObject>>()) return obj.as<LuaProxy<NamedObject>>().lockAsNamedObject();
    if (obj.is<LuaProxy<NamedInteger<int64_t>>>()) return obj.as<LuaProxy<NamedInteger<int64_t>>>().lockAsNamedObject();
    if (obj.is<LuaProxy<NamedInteger<uint64_t>>>()) return obj.as<LuaProxy<NamedInteger<uint64_t>>>().lockAsNamedObject();

    return nullptr;
}

/** @brief Helper to bind common NamedObject methods to a Proxy usertype. */
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
        std::transform(children.begin(), children.end(), std::back_inserter(proxies),
            [](std::shared_ptr<NamedObject>& c) { return LuaProxy<NamedObject>(c); });
        return proxies;
    };
    
    // Type Casting Helpers
    ut["asLong"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedInteger<int64_t>>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedInteger") {
            return std::make_optional(LuaProxy<NamedInteger<int64_t>>(std::static_pointer_cast<NamedInteger<int64_t>>(obj)));
        }
        return std::nullopt;
    };
    ut["asULong"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedInteger<uint64_t>>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedULong") {
            return std::make_optional(LuaProxy<NamedInteger<uint64_t>>(std::static_pointer_cast<NamedInteger<uint64_t>>(obj)));
        }
        return std::nullopt;
    };
    ut["asDouble"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedFloatingPoint<double>>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedDouble" || obj->getType() == "NamedFloatingPoint") {
            return std::make_optional(LuaProxy<NamedFloatingPoint<double>>(std::static_pointer_cast<NamedFloatingPoint<double>>(obj)));
        }
        return std::nullopt;
    };
    ut["asBoolean"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedBoolean>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedBoolean") {
            return std::make_optional(LuaProxy<NamedBoolean>(std::static_pointer_cast<NamedBoolean>(obj)));
        }
        return std::nullopt;
    };
    ut["asString"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedString>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedString") {
            return std::make_optional(LuaProxy<NamedString>(std::static_pointer_cast<NamedString>(obj)));
        }
        return std::nullopt;
    };
    ut["asTimestamp"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedTimestamp>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedTimestamp") {
            return std::make_optional(LuaProxy<NamedTimestamp>(std::static_pointer_cast<NamedTimestamp>(obj)));
        }
        return std::nullopt;
    };
    ut["asDuration"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedDuration>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedDuration") {
            return std::make_optional(LuaProxy<NamedDuration>(std::static_pointer_cast<NamedDuration>(obj)));
        }
        return std::nullopt;
    };
    ut["asDate"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedDate>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedDate") {
            return std::make_optional(LuaProxy<NamedDate>(std::static_pointer_cast<NamedDate>(obj)));
        }
        return std::nullopt;
    };
    ut["asQuantity"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedQuantity>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedQuantity") {
            return std::make_optional(LuaProxy<NamedQuantity>(std::static_pointer_cast<NamedQuantity>(obj)));
        }
        return std::nullopt;
    };
    ut["asVariant"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedVariant>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedVariant") {
            return std::make_optional(LuaProxy<NamedVariant>(std::static_pointer_cast<NamedVariant>(obj)));
        }
        return std::nullopt;
    };
    ut["asBuffer"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedBuffer>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedBuffer") {
            return std::make_optional(LuaProxy<NamedBuffer>(std::static_pointer_cast<NamedBuffer>(obj)));
        }
        return std::nullopt;
    };
    ut["asBitBuffer"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedBitBuffer>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedBitBuffer") {
            return std::make_optional(LuaProxy<NamedBitBuffer>(std::static_pointer_cast<NamedBitBuffer>(obj)));
        }
        return std::nullopt;
    };
    ut["asActive"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<ActiveEntity>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        std::shared_ptr<ActiveEntity> ptr = std::dynamic_pointer_cast<ActiveEntity>(obj);
        return ptr ? std::make_optional(LuaProxy<ActiveEntity>(ptr)) : std::nullopt;
    };
    ut["asMethod"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedMethod>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedMethod" || obj->getType() == "NamedLuaMethod") {
            return std::make_optional(LuaProxy<NamedMethod>(std::static_pointer_cast<NamedMethod>(obj)));
        }
        return std::nullopt;
    };
    ut["asService"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedService>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedService" || obj->getType() == "DataLoggerService" || obj->getType() == "OpcUaServerService" || obj->getType() == "OpcUaClientService") {
            return std::make_optional(LuaProxy<NamedService>(std::static_pointer_cast<NamedService>(obj)));
        }
        return std::nullopt;
    };
    ut["asMap"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedMap<NamedObject>>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedMap") {
            return std::make_optional(LuaProxy<NamedMap<NamedObject>>(std::static_pointer_cast<NamedMap<NamedObject>>(obj)));
        }
        return std::nullopt;
    };
    ut["asArray"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedArray<NamedObject>>> {
        std::shared_ptr<NamedObject> obj = self.lock();
        if (obj->getType() == "NamedArray") {
            return std::make_optional(LuaProxy<NamedArray<NamedObject>>(std::static_pointer_cast<NamedArray<NamedObject>>(obj)));
        }
        return std::nullopt;
    };
}

void bindNamedTypes(sol::state_view lua, std::shared_ptr<LuaService> service) {
    sol::table quasarTable = lua["quasar"].get_or_create<sol::table>();
    sol::table namedTable = quasarTable["named"].get_or_create<sol::table>();
    sol::table serializationTable = namedTable["serialization"].get_or_create<sol::table>();

    serializationTable["toJson"] = [](sol::object obj) {
        return quasar::named::serialization::toJson(extractNamedObject(obj));
    };
    serializationTable["fromJson"] = [](const std::string& json) {
        std::shared_ptr<NamedObject> ptr = quasar::named::serialization::fromJson(json);
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedObject>(ptr);
    };
    serializationTable["toYaml"] = [](sol::object obj) {
        return quasar::named::serialization::toYaml(extractNamedObject(obj));
    };
    serializationTable["fromYaml"] = [](const std::string& yaml) {
        std::shared_ptr<NamedObject> ptr = quasar::named::serialization::fromYaml(yaml);
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedObject>(ptr);
    };
    serializationTable["toXml"] = [](sol::object obj) {
        return quasar::named::serialization::toXml(extractNamedObject(obj));
    };
    serializationTable["fromXml"] = [](const std::string& xml) {
        std::shared_ptr<NamedObject> ptr = quasar::named::serialization::fromXml(xml);
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedObject>(ptr);
    };
    serializationTable["toBinary"] = [](sol::object obj) {
        return quasar::named::serialization::toBinary(extractNamedObject(obj));
    };
    serializationTable["fromBinary"] = [](sol::table data) {
        std::vector<uint8_t> v;
        v.reserve(data.size());
        for (size_t i = 1; i <= data.size(); ++i) v.push_back(data.get<uint8_t>(i));
        std::shared_ptr<NamedObject> ptr = quasar::named::serialization::fromBinary(v);
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedObject>(ptr);
    };

    lua.new_usertype<ILuaProxy>("ILuaProxy", sol::no_constructor,
        "isAlive", &ILuaProxy::isAlive
    );

    lua.new_usertype<IObserver>("IObserver", sol::no_constructor);

    // NamedObject
    sol::usertype<LuaProxy<NamedObject>> utNamedObject = lua.new_usertype<LuaProxy<NamedObject>>("NamedObject", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedObject>(utNamedObject);
    namedTable["createObject"] = [](const std::string& name, sol::object parent) {
        std::shared_ptr<NamedObject> ptr = NamedObject::create(name, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedObject>(ptr);
    };

    // ScriptableNamedObject
    sol::usertype<LuaProxy<ScriptableNamedObject>> utScriptable = lua.new_usertype<LuaProxy<ScriptableNamedObject>>("ScriptableNamedObject", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<ScriptableNamedObject>(utScriptable);
    utScriptable["onEvent"] = [](LuaProxy<ScriptableNamedObject> self, const std::string& ev, sol::object data) { self.lock()->onEvent(ev, data); };
    utScriptable["setLuaSelf"] = [](LuaProxy<ScriptableNamedObject> self, sol::table t) { self.lock()->setLuaSelf(t); };
    utScriptable["getLuaSelf"] = [](LuaProxy<ScriptableNamedObject> self) { return self.lock()->getLuaSelf(); };
    namedTable["createScriptable"] = [](const std::string& name, sol::object parent) {
        std::shared_ptr<ScriptableNamedObject> ptr = ScriptableNamedObject::create(name, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<ScriptableNamedObject>(ptr);
    };

    // NamedLong
    using NamedLong = NamedInteger<int64_t>;
    sol::usertype<LuaProxy<NamedLong>> utNamedLong = lua.new_usertype<LuaProxy<NamedLong>>("NamedLong", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedLong>(utNamedLong);
    utNamedLong["value"] = [](LuaProxy<NamedLong> self) { return self.lock()->value(); };
    utNamedLong["setValue"] = [](LuaProxy<NamedLong> self, int64_t v) { self.lock()->setValue(v); };
    namedTable["createLong"] = [](const std::string& name, int64_t v, sol::object parent) {
        std::shared_ptr<NamedLong> ptr = NamedLong::create(name, v, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedLong>(ptr);
    };

    // NamedULong
    using NamedULong = NamedInteger<uint64_t>;
    sol::usertype<LuaProxy<NamedULong>> utNamedULong = lua.new_usertype<LuaProxy<NamedULong>>("NamedULong", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedULong>(utNamedULong);
    utNamedULong["value"] = [](LuaProxy<NamedULong> self) { return self.lock()->value(); };
    utNamedULong["setValue"] = [](LuaProxy<NamedULong> self, uint64_t v) { self.lock()->setValue(v); };
    namedTable["createULong"] = [](const std::string& name, uint64_t v, sol::object parent) {
        std::shared_ptr<NamedULong> ptr = NamedULong::create(name, v, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedULong>(ptr);
    };

    // NamedDouble
    using NamedDouble = NamedFloatingPoint<double>;
    sol::usertype<LuaProxy<NamedDouble>> utNamedDouble = lua.new_usertype<LuaProxy<NamedDouble>>("NamedDouble", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedDouble>(utNamedDouble);
    utNamedDouble["value"] = [](LuaProxy<NamedDouble> self) { return self.lock()->value(); };
    utNamedDouble["setValue"] = [](LuaProxy<NamedDouble> self, double v) { self.lock()->setValue(v); };
    namedTable["createDouble"] = [](const std::string& name, double v, sol::object parent) {
        std::shared_ptr<NamedDouble> ptr = NamedDouble::create(name, v, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedDouble>(ptr);
    };

    // NamedBoolean
    sol::usertype<LuaProxy<NamedBoolean>> utNamedBoolean = lua.new_usertype<LuaProxy<NamedBoolean>>("NamedBoolean", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedBoolean>(utNamedBoolean);
    utNamedBoolean["value"] = [](LuaProxy<NamedBoolean> self) { return self.lock()->booleanValue(); };
    utNamedBoolean["setValue"] = [](LuaProxy<NamedBoolean> self, bool v) { self.lock()->setValue(v); };
    namedTable["createBoolean"] = [](const std::string& name, bool v, sol::object parent) {
        std::shared_ptr<NamedBoolean> ptr = NamedBoolean::create(name, v, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedBoolean>(ptr);
    };

    // NamedString
    sol::usertype<LuaProxy<NamedString>> utNamedString = lua.new_usertype<LuaProxy<NamedString>>("NamedString", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedString>(utNamedString);
    utNamedString["value"] = [](LuaProxy<NamedString> self) { return self.lock()->toString(); };
    utNamedString["setValue"] = [](LuaProxy<NamedString> self, const std::string& v) { self.lock()->setValue(v); };
    namedTable["createString"] = [](const std::string& name, const std::string& v, sol::object parent) {
        std::shared_ptr<NamedString> ptr = NamedString::create(name, v, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedString>(ptr);
    };

    // NamedTimestamp
    sol::usertype<LuaProxy<NamedTimestamp>> utNamedTimestamp = lua.new_usertype<LuaProxy<NamedTimestamp>>("NamedTimestamp", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedTimestamp>(utNamedTimestamp);
    utNamedTimestamp["value"] = [](LuaProxy<NamedTimestamp> self) { return self.lock()->value(); };
    namedTable["createTimestamp"] = [](const std::string& name, sol::optional<int64_t> v, sol::object parent) {
        std::shared_ptr<NamedTimestamp> ptr = NamedTimestamp::create(name, v.value_or(0), extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedTimestamp>(ptr);
    };

    // NamedDuration
    sol::usertype<LuaProxy<NamedDuration>> utNamedDuration = lua.new_usertype<LuaProxy<NamedDuration>>("NamedDuration", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedDuration>(utNamedDuration);
    utNamedDuration["value"] = [](LuaProxy<NamedDuration> self) { return self.lock()->value(); };
    namedTable["createDuration"] = [](const std::string& name, sol::optional<int64_t> v, sol::object parent) {
        std::shared_ptr<NamedDuration> ptr = NamedDuration::create(name, v.value_or(0), extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedDuration>(ptr);
    };

    // NamedDate
    sol::usertype<LuaProxy<NamedDate>> utNamedDate = lua.new_usertype<LuaProxy<NamedDate>>("NamedDate", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedDate>(utNamedDate);
    utNamedDate["value"] = [](LuaProxy<NamedDate> self) { return self.lock()->value(); };
    namedTable["createDate"] = [](const std::string& name, sol::optional<int64_t> v, sol::object parent) {
        std::shared_ptr<NamedDate> ptr = NamedDate::create(name, v.value_or(0), extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedDate>(ptr);
    };

    // NamedQuantity
    sol::usertype<LuaProxy<NamedQuantity>> utNamedQuantity = lua.new_usertype<LuaProxy<NamedQuantity>>("NamedQuantity", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedQuantity>(utNamedQuantity);
    utNamedQuantity["value"] = [](LuaProxy<NamedQuantity> self) { return self.lock()->value(); };
    utNamedQuantity["getUnitSymbol"] = [](LuaProxy<NamedQuantity> self) { return self.lock()->getUnitSymbol(); };
    namedTable["createQuantity"] = sol::overload(
        [](const std::string& name, double v, const std::string& sym, sol::object parent) {
            std::shared_ptr<NamedQuantity> ptr = NamedQuantity::create(name, v, sym, extractNamedObject(parent));
            if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
            return LuaProxy<NamedQuantity>(ptr);
        },
        [](const std::string& name, double v, const quasar::coretypes::Unit& u, sol::object parent) {
            std::shared_ptr<NamedQuantity> ptr = NamedQuantity::create(name, v, u, extractNamedObject(parent));
            if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
            return LuaProxy<NamedQuantity>(ptr);
        }
    );

    // NamedVariant
    sol::usertype<LuaProxy<NamedVariant>> utNamedVariant = lua.new_usertype<LuaProxy<NamedVariant>>("NamedVariant", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedVariant>(utNamedVariant);
    utNamedVariant["set"] = [](LuaProxy<NamedVariant> self, sol::object obj) { self.lock()->set(extractNamedObject(obj)); };
    utNamedVariant["get"] = [](LuaProxy<NamedVariant> self) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedObject> obj = self.lock()->get();
        return obj ? std::make_optional(LuaProxy<NamedObject>(obj)) : std::nullopt;
    };
    namedTable["createVariant"] = [](const std::string& name, sol::object parent) {
        std::shared_ptr<NamedVariant> ptr = NamedVariant::create(name, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedVariant>(ptr);
    };

    // NamedBuffer
    sol::usertype<LuaProxy<NamedBuffer>> utNamedBuffer = lua.new_usertype<LuaProxy<NamedBuffer>>("NamedBuffer", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedBuffer>(utNamedBuffer);
    utNamedBuffer["getSize"] = [](LuaProxy<NamedBuffer> self) { return self.lock()->size(); };
    utNamedBuffer["read"] = [](LuaProxy<NamedBuffer> self, size_t off, size_t sz) {
        std::shared_ptr<NamedBuffer> b = self.lock();
        if (off >= b->size()) return std::vector<uint8_t>();
        return b->slice(off, std::min(sz, b->size() - off)).toVector();
    };
    utNamedBuffer["write"] = [](LuaProxy<NamedBuffer> self, size_t off, sol::table data) {
        std::shared_ptr<NamedBuffer> b = self.lock();
        for (size_t i = 1; i <= data.size() && (off + i - 1) < b->size(); ++i) {
            b->set(off + i - 1, data.get<uint8_t>(i));
        }
    };
    namedTable["createBuffer"] = [](const std::string& name, size_t sz, sol::object parent) {
        std::shared_ptr<NamedBuffer> ptr = NamedBuffer::create(name, sz, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedBuffer>(ptr);
    };

    // NamedBitBuffer
    sol::usertype<LuaProxy<NamedBitBuffer>> utNamedBitBuffer = lua.new_usertype<LuaProxy<NamedBitBuffer>>("NamedBitBuffer", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedBitBuffer>(utNamedBitBuffer);
    utNamedBitBuffer["getBitCount"] = [](LuaProxy<NamedBitBuffer> self) { return self.lock()->bitSize(); };
    utNamedBitBuffer["getBit"] = [](LuaProxy<NamedBitBuffer> self, size_t i) { return self.lock()->getBit(i); };
    utNamedBitBuffer["setBit"] = [](LuaProxy<NamedBitBuffer> self, size_t i, bool v) { self.lock()->setBit(i, v); };
    namedTable["createBitBuffer"] = [](const std::string& name, size_t bc, sol::object parent) {
        std::shared_ptr<NamedBitBuffer> ptr = NamedBitBuffer::create(name, bc, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedBitBuffer>(ptr);
    };

    // Collections
    using NamedObjectArray = NamedArray<NamedObject>;
    sol::usertype<LuaProxy<NamedObjectArray>> utNamedArray = lua.new_usertype<LuaProxy<NamedObjectArray>>("NamedArray", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedObjectArray>(utNamedArray);
    utNamedArray["size"] = [](LuaProxy<NamedObjectArray> self) { return self.lock()->size(); };
    utNamedArray["get"] = [](LuaProxy<NamedObjectArray> self, size_t i) -> std::optional<LuaProxy<NamedObject>> {
        try { return LuaProxy<NamedObject>(self.lock()->get(i - 1)); } catch(...) { return std::nullopt; }
    };
    namedTable["createArray"] = [](const std::string& name, sol::object parent) {
        std::shared_ptr<NamedObjectArray> ptr = NamedObjectArray::create(name, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedObjectArray>(ptr);
    };

    using NamedObjectMap = NamedMap<NamedObject>;
    sol::usertype<LuaProxy<NamedObjectMap>> utNamedMap = lua.new_usertype<LuaProxy<NamedObjectMap>>("NamedMap", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedObjectMap>(utNamedMap);
    utNamedMap["size"] = [](LuaProxy<NamedObjectMap> self) { return self.lock()->size(); };
    utNamedMap["get"] = [](LuaProxy<NamedObjectMap> self, const std::string& k) -> std::optional<LuaProxy<NamedObject>> {
        try { return LuaProxy<NamedObject>(self.lock()->get(k)); } catch(...) { return std::nullopt; }
    };
    namedTable["createMap"] = [](const std::string& name, sol::object parent) {
        std::shared_ptr<NamedObjectMap> ptr = NamedObjectMap::create(name, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedObjectMap>(ptr);
    };

    // ActiveEntity
    sol::usertype<LuaProxy<ActiveEntity>> utActiveEntity = lua.new_usertype<LuaProxy<ActiveEntity>>("ActiveEntity", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<ActiveEntity>(utActiveEntity);
    utActiveEntity["subscribe"] = [](LuaProxy<ActiveEntity> self, std::shared_ptr<IObserver> obs) { self.lock()->subscribe(obs); };
    utActiveEntity["unsubscribe"] = [](LuaProxy<ActiveEntity> self, std::shared_ptr<IObserver> obs) { self.lock()->unsubscribe(obs); };
    utActiveEntity["getState"] = [](LuaProxy<ActiveEntity> self) { return (int)self.lock()->getState(); };

    // NamedMethod
    sol::usertype<LuaProxy<NamedMethod>> utNamedMethod = lua.new_usertype<LuaProxy<NamedMethod>>("NamedMethod", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedMethod>(utNamedMethod);
    utNamedMethod["execute"] = [](LuaProxy<NamedMethod> self, sol::object args, sol::this_state L) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedMethod> method = self.lock();
        std::shared_ptr<NamedObject> argsObj = extractNamedObject(args);
        
        sol::state_view lua(L);
        sol::object engineObj = lua["__quasar_engine"];
        std::shared_ptr<NamedObject> res = nullptr;
        
        if (!engineObj.is<sol::nil_t>()) {
            LuaEngine* engine = engineObj.as<LuaEngine*>();
            engine->getMutex().unlock();
            res = method->execute(argsObj);
            engine->getMutex().lock();
        } else {
            res = method->execute(argsObj);
        }
        
        if (res && !res->getParent()) ObjectTracker::getInstance().trackStrong(res);
        return res ? std::make_optional(LuaProxy<NamedObject>(res)) : std::nullopt;
    };

    // NamedLuaMethod
    sol::usertype<LuaProxy<NamedLuaMethod>> utNamedLuaMethod = lua.new_usertype<LuaProxy<NamedLuaMethod>>("NamedLuaMethod", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindNamedObjectMethods<NamedLuaMethod>(utNamedLuaMethod);
    utNamedLuaMethod["execute"] = [](LuaProxy<NamedLuaMethod> self, sol::object args, sol::this_state L) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedLuaMethod> method = self.lock();
        std::shared_ptr<NamedObject> argsObj = extractNamedObject(args);
        
        sol::state_view lua(L);
        sol::object engineObj = lua["__quasar_engine"];
        std::shared_ptr<NamedObject> res = nullptr;
        
        if (!engineObj.is<sol::nil_t>()) {
            LuaEngine* engine = engineObj.as<LuaEngine*>();
            engine->getMutex().unlock();
            res = method->execute(argsObj);
            engine->getMutex().lock();
        } else {
            res = method->execute(argsObj);
        }

        if (res && !res->getParent()) ObjectTracker::getInstance().trackStrong(res);
        return res ? std::make_optional(LuaProxy<NamedObject>(res)) : std::nullopt;
    };
    namedTable["createLuaMethod"] = [](const std::string& name, sol::function f, sol::object parent) {
        std::shared_ptr<NamedLuaMethod> ptr = NamedLuaMethod::create(name, f, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
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
        if (!engineObj.is<sol::nil_t>()) {
            LuaEngine* engine = engineObj.as<LuaEngine*>();
            // [CS-0010.46] Release lock during stop to allow onStop hook to execute in the background thread.
            engine->getMutex().unlock();
            svc->stop();
            engine->getMutex().lock();
        } else {
            svc->stop();
        }
    };
    utNamedService["isRunning"] = [](LuaProxy<NamedService> self) { return self.lock()->isRunning(); };
    utNamedService["setCycleTime"] = [](LuaProxy<NamedService> self, int ms) { self.lock()->setCycleTime(std::chrono::milliseconds(ms)); };
    namedTable["createService"] = [](const std::string& name, sol::object parent) {
        std::shared_ptr<NamedService> ptr = NamedService::create(name, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<NamedService>(ptr);
    };

    // Global quasar methods
    quasarTable["resolve"] = [](sol::object root, const std::string& path) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedObject> ptr = extractNamedObject(root);
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
    quasarTable["sleep"] = [](int ms, sol::this_state L) {
        sol::state_view lua(L);
        sol::object engineObj = lua["__quasar_engine"];
        if (!engineObj.is<sol::nil_t>()) {
            LuaEngine* engine = engineObj.as<LuaEngine*>();
            // [CS-0010.46] Release lock during sleep to allow other threads (like NamedService) to use the Lua state.
            engine->getMutex().unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            engine->getMutex().lock();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
    };
    quasarTable["createObserver"] = [weakService = std::weak_ptr<LuaService>(service)](sol::function cb, sol::optional<size_t> wm) {
        return std::make_shared<QueuedObserver>(weakService.lock(), cb, wm.value_or(1000));
    };
    quasarTable["track"] = [](LuaProxy<NamedObject> obj) { ObjectTracker::getInstance().trackStrong(obj.lock()); };
    quasarTable["isAlive"] = [](LuaProxy<NamedObject> obj) { return obj.isAlive(); };
}

} // namespace quasar::scripting
