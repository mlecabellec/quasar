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
#include <list>
#include <memory>

namespace quasar::scripting {

using namespace quasar::named;

/**
 * @brief Helper to resolve a path in a NamedObject hierarchy and return a proxy.
 * [CS-0010.45] Documented with Doxygen.
 * @param root The root object proxy.
 * @param path The path string (e.g., "a/b/c").
 * @return Optional proxy to the resolved object.
 */
std::optional<LuaProxy<NamedObject>> resolvePathProxy(LuaProxy<NamedObject> root, const std::string& path) {
    // [CS-0010.44] Each code block documented.
    std::shared_ptr<NamedObject> rootPtr = root.lock();
    if (!rootPtr || path.empty()) return root;
    
    // [CS-0010.44] segments resolution.
    std::stringstream ss(path);
    std::string segment;
    std::shared_ptr<NamedObject> current = rootPtr;
    
    // [CS-0010.37] Hard limit on loops.
    std::size_t iterations = 0;
    while (std::getline(ss, segment, '/')) {
        if (++iterations > 1000) throw std::runtime_error("Hard limit in resolvePathProxy");
        if (segment.empty()) continue; 
        current = current->getChild(segment);
        if (!current) return std::nullopt;
    }
    
    // [CS-0010.4] Return using move semantics (implicit).
    return LuaProxy<NamedObject>(current);
}

namespace {

using NamedLong = NamedInteger<int64_t>;
using NamedDouble = NamedFloatingPoint<double>;
using NamedObjectArray = NamedArray<NamedObject>;
using NamedObjectMap = NamedMap<NamedObject>;

/** @brief Proxy name getter [CS-0010.45] */
template<typename T>
std::string proxy_getName(LuaProxy<T> self) { return self.lock()->getName(); }

/** @brief Proxy name setter [CS-0010.45] */
template<typename T>
void proxy_setName(LuaProxy<T> self, const std::string& name) { self.lock()->setName(name); }

/** @brief Proxy type getter [CS-0010.45] */
template<typename T>
std::string proxy_getType(LuaProxy<T> self) { return self.lock()->getType(); }

/** @brief Cast proxy to ActiveEntity [CS-0010.45] */
template<typename T>
std::optional<LuaProxy<ActiveEntity>> proxy_asActive(LuaProxy<T> self) {
    // [CS-0010.34] No auto.
    std::shared_ptr<ActiveEntity> ptr = std::dynamic_pointer_cast<ActiveEntity>(self.lock());
    return ptr ? std::make_optional(LuaProxy<ActiveEntity>(ptr)) : std::nullopt;
}

/** @brief Get child proxy by name [CS-0010.45] */
template<typename T>
std::optional<LuaProxy<NamedObject>> proxy_getChild(LuaProxy<T> self, const std::string& name) {
    // [CS-0010.34] No auto.
    std::shared_ptr<NamedObject> c = self.lock()->getChild(name);
    return c ? std::make_optional(LuaProxy<NamedObject>(c)) : std::nullopt;
}

/** @brief Get all children proxies [CS-0010.45] */
template<typename T>
std::vector<LuaProxy<NamedObject>> proxy_getChildren(LuaProxy<T> self) {
    // [CS-0010.34] Explicit types for collections.
    std::list<std::shared_ptr<NamedObject>> children = self.lock()->getChildren();
    std::vector<LuaProxy<NamedObject>> proxies;
    proxies.reserve(children.size());
    
    // [CS-0010.37] Hard limit on loops.
    std::size_t iterations = 0;
    for (std::shared_ptr<NamedObject>& c : children) {
        if (++iterations > 10000) throw std::runtime_error("Hard limit in proxy_getChildren");
        proxies.emplace_back(LuaProxy<NamedObject>(c));
    }
    return proxies;
}

/** @brief Cast proxy to NamedLong [CS-0010.45] */
template<typename T>
std::optional<LuaProxy<NamedInteger<int64_t>>> proxy_asLong(LuaProxy<T> self) {
    std::shared_ptr<NamedInteger<int64_t>> ptr = std::dynamic_pointer_cast<NamedInteger<int64_t>>(self.lock());
    return ptr ? std::make_optional(LuaProxy<NamedInteger<int64_t>>(ptr)) : std::nullopt;
}

/** @brief Cast proxy to NamedDouble [CS-0010.45] */
template<typename T>
std::optional<LuaProxy<NamedFloatingPoint<double>>> proxy_asDouble(LuaProxy<T> self) {
    std::shared_ptr<NamedFloatingPoint<double>> ptr = std::dynamic_pointer_cast<NamedFloatingPoint<double>>(self.lock());
    return ptr ? std::make_optional(LuaProxy<NamedFloatingPoint<double>>(ptr)) : std::nullopt;
}

/** @brief Cast proxy to NamedQuantity [CS-0010.45] */
template<typename T>
std::optional<LuaProxy<NamedQuantity>> proxy_asQuantity(LuaProxy<T> self) {
    std::shared_ptr<NamedQuantity> ptr = std::dynamic_pointer_cast<NamedQuantity>(self.lock());
    return ptr ? std::make_optional(LuaProxy<NamedQuantity>(ptr)) : std::nullopt;
}

/** @brief Cast proxy to NamedBuffer [CS-0010.45] */
template<typename T>
std::optional<LuaProxy<NamedBuffer>> proxy_asBuffer(LuaProxy<T> self) {
    std::shared_ptr<NamedBuffer> ptr = std::dynamic_pointer_cast<NamedBuffer>(self.lock());
    return ptr ? std::make_optional(LuaProxy<NamedBuffer>(ptr)) : std::nullopt;
}

/** @brief Cast proxy to NamedBitBuffer [CS-0010.45] */
template<typename T>
std::optional<LuaProxy<NamedBitBuffer>> proxy_asBitBuffer(LuaProxy<T> self) {
    std::shared_ptr<NamedBitBuffer> ptr = std::dynamic_pointer_cast<NamedBitBuffer>(self.lock());
    return ptr ? std::make_optional(LuaProxy<NamedBitBuffer>(ptr)) : std::nullopt;
}

/** @brief Extract shared_ptr from sol::object if it's a proxy [CS-0010.45] */
template<typename T>
std::shared_ptr<NamedObject> tryExtract(sol::object obj) {
    if (obj.is<LuaProxy<T>>()) return obj.as<LuaProxy<T>>().lock();
    return nullptr;
}

/** @brief Force extraction of a NamedObject from various proxy types [CS-0010.45] */
static std::shared_ptr<NamedObject> forceExtractNamedObject(sol::object obj) {
    // [CS-0010.44] Basic checks.
    if (!obj.valid() || obj.is<sol::nil_t>()) return nullptr;
    
    // [CS-0010.15] Null checks are implicit in tryExtract.
    if (std::shared_ptr<NamedObject> p = tryExtract<NamedObject>(obj)) return p;
    if (std::shared_ptr<NamedObject> p = tryExtract<ScriptableNamedObject>(obj)) return p;
    if (std::shared_ptr<NamedObject> p = tryExtract<NamedInteger<int64_t>>(obj)) return p;
    if (std::shared_ptr<NamedObject> p = tryExtract<NamedFloatingPoint<double>>(obj)) return p;
    if (std::shared_ptr<NamedObject> p = tryExtract<NamedQuantity>(obj)) return p;
    if (std::shared_ptr<NamedObject> p = tryExtract<NamedVariant>(obj)) return p;
    if (std::shared_ptr<NamedObject> p = tryExtract<NamedBuffer>(obj)) return p;
    if (std::shared_ptr<NamedObject> p = tryExtract<NamedBitBuffer>(obj)) return p;
    if (std::shared_ptr<NamedObject> p = tryExtract<NamedObjectArray>(obj)) return p;
    if (std::shared_ptr<NamedObject> p = tryExtract<NamedObjectMap>(obj)) return p;
    if (std::shared_ptr<NamedObject> p = tryExtract<ActiveEntity>(obj)) return p;
    
    return nullptr;
}

/** @brief Get parent proxy [CS-0010.45] */
template<typename T>
std::optional<LuaProxy<NamedObject>> proxy_getParent(LuaProxy<T> self) {
    std::shared_ptr<NamedObject> p = self.lock()->getParent();
    return p ? std::make_optional(LuaProxy<NamedObject>(p)) : std::nullopt;
}

/** @brief Proxy parent setter [CS-0010.45] */
template<typename T>
void proxy_setParent(LuaProxy<T> self, sol::object parent) {
    // [CS-0010.44] Each code block documented.
    self.lock()->setParent(forceExtractNamedObject(parent));
}

// NamedLong
/** @brief Get NamedLong value [CS-0010.45] */
static int64_t proxy_long_value(LuaProxy<NamedInteger<int64_t>> self) { return self.lock()->value(); }

// NamedDouble
/** @brief Get NamedDouble value [CS-0010.45] */
static double proxy_double_value(LuaProxy<NamedFloatingPoint<double>> self) { return self.lock()->value(); }

// NamedQuantity
/** @brief Get NamedQuantity value [CS-0010.45] */
static double proxy_quantity_value(LuaProxy<NamedQuantity> self) { return self.lock()->value(); }
/** @brief Get NamedQuantity unit [CS-0010.45] */
static std::string proxy_quantity_getUnitSymbol(LuaProxy<NamedQuantity> self) { return self.lock()->getUnitSymbol(); }

// NamedVariant
/** @brief Set NamedVariant value [CS-0010.45] */
static void proxy_variant_set(LuaProxy<NamedVariant> self, sol::object obj) {
    // [CS-0010.44] Forward to variant set.
    self.lock()->set(forceExtractNamedObject(obj));
}
/** @brief Get NamedVariant value [CS-0010.45] */
static std::optional<LuaProxy<NamedObject>> proxy_variant_get(LuaProxy<NamedVariant> self) {
    // [CS-0010.34] No auto.
    std::shared_ptr<NamedObject> obj = self.lock()->get();
    return obj ? std::make_optional(LuaProxy<NamedObject>(obj)) : std::nullopt;
}

// NamedBuffer
/** @brief Get buffer size [CS-0010.45] */
static size_t proxy_buffer_getSize(LuaProxy<NamedBuffer> self) { return self.lock()->size(); }
/** @brief Read from buffer [CS-0010.45] */
static std::vector<uint8_t> proxy_buffer_read(LuaProxy<NamedBuffer> self, size_t offset, size_t size) {
    // [CS-0010.34] No auto.
    std::shared_ptr<NamedBuffer> buf = self.lock();
    if (offset >= buf->size()) return {};
    
    // [CS-0010.44] Calculation of length with bounds check.
    size_t len = std::min(size, buf->size() - offset);
    return buf->slice(offset, len).toVector();
}
/** @brief Write to buffer [CS-0010.45] */
static void proxy_buffer_write(LuaProxy<NamedBuffer> self, size_t offset, const std::vector<uint8_t>& data) {
    // [CS-0010.34] No auto.
    std::shared_ptr<NamedBuffer> buf = self.lock();
    
    // [CS-0010.37] Hard limit on loops.
    std::size_t iterations = 0;
    for (size_t i = 0; i < data.size() && (offset + i) < buf->size(); ++i) {
        if (++iterations > 1000000) throw std::runtime_error("Hard limit in proxy_buffer_write");
        buf->set(offset + i, data[i]);
    }
}

// NamedBitBuffer
/** @brief Get bit count [CS-0010.45] */
static size_t proxy_bitbuffer_getBitCount(LuaProxy<NamedBitBuffer> self) { return self.lock()->bitSize(); }
/** @brief Get bit value [CS-0010.45] */
static bool proxy_bitbuffer_getBit(LuaProxy<NamedBitBuffer> self, size_t index) { return self.lock()->getBit(index); }
/** @brief Set bit value [CS-0010.45] */
static void proxy_bitbuffer_setBit(LuaProxy<NamedBitBuffer> self, size_t index, bool value) { self.lock()->setBit(index, value); }

// ScriptableNamedObject
/** @brief Forward event to scriptable object [CS-0010.45] */
static void proxy_scriptable_onEvent(LuaProxy<ScriptableNamedObject> self, const std::string& eventName, sol::object data) {
    // [CS-0010.44] Forward event call.
    self.lock()->onEvent(eventName, data);
}
/** @brief Set Lua self table [CS-0010.45] */
static void proxy_scriptable_setLuaSelf(LuaProxy<ScriptableNamedObject> self, sol::table table) {
    // [CS-0010.44] Store Lua table reference.
    self.lock()->setLuaSelf(table);
}
/** @brief Get Lua self table [CS-0010.45] */
static sol::table proxy_scriptable_getLuaSelf(LuaProxy<ScriptableNamedObject> self) {
    // [CS-0010.44] Retrieve Lua table reference.
    return self.lock()->getLuaSelf();
}

// Collections
/** @brief Get array size [CS-0010.45] */
static size_t proxy_array_size(LuaProxy<NamedObjectArray> self) { return self.lock()->size(); }
/** @brief Get array element by 1-based index [CS-0010.45] */
static std::optional<LuaProxy<NamedObject>> proxy_array_get(LuaProxy<NamedObjectArray> self, size_t index) {
    // [CS-0010.20] Mandatory exception handling.
    try { 
        return LuaProxy<NamedObject>(self.lock()->get(index - 1)); 
    } catch(...) { 
        return std::nullopt; 
    }
}

/** @brief Get map size [CS-0010.45] */
static size_t proxy_map_size(LuaProxy<NamedObjectMap> self) { return self.lock()->size(); }
/** @brief Get map element by key [CS-0010.45] */
static std::optional<LuaProxy<NamedObject>> proxy_map_get(LuaProxy<NamedObjectMap> self, const std::string& key) {
    // [CS-0010.20] Mandatory exception handling.
    try { 
        return LuaProxy<NamedObject>(self.lock()->get(key)); 
    } catch(...) { 
        return std::nullopt; 
    }
}

// ActiveEntity
/** @brief Subscribe observer [CS-0010.45] */
static void proxy_active_subscribe(LuaProxy<ActiveEntity> self, std::shared_ptr<IObserver> observer) {
    // [CS-0010.44] Delegate to ActiveEntity.
    self.lock()->subscribe(observer);
}
/** @brief Unsubscribe observer [CS-0010.45] */
static void proxy_active_unsubscribe(LuaProxy<ActiveEntity> self, std::shared_ptr<IObserver> observer) {
    // [CS-0010.44] Delegate to ActiveEntity.
    self.lock()->unsubscribe(observer);
}
/** @brief Get entity state [CS-0010.45] */
static int proxy_active_getState(LuaProxy<ActiveEntity> self) { 
    // [CS-0010.44] Cast enum to int for Lua.
    return (int)self.lock()->getState(); 
}

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

/** @brief Entry point for binding all Named derivatives to Lua [CS-0010.45] */
void bindNamedTypes(sol::state_view lua, std::shared_ptr<LuaService> service) {
    // --- Global quasar namespace table ---
    sol::table quasarTable = lua["quasar"].get_or_create<sol::table>();
    sol::table namedTable = quasarTable["named"].get_or_create<sol::table>();

    // --- IObserver Usertype ---
    lua.new_usertype<IObserver>("IObserver", sol::no_constructor);

    // --- NamedObject Proxy ---
    sol::usertype<LuaProxy<NamedObject>> utNamedObject = lua.new_usertype<LuaProxy<NamedObject>>("NamedObject", sol::no_constructor);
    bindNamedObjectMethods(utNamedObject);
    
    std::function<LuaProxy<NamedObject>(const std::string&, sol::object)> create_object = 
        [](const std::string& name, sol::object parent) {
            std::shared_ptr<NamedObject> ptr = NamedObject::create(name, forceExtractNamedObject(parent));
            if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
            return LuaProxy<NamedObject>(ptr);
        };
    namedTable["createObject"] = create_object;

    // --- ScriptableNamedObject Proxy ---
    sol::usertype<LuaProxy<ScriptableNamedObject>> utScriptable = lua.new_usertype<LuaProxy<ScriptableNamedObject>>("ScriptableNamedObject", sol::no_constructor);
    bindNamedObjectMethods(utScriptable);
    
    std::function<LuaProxy<ScriptableNamedObject>(const std::string&, sol::object)> create_scriptable = 
        [](const std::string& name, sol::object parent) {
            std::shared_ptr<ScriptableNamedObject> ptr = ScriptableNamedObject::create(name, forceExtractNamedObject(parent));
            if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
            return LuaProxy<ScriptableNamedObject>(ptr);
        };
    namedTable["createScriptable"] = create_scriptable;
    utScriptable["onEvent"] = &proxy_scriptable_onEvent;
    utScriptable["setLuaSelf"] = &proxy_scriptable_setLuaSelf;
    utScriptable["getLuaSelf"] = &proxy_scriptable_getLuaSelf;

    // --- NamedLong Proxy ---
    sol::usertype<LuaProxy<NamedLong>> utNamedLong = lua.new_usertype<LuaProxy<NamedLong>>("NamedLong", sol::no_constructor);
    bindNamedObjectMethods(utNamedLong);
    
    std::function<LuaProxy<NamedLong>(const std::string&, int64_t, sol::object)> create_long = 
        [](const std::string& name, int64_t value, sol::object parent) {
            std::shared_ptr<NamedLong> ptr = NamedLong::create(name, value, forceExtractNamedObject(parent));
            if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
            return LuaProxy<NamedLong>(ptr);
        };
    namedTable["createLong"] = create_long;
    utNamedLong["value"] = &proxy_long_value;

    // --- NamedDouble Proxy ---
    sol::usertype<LuaProxy<NamedDouble>> utNamedDouble = lua.new_usertype<LuaProxy<NamedDouble>>("NamedDouble", sol::no_constructor);
    bindNamedObjectMethods(utNamedDouble);
    
    std::function<LuaProxy<NamedDouble>(const std::string&, double, sol::object)> create_double = 
        [](const std::string& name, double value, sol::object parent) {
            std::shared_ptr<NamedDouble> ptr = NamedDouble::create(name, value, forceExtractNamedObject(parent));
            if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
            return LuaProxy<NamedDouble>(ptr);
        };
    namedTable["createDouble"] = create_double;
    utNamedDouble["value"] = &proxy_double_value;

    // --- NamedQuantity Proxy ---
    sol::usertype<LuaProxy<NamedQuantity>> utNamedQuantity = lua.new_usertype<LuaProxy<NamedQuantity>>("NamedQuantity", sol::no_constructor);
    bindNamedObjectMethods(utNamedQuantity);
    
    std::function<LuaProxy<NamedQuantity>(const std::string&, double, const std::string&, sol::object)> create_quantity_sym = 
        [](const std::string& name, double value, const std::string& unitSymbol, sol::object parent) {
            std::shared_ptr<NamedQuantity> ptr = NamedQuantity::create(name, value, unitSymbol, forceExtractNamedObject(parent));
            if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
            return LuaProxy<NamedQuantity>(ptr);
        };

    auto create_quantity = sol::overload(
        std::function<LuaProxy<NamedQuantity>(const std::string&, double, const quasar::coretypes::Unit&, sol::object)>(
            [](const std::string& name, double value, const quasar::coretypes::Unit& unit, sol::object parent) {
                std::shared_ptr<NamedQuantity> ptr = NamedQuantity::create(name, value, unit, forceExtractNamedObject(parent));
                if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
                return LuaProxy<NamedQuantity>(ptr);
            }),
        create_quantity_sym
    );
    namedTable["createQuantity"] = create_quantity;
    utNamedQuantity["value"] = &proxy_quantity_value;
    utNamedQuantity["getUnitSymbol"] = &proxy_quantity_getUnitSymbol;

    // --- NamedVariant Proxy ---
    sol::usertype<LuaProxy<NamedVariant>> utNamedVariant = lua.new_usertype<LuaProxy<NamedVariant>>("NamedVariant", sol::no_constructor);
    bindNamedObjectMethods(utNamedVariant);
    
    std::function<LuaProxy<NamedVariant>(const std::string&, sol::object)> create_variant = 
        [](const std::string& name, sol::object parent) {
            std::shared_ptr<NamedVariant> ptr = NamedVariant::create(name, forceExtractNamedObject(parent));
            if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
            return LuaProxy<NamedVariant>(ptr);
        };
    namedTable["createVariant"] = create_variant;
    utNamedVariant["set"] = &proxy_variant_set;
    utNamedVariant["get"] = &proxy_variant_get;

    // --- NamedBuffer Proxy ---
    sol::usertype<LuaProxy<NamedBuffer>> utNamedBuffer = lua.new_usertype<LuaProxy<NamedBuffer>>("NamedBuffer", sol::no_constructor);
    bindNamedObjectMethods(utNamedBuffer);
    
    std::function<LuaProxy<NamedBuffer>(const std::string&, size_t, sol::object)> create_buffer = 
        [](const std::string& name, size_t size, sol::object parent) {
            std::shared_ptr<NamedBuffer> ptr = NamedBuffer::create(name, size, forceExtractNamedObject(parent));
            if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
            return LuaProxy<NamedBuffer>(ptr);
        };
    namedTable["createBuffer"] = create_buffer;
    utNamedBuffer["getSize"] = &proxy_buffer_getSize;
    utNamedBuffer["read"] = &proxy_buffer_read;
    utNamedBuffer["write"] = [](LuaProxy<NamedBuffer> self, size_t offset, sol::table data) {
        std::shared_ptr<NamedBuffer> buf = self.lock();
        std::vector<uint8_t> vec;
        vec.reserve(data.size());
        for (size_t i = 1; i <= data.size(); ++i) {
            vec.push_back(data.get<uint8_t>(i));
        }
        proxy_buffer_write(self, offset, vec);
    };

    // --- NamedBitBuffer Proxy ---
    sol::usertype<LuaProxy<NamedBitBuffer>> utNamedBitBuffer = lua.new_usertype<LuaProxy<NamedBitBuffer>>("NamedBitBuffer", sol::no_constructor);
    bindNamedObjectMethods(utNamedBitBuffer);
    
    std::function<LuaProxy<NamedBitBuffer>(const std::string&, size_t, sol::object)> create_bitbuffer = 
        [](const std::string& name, size_t bitCount, sol::object parent) {
            std::shared_ptr<NamedBitBuffer> ptr = NamedBitBuffer::create(name, bitCount, forceExtractNamedObject(parent));
            if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
            return LuaProxy<NamedBitBuffer>(ptr);
        };
    namedTable["createBitBuffer"] = create_bitbuffer;
    utNamedBitBuffer["getBitCount"] = &proxy_bitbuffer_getBitCount;
    utNamedBitBuffer["getBit"] = &proxy_bitbuffer_getBit;
    utNamedBitBuffer["setBit"] = &proxy_bitbuffer_setBit;

    // --- Collection Proxies ---
    sol::usertype<LuaProxy<NamedObjectArray>> utNamedArray = lua.new_usertype<LuaProxy<NamedObjectArray>>("NamedArray", sol::no_constructor);
    bindNamedObjectMethods(utNamedArray);
    
    std::function<LuaProxy<NamedObjectArray>(const std::string&, sol::object)> create_array = 
        [](const std::string& name, sol::object parent) {
            std::shared_ptr<NamedObjectArray> ptr = NamedObjectArray::create(name, forceExtractNamedObject(parent));
            if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
            return LuaProxy<NamedObjectArray>(ptr);
        };
    namedTable["createArray"] = create_array;
    utNamedArray["size"] = &proxy_array_size;
    utNamedArray["get"] = &proxy_array_get;

    sol::usertype<LuaProxy<NamedObjectMap>> utNamedMap = lua.new_usertype<LuaProxy<NamedObjectMap>>("NamedMap", sol::no_constructor);
    bindNamedObjectMethods(utNamedMap);
    
    std::function<LuaProxy<NamedObjectMap>(const std::string&, sol::object)> create_map = 
        [](const std::string& name, sol::object parent) {
            std::shared_ptr<NamedObjectMap> ptr = NamedObjectMap::create(name, forceExtractNamedObject(parent));
            if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
            return LuaProxy<NamedObjectMap>(ptr);
        };
    namedTable["createMap"] = create_map;
    utNamedMap["size"] = &proxy_map_size;
    utNamedMap["get"] = &proxy_map_get;

    // --- ActiveEntity Proxy ---
    sol::usertype<LuaProxy<ActiveEntity>> utActiveEntity = lua.new_usertype<LuaProxy<ActiveEntity>>("ActiveEntity", sol::no_constructor);
    bindNamedObjectMethods(utActiveEntity);
    utActiveEntity["subscribe"] = &proxy_active_subscribe;
    utActiveEntity["unsubscribe"] = &proxy_active_unsubscribe;
    utActiveEntity["getState"] = &proxy_active_getState;

    // --- quasarTable methods ---
    quasarTable["resolve"] = &resolvePathProxy;
    
    quasarTable["createObserver"] = [weakService = std::weak_ptr<LuaService>(service)](sol::function callback, sol::optional<size_t> watermark) {
        return std::make_shared<QueuedObserver>(weakService.lock(), callback, watermark.value_or(1000));
    };

    quasarTable["track"] = [](LuaProxy<NamedObject> obj) {
        ObjectTracker::getInstance().trackStrong(obj.lock());
    };
    quasarTable["isAlive"] = [](LuaProxy<NamedObject> obj) {
        return obj.isAlive();
    };
}

} // namespace quasar::scripting
