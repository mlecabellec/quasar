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
#include "quasar/named/NamedTimestamp.hpp"
#include "quasar/named/NamedDuration.hpp"
#include "quasar/named/NamedDate.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedBitBuffer.hpp"
#include "quasar/named/NamedArray.hpp"
#include "quasar/named/NamedMap.hpp"
#include "quasar/named/ActiveEntity.hpp"
#include <sstream>
#include <iostream>
#include <optional>
#include <vector>

namespace quasar::scripting {

using namespace quasar::named;

/**
 * @brief Helper to resolve a path in a NamedObject hierarchy and return a proxy.
 */
std::optional<LuaProxy<NamedObject>> resolvePathProxy(LuaProxy<NamedObject> root, const std::string& path) {
    auto rootPtr = root.lock();
    if (!rootPtr || path.empty()) return root;
    
    std::stringstream ss(path);
    std::string segment;
    std::shared_ptr<NamedObject> current = rootPtr;
    
    while (std::getline(ss, segment, '/')) {
        if (segment.empty()) continue; 
        current = current->getChild(segment);
        if (!current) return std::nullopt;
    }
    
    return LuaProxy<NamedObject>(current);
}

namespace {

using NamedLong = NamedInteger<int64_t>;
using NamedDouble = NamedFloatingPoint<double>;
using NamedObjectArray = NamedArray<NamedObject>;
using NamedObjectMap = NamedMap<NamedObject>;

template<typename T>
std::string proxy_getName(LuaProxy<T> self) { return self.lock()->getName(); }

template<typename T>
void proxy_setName(LuaProxy<T> self, const std::string& name) { self.lock()->setName(name); }

template<typename T>
std::string proxy_getType(LuaProxy<T> self) { return self.lock()->getType(); }

template<typename T>
std::optional<LuaProxy<ActiveEntity>> proxy_asActive(LuaProxy<T> self) {
    auto ptr = std::dynamic_pointer_cast<ActiveEntity>(self.lock());
    return ptr ? std::make_optional(LuaProxy<ActiveEntity>(ptr)) : std::nullopt;
}

template<typename T>
std::optional<LuaProxy<NamedObject>> proxy_getChild(LuaProxy<T> self, const std::string& name) {
    auto c = self.lock()->getChild(name);
    return c ? std::make_optional(LuaProxy<NamedObject>(c)) : std::nullopt;
}

template<typename T>
std::vector<LuaProxy<NamedObject>> proxy_getChildren(LuaProxy<T> self) {
    auto children = self.lock()->getChildren();
    std::vector<LuaProxy<NamedObject>> proxies;
    proxies.reserve(children.size());
    for (auto& c : children) proxies.emplace_back(LuaProxy<NamedObject>(c));
    return proxies;
}

template<typename T>
std::optional<LuaProxy<NamedInteger<int64_t>>> proxy_asLong(LuaProxy<T> self) {
    auto ptr = std::dynamic_pointer_cast<NamedInteger<int64_t>>(self.lock());
    return ptr ? std::make_optional(LuaProxy<NamedInteger<int64_t>>(ptr)) : std::nullopt;
}

template<typename T>
std::optional<LuaProxy<NamedFloatingPoint<double>>> proxy_asDouble(LuaProxy<T> self) {
    auto ptr = std::dynamic_pointer_cast<NamedFloatingPoint<double>>(self.lock());
    return ptr ? std::make_optional(LuaProxy<NamedFloatingPoint<double>>(ptr)) : std::nullopt;
}

template<typename T>
std::optional<LuaProxy<NamedQuantity>> proxy_asQuantity(LuaProxy<T> self) {
    auto ptr = std::dynamic_pointer_cast<NamedQuantity>(self.lock());
    return ptr ? std::make_optional(LuaProxy<NamedQuantity>(ptr)) : std::nullopt;
}

template<typename T>
std::optional<LuaProxy<NamedBuffer>> proxy_asBuffer(LuaProxy<T> self) {
    auto ptr = std::dynamic_pointer_cast<NamedBuffer>(self.lock());
    return ptr ? std::make_optional(LuaProxy<NamedBuffer>(ptr)) : std::nullopt;
}

template<typename T>
std::optional<LuaProxy<NamedBitBuffer>> proxy_asBitBuffer(LuaProxy<T> self) {
    auto ptr = std::dynamic_pointer_cast<NamedBitBuffer>(self.lock());
    return ptr ? std::make_optional(LuaProxy<NamedBitBuffer>(ptr)) : std::nullopt;
}

template<typename T>
std::shared_ptr<NamedObject> tryExtract(sol::object obj) {
    if (obj.is<LuaProxy<T>>()) return obj.as<LuaProxy<T>>().lock();
    return nullptr;
}

static std::shared_ptr<NamedObject> forceExtractNamedObject(sol::object obj) {
    if (!obj.valid() || obj.is<sol::nil_t>()) return nullptr;
    if (auto p = tryExtract<NamedObject>(obj)) return p;
    if (auto p = tryExtract<ScriptableNamedObject>(obj)) return p;
    if (auto p = tryExtract<NamedInteger<int64_t>>(obj)) return p;
    if (auto p = tryExtract<NamedFloatingPoint<double>>(obj)) return p;
    if (auto p = tryExtract<NamedQuantity>(obj)) return p;
    if (auto p = tryExtract<NamedVariant>(obj)) return p;
    if (auto p = tryExtract<NamedBuffer>(obj)) return p;
    if (auto p = tryExtract<NamedBitBuffer>(obj)) return p;
    if (auto p = tryExtract<NamedObjectArray>(obj)) return p;
    if (auto p = tryExtract<NamedObjectMap>(obj)) return p;
    if (auto p = tryExtract<ActiveEntity>(obj)) return p;
    return nullptr;
}

template<typename T>
std::optional<LuaProxy<NamedObject>> proxy_getParent(LuaProxy<T> self) {
    auto p = self.lock()->getParent();
    return p ? std::make_optional(LuaProxy<NamedObject>(p)) : std::nullopt;
}

template<typename T>
void proxy_setParent(LuaProxy<T> self, sol::object parent) {
    self.lock()->setParent(forceExtractNamedObject(parent));
}

// NamedLong
static int64_t proxy_long_value(LuaProxy<NamedInteger<int64_t>> self) { return self.lock()->value(); }

// NamedDouble
static double proxy_double_value(LuaProxy<NamedFloatingPoint<double>> self) { return self.lock()->value(); }

// NamedQuantity
static double proxy_quantity_value(LuaProxy<NamedQuantity> self) { return self.lock()->value(); }
static std::string proxy_quantity_getUnitSymbol(LuaProxy<NamedQuantity> self) { return self.lock()->getUnitSymbol(); }

// NamedVariant
static void proxy_variant_set(LuaProxy<NamedVariant> self, sol::object obj) {
    self.lock()->set(forceExtractNamedObject(obj));
}
static std::optional<LuaProxy<NamedObject>> proxy_variant_get(LuaProxy<NamedVariant> self) {
    auto obj = self.lock()->get();
    return obj ? std::make_optional(LuaProxy<NamedObject>(obj)) : std::nullopt;
}

// NamedBuffer
static size_t proxy_buffer_getSize(LuaProxy<NamedBuffer> self) { return self.lock()->size(); }
static std::vector<uint8_t> proxy_buffer_read(LuaProxy<NamedBuffer> self, size_t offset, size_t size) {
    auto buf = self.lock();
    if (offset >= buf->size()) return {};
    size_t len = std::min(size, buf->size() - offset);
    return buf->slice(offset, len).toVector();
}
static void proxy_buffer_write(LuaProxy<NamedBuffer> self, size_t offset, const std::vector<uint8_t>& data) {
    auto buf = self.lock();
    for (size_t i = 0; i < data.size() && (offset + i) < buf->size(); ++i) {
        buf->set(offset + i, data[i]);
    }
}

// NamedBitBuffer
static size_t proxy_bitbuffer_getBitCount(LuaProxy<NamedBitBuffer> self) { return self.lock()->bitSize(); }
static bool proxy_bitbuffer_getBit(LuaProxy<NamedBitBuffer> self, size_t index) { return self.lock()->getBit(index); }
static void proxy_bitbuffer_setBit(LuaProxy<NamedBitBuffer> self, size_t index, bool value) { self.lock()->setBit(index, value); }

// ScriptableNamedObject
static void proxy_scriptable_onEvent(LuaProxy<ScriptableNamedObject> self, const std::string& eventName, sol::object data) {
    self.lock()->onEvent(eventName, data);
}
static void proxy_scriptable_setLuaSelf(LuaProxy<ScriptableNamedObject> self, sol::table table) {
    self.lock()->setLuaSelf(table);
}
static sol::table proxy_scriptable_getLuaSelf(LuaProxy<ScriptableNamedObject> self) {
    return self.lock()->getLuaSelf();
}

// Collections
static size_t proxy_array_size(LuaProxy<NamedObjectArray> self) { return self.lock()->size(); }
static std::optional<LuaProxy<NamedObject>> proxy_array_get(LuaProxy<NamedObjectArray> self, size_t index) {
    try { return LuaProxy<NamedObject>(self.lock()->get(index - 1)); } catch(...) { return std::nullopt; }
}

static size_t proxy_map_size(LuaProxy<NamedObjectMap> self) { return self.lock()->size(); }
static std::optional<LuaProxy<NamedObject>> proxy_map_get(LuaProxy<NamedObjectMap> self, const std::string& key) {
    try { return LuaProxy<NamedObject>(self.lock()->get(key)); } catch(...) { return std::nullopt; }
}

// ActiveEntity
static void proxy_active_subscribe(LuaProxy<ActiveEntity> self, std::shared_ptr<IObserver> observer) {
    self.lock()->subscribe(observer);
}
static void proxy_active_unsubscribe(LuaProxy<ActiveEntity> self, std::shared_ptr<IObserver> observer) {
    self.lock()->unsubscribe(observer);
}
static int proxy_active_getState(LuaProxy<ActiveEntity> self) { return (int)self.lock()->getState(); }

} // namespace

/**
 * @brief Helper to bind common NamedObject methods to a Proxy usertype.
 */
template<typename T>
void bindNamedObjectMethods(sol::usertype<LuaProxy<T>>& ut) {
    ut["getName"] = &proxy_getName<T>;
    ut["setName"] = &proxy_setName<T>;
    ut["getType"] = &proxy_getType<T>;
    ut["isAlive"] = &LuaProxy<T>::isAlive;
    ut["getParent"] = &proxy_getParent<T>;
    ut["getChild"] = &proxy_getChild<T>;
    ut["getChildren"] = &proxy_getChildren<T>;
    ut["asLong"] = &proxy_asLong<T>;
    ut["asDouble"] = &proxy_asDouble<T>;
    ut["asQuantity"] = &proxy_asQuantity<T>;
    ut["asBuffer"] = &proxy_asBuffer<T>;
    ut["asBitBuffer"] = &proxy_asBitBuffer<T>;
    ut["asActive"] = &proxy_asActive<T>;
}

void bindNamedTypes(sol::state_view lua, std::shared_ptr<LuaService> service) {
    // --- IObserver Usertype ---
    // (Used to allow passing QueuedObserver to subscribe)
    lua.new_usertype<IObserver>("IObserver", sol::no_constructor);

    // --- NamedObject Proxy ---
    auto utNamedObject = lua.new_usertype<LuaProxy<NamedObject>>("NamedObject", sol::no_constructor);
    bindNamedObjectMethods(utNamedObject);

    // --- ScriptableNamedObject Proxy ---
    auto utScriptable = lua.new_usertype<LuaProxy<ScriptableNamedObject>>("ScriptableNamedObject", sol::no_constructor);
    bindNamedObjectMethods(utScriptable);
    std::function<LuaProxy<ScriptableNamedObject>(const std::string&, sol::object)> create_scriptable = 
        [](const std::string& name, sol::object parent) {
            return LuaProxy<ScriptableNamedObject>(ScriptableNamedObject::create(name, forceExtractNamedObject(parent)));
        };
    utScriptable["new"] = create_scriptable;
    utScriptable["create"] = create_scriptable;
    utScriptable["onEvent"] = &proxy_scriptable_onEvent;
    utScriptable["setLuaSelf"] = &proxy_scriptable_setLuaSelf;
    utScriptable["getLuaSelf"] = &proxy_scriptable_getLuaSelf;

    // --- NamedLong Proxy ---
    auto utNamedLong = lua.new_usertype<LuaProxy<NamedLong>>("NamedLong", sol::no_constructor);
    bindNamedObjectMethods(utNamedLong);
    std::function<LuaProxy<NamedLong>(const std::string&, int64_t, sol::object)> create_long = 
        [](const std::string& name, int64_t value, sol::object parent) {
            return LuaProxy<NamedLong>(NamedLong::create(name, value, forceExtractNamedObject(parent)));
        };
    utNamedLong["new"] = create_long;
    utNamedLong["create"] = create_long;
    utNamedLong["value"] = &proxy_long_value;

    // --- NamedDouble Proxy ---
    auto utNamedDouble = lua.new_usertype<LuaProxy<NamedDouble>>("NamedDouble", sol::no_constructor);
    bindNamedObjectMethods(utNamedDouble);
    std::function<LuaProxy<NamedDouble>(const std::string&, double, sol::object)> create_double = 
        [](const std::string& name, double value, sol::object parent) {
            return LuaProxy<NamedDouble>(NamedDouble::create(name, value, forceExtractNamedObject(parent)));
        };
    utNamedDouble["new"] = create_double;
    utNamedDouble["create"] = create_double;
    utNamedDouble["value"] = &proxy_double_value;

    // --- NamedQuantity Proxy ---
    auto utNamedQuantity = lua.new_usertype<LuaProxy<NamedQuantity>>("NamedQuantity", sol::no_constructor);
    bindNamedObjectMethods(utNamedQuantity);
    auto create_quantity = sol::overload(
        std::function<LuaProxy<NamedQuantity>(const std::string&, double, const quasar::coretypes::Unit&, sol::object)>(
            [](const std::string& name, double value, const quasar::coretypes::Unit& unit, sol::object parent) {
                return LuaProxy<NamedQuantity>(NamedQuantity::create(name, value, unit, forceExtractNamedObject(parent)));
            }),
        std::function<LuaProxy<NamedQuantity>(const std::string&, double, const std::string&, sol::object)>(
            [](const std::string& name, double value, const std::string& unitSymbol, sol::object parent) {
                return LuaProxy<NamedQuantity>(NamedQuantity::create(name, value, unitSymbol, forceExtractNamedObject(parent)));
            })
    );
    utNamedQuantity["new"] = create_quantity;
    utNamedQuantity["create"] = create_quantity;
    utNamedQuantity["value"] = &proxy_quantity_value;
    utNamedQuantity["getUnitSymbol"] = &proxy_quantity_getUnitSymbol;

    // --- NamedVariant Proxy ---
    auto utNamedVariant = lua.new_usertype<LuaProxy<NamedVariant>>("NamedVariant", sol::no_constructor);
    bindNamedObjectMethods(utNamedVariant);
    std::function<LuaProxy<NamedVariant>(const std::string&, sol::object)> create_variant = 
        [](const std::string& name, sol::object parent) {
            return LuaProxy<NamedVariant>(NamedVariant::create(name, forceExtractNamedObject(parent)));
        };
    utNamedVariant["new"] = create_variant;
    utNamedVariant["create"] = create_variant;
    utNamedVariant["set"] = &proxy_variant_set;
    utNamedVariant["get"] = &proxy_variant_get;

    // --- NamedBuffer Proxy ---
    auto utNamedBuffer = lua.new_usertype<LuaProxy<NamedBuffer>>("NamedBuffer", sol::no_constructor);
    bindNamedObjectMethods(utNamedBuffer);
    std::function<LuaProxy<NamedBuffer>(const std::string&, size_t, sol::object)> create_buffer = 
        [](const std::string& name, size_t size, sol::object parent) {
            return LuaProxy<NamedBuffer>(NamedBuffer::create(name, size, forceExtractNamedObject(parent)));
        };
    utNamedBuffer["new"] = create_buffer;
    utNamedBuffer["create"] = create_buffer;
    utNamedBuffer["getSize"] = &proxy_buffer_getSize;
    utNamedBuffer["read"] = &proxy_buffer_read;
    utNamedBuffer["write"] = &proxy_buffer_write;

    // --- NamedBitBuffer Proxy ---
    auto utNamedBitBuffer = lua.new_usertype<LuaProxy<NamedBitBuffer>>("NamedBitBuffer", sol::no_constructor);
    bindNamedObjectMethods(utNamedBitBuffer);
    std::function<LuaProxy<NamedBitBuffer>(const std::string&, size_t, sol::object)> create_bitbuffer = 
        [](const std::string& name, size_t bitCount, sol::object parent) {
            return LuaProxy<NamedBitBuffer>(NamedBitBuffer::create(name, bitCount, forceExtractNamedObject(parent)));
        };
    utNamedBitBuffer["new"] = create_bitbuffer;
    utNamedBitBuffer["create"] = create_bitbuffer;
    utNamedBitBuffer["getBitCount"] = &proxy_bitbuffer_getBitCount;
    utNamedBitBuffer["getBit"] = &proxy_bitbuffer_getBit;
    utNamedBitBuffer["setBit"] = &proxy_bitbuffer_setBit;

    // --- Collection Proxies ---
    auto utNamedArray = lua.new_usertype<LuaProxy<NamedObjectArray>>("NamedArray", sol::no_constructor);
    bindNamedObjectMethods(utNamedArray);
    std::function<LuaProxy<NamedObjectArray>(const std::string&, sol::object)> create_array = 
        [](const std::string& name, sol::object parent) {
            return LuaProxy<NamedObjectArray>(NamedObjectArray::create(name, forceExtractNamedObject(parent)));
        };
    utNamedArray["new"] = create_array;
    utNamedArray["create"] = create_array;
    utNamedArray["size"] = &proxy_array_size;
    utNamedArray["get"] = &proxy_array_get;

    auto utNamedMap = lua.new_usertype<LuaProxy<NamedObjectMap>>("NamedMap", sol::no_constructor);
    bindNamedObjectMethods(utNamedMap);
    std::function<LuaProxy<NamedObjectMap>(const std::string&, sol::object)> create_map = 
        [](const std::string& name, sol::object parent) {
            return LuaProxy<NamedObjectMap>(NamedObjectMap::create(name, forceExtractNamedObject(parent)));
        };
    utNamedMap["new"] = create_map;
    utNamedMap["create"] = create_map;
    utNamedMap["size"] = &proxy_map_size;
    utNamedMap["get"] = &proxy_map_get;

    // --- ActiveEntity Proxy ---
    auto utActiveEntity = lua.new_usertype<LuaProxy<ActiveEntity>>("ActiveEntity", sol::no_constructor);
    bindNamedObjectMethods(utActiveEntity);
    utActiveEntity["subscribe"] = &proxy_active_subscribe;
    utActiveEntity["unsubscribe"] = &proxy_active_unsubscribe;
    utActiveEntity["getState"] = &proxy_active_getState;

    // --- Global quasar table ---
    sol::table quasarTable = lua.create_named_table("quasar");
    quasarTable["resolve"] = &resolvePathProxy;
    
    // Observer Factory
    quasarTable["createObserver"] = [service](sol::function callback, sol::optional<size_t> watermark) {
        return std::make_shared<QueuedObserver>(service, callback, watermark.value_or(1000));
    };

    // Lifecycle Tracking
    quasarTable["track"] = [](LuaProxy<NamedObject> obj) {
        ObjectTracker::getInstance().track(obj.lock());
    };
    quasarTable["isAlive"] = [](LuaProxy<NamedObject> obj) {
        return obj.isAlive();
    };
}

} // namespace quasar::scripting
