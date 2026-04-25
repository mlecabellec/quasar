#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/LuaService.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/ScriptableNamedObject.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedTimestamp.hpp"
#include "quasar/named/NamedDuration.hpp"
#include "quasar/named/NamedDate.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedBitBuffer.hpp"
#include "quasar/named/NamedArray.hpp"
#include "quasar/named/NamedMap.hpp"
#include "quasar/named/NamedQuantity.hpp"
#include "quasar/named/NamedVariant.hpp"
#include "quasar/named/ActiveEntity.hpp"
#include "quasar/named/NamedMethod.hpp"
#include "quasar/named/NamedService.hpp"
#include "quasar/scripting/NamedLuaMethod.hpp"
#include "quasar/named/traversal/Transformer.hpp"
#include "quasar/named/Serialization.hpp"
#include <memory>
#include <vector>
#include <list>
#include <sstream>
#include <thread>

namespace quasar::scripting {

using namespace quasar::named;

// Internal path resolution logic
std::shared_ptr<NamedObject> resolvePath(std::shared_ptr<NamedObject> root, const std::string& path) {
    if (path.empty()) return root;
    
    std::shared_ptr<NamedObject> current = root;
    std::stringstream ss(path);
    std::string part;

    if (!path.empty() && path[0] == '/') {
        if (!current) return nullptr;
        while (current->getParent()) {
            current = current->getParent();
        }
        if (path.size() == 1) return current;
    }

    if (!current) return nullptr;

    while (std::getline(ss, part, '/')) {
        if (part.empty() || part == ".") continue;
        if (part == "..") {
            if (current->getParent()) current = current->getParent();
        } else {
            std::shared_ptr<NamedObject> child = current->getChild(part);
            if (child) current = child;
            else return nullptr;
        }
    }
    return current;
}

// Shared extraction helper
std::shared_ptr<NamedObject> extractNamedObject(sol::object obj) {
    if (!obj.valid() || obj.is<sol::nil_t>()) return nullptr;
    if (obj.is<ILuaProxy>()) return obj.as<ILuaProxy&>().lockAsNamedObject();
    return nullptr;
}

size_t getEngineId(sol::this_state L) {
    sol::state_view lua(L);
    sol::object engineObj = lua["__quasar_engine"];
    if (engineObj.valid() && engineObj.is<std::weak_ptr<LuaEngine>>()) {
        std::shared_ptr<LuaEngine> engine = engineObj.as<std::weak_ptr<LuaEngine>>().lock();
        if (engine) return engine->getId();
    }
    return 0;
}

template<typename T, typename U>
void bindBaseMethods(U& ut) {
    ut["getName"] = [](LuaProxy<T> self) -> std::string { return self.lock()->getName(); };
    ut["setName"] = [](LuaProxy<T> self, const std::string& name) { self.lock()->setName(name); };
    ut["getType"] = [](LuaProxy<T> self) -> std::string { return self.lock()->getType(); };
    ut["isAlive"] = [](LuaProxy<T> self) -> bool { return self.isAlive(); };
    ut["getParent"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedObject> p = self.lock()->getParent();
        return p ? std::make_optional(LuaProxy<NamedObject>(p)) : std::nullopt;
    };
    ut["getChild"] = [](LuaProxy<T> self, const std::string& name) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedObject> c = self.lock()->getChild(name);
        return c ? std::make_optional(LuaProxy<NamedObject>(c)) : std::nullopt;
    };
    ut["getChildren"] = [](LuaProxy<T> self) -> std::vector<LuaProxy<NamedObject>> {
        std::list<std::shared_ptr<NamedObject>> children = self.lock()->getChildren();
        std::vector<LuaProxy<NamedObject>> proxies;
        proxies.reserve(children.size());
        for (const std::shared_ptr<NamedObject>& c : children) proxies.emplace_back(c);
        return proxies;
    };
    
    // Explicit casting helpers
    ut["asLong"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedInteger<int64_t>>> {
        auto p = std::dynamic_pointer_cast<NamedInteger<int64_t>>(self.lock());
        return p ? std::make_optional(LuaProxy<NamedInteger<int64_t>>(p)) : std::nullopt;
    };
    ut["asDouble"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedFloatingPoint<double>>> {
        auto p = std::dynamic_pointer_cast<NamedFloatingPoint<double>>(self.lock());
        return p ? std::make_optional(LuaProxy<NamedFloatingPoint<double>>(p)) : std::nullopt;
    };
    ut["asBoolean"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedBoolean>> {
        auto p = std::dynamic_pointer_cast<NamedBoolean>(self.lock());
        return p ? std::make_optional(LuaProxy<NamedBoolean>(p)) : std::nullopt;
    };
    ut["asString"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedString>> {
        auto p = std::dynamic_pointer_cast<NamedString>(self.lock());
        return p ? std::make_optional(LuaProxy<NamedString>(p)) : std::nullopt;
    };
    ut["asBuffer"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedBuffer>> {
        auto p = std::dynamic_pointer_cast<NamedBuffer>(self.lock());
        return p ? std::make_optional(LuaProxy<NamedBuffer>(p)) : std::nullopt;
    };
    ut["asService"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedService>> {
        auto p = std::dynamic_pointer_cast<NamedService>(self.lock());
        return p ? std::make_optional(LuaProxy<NamedService>(p)) : std::nullopt;
    };
    ut["asVariant"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedVariant>> {
        auto p = std::dynamic_pointer_cast<NamedVariant>(self.lock());
        return p ? std::make_optional(LuaProxy<NamedVariant>(p)) : std::nullopt;
    };
    ut["asQuantity"] = [](LuaProxy<T> self) -> std::optional<LuaProxy<NamedQuantity>> {
        auto p = std::dynamic_pointer_cast<NamedQuantity>(self.lock());
        return p ? std::make_optional(LuaProxy<NamedQuantity>(p)) : std::nullopt;
    };
}

template<typename T, typename U>
void bindMethodMethods(U& ut) {
    ut["execute"] = [](LuaProxy<T> self, sol::object args, sol::this_state L) -> std::optional<LuaProxy<NamedObject>> {
        auto method = std::dynamic_pointer_cast<NamedMethod>(self.lock());
        if (!method) return std::nullopt;
        std::shared_ptr<NamedObject> res = method->execute(extractNamedObject(args));
        if (res) {
            if (!res->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), res);
            return std::make_optional(LuaProxy<NamedObject>(res));
        }
        return std::nullopt;
    };
}

void bindNamedTypes(sol::state_view lua, std::shared_ptr<LuaService> service) {
    sol::table quasarTable = lua["quasar"].get_or_create<sol::table>();
    sol::table namedTable = quasarTable["named"].get_or_create<sol::table>();

    lua.new_usertype<ILuaProxy>("ILuaProxy", sol::no_constructor, "isAlive", &ILuaProxy::isAlive);

    // 1. Core Usertypes
    sol::usertype<LuaProxy<NamedObject>> utNamedObject = lua.new_usertype<LuaProxy<NamedObject>>("NamedObject", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedObject>(utNamedObject);

    sol::usertype<LuaProxy<ScriptableNamedObject>> utScriptable = lua.new_usertype<LuaProxy<ScriptableNamedObject>>("ScriptableNamedObject", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<ScriptableNamedObject>(utScriptable);
    utScriptable["setLuaSelf"] = [](LuaProxy<ScriptableNamedObject> self, sol::table t) { self.lock()->setLuaSelf(t); };
    utScriptable["getLuaSelf"] = [](LuaProxy<ScriptableNamedObject> self) -> sol::table { return self.lock()->getLuaSelf(); };

    // 2. Primitives
    using NamedLong = NamedInteger<int64_t>;
    sol::usertype<LuaProxy<NamedLong>> utLong = lua.new_usertype<LuaProxy<NamedLong>>("NamedLong", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedLong>(utLong);
    utLong["value"] = [](LuaProxy<NamedLong> self) -> int64_t { return self.lock()->value(); };
    utLong["setValue"] = [](LuaProxy<NamedLong> self, int64_t v) { self.lock()->setValue(v); };

    using NamedDouble = NamedFloatingPoint<double>;
    sol::usertype<LuaProxy<NamedDouble>> utDouble = lua.new_usertype<LuaProxy<NamedDouble>>("NamedDouble", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedDouble>(utDouble);
    utDouble["value"] = [](LuaProxy<NamedDouble> self) -> double { return self.lock()->value(); };
    utDouble["setValue"] = [](LuaProxy<NamedDouble> self, double v) { self.lock()->setValue(v); };

    sol::usertype<LuaProxy<NamedBoolean>> utBool = lua.new_usertype<LuaProxy<NamedBoolean>>("NamedBoolean", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedBoolean>(utBool);
    utBool["value"] = [](LuaProxy<NamedBoolean> self) -> bool { return self.lock()->booleanValue(); };
    utBool["setValue"] = [](LuaProxy<NamedBoolean> self, bool v) { self.lock()->setValue(v); };

    sol::usertype<LuaProxy<NamedString>> utString = lua.new_usertype<LuaProxy<NamedString>>("NamedString", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedString>(utString);
    utString["value"] = [](LuaProxy<NamedString> self) -> std::string { return self.lock()->toString(); };
    utString["setValue"] = [](LuaProxy<NamedString> self, const std::string& v) { self.lock()->setValue(v); };

    sol::usertype<LuaProxy<NamedBuffer>> utBuffer = lua.new_usertype<LuaProxy<NamedBuffer>>("NamedBuffer", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedBuffer>(utBuffer);
    utBuffer["getSize"] = [](LuaProxy<NamedBuffer> self) -> size_t { return self.lock()->size(); };
    utBuffer["write"] = [](LuaProxy<NamedBuffer> self, long long offset, const std::string& data) {
        auto ptr = self.lock();
        for (size_t i = 0; i < data.size(); ++i) {
            ptr->set(static_cast<size_t>(offset) + i, static_cast<uint8_t>(data[i]));
        }
    };
    utBuffer["read"] = [](LuaProxy<NamedBuffer> self, long long offset, long long length) -> std::string {
        auto ptr = self.lock();
        std::string res;
        for (size_t i = 0; i < static_cast<size_t>(length); ++i) {
            res.push_back(static_cast<char>(ptr->get(static_cast<size_t>(offset) + i)));
        }
        return res;
    };

    // 3. Complex Types
    sol::usertype<LuaProxy<NamedVariant>> utVariant = lua.new_usertype<LuaProxy<NamedVariant>>("NamedVariant", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedVariant>(utVariant);
    utVariant["get"] = [](LuaProxy<NamedVariant> self, sol::this_state L) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedObject> obj = self.lock()->get();
        if (obj) {
            if (!obj->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), obj);
            return std::make_optional(LuaProxy<NamedObject>(obj));
        }
        return std::nullopt;
    };
    utVariant["set"] = [](LuaProxy<NamedVariant> self, sol::object v) { self.lock()->set(extractNamedObject(v)); };

    sol::usertype<LuaProxy<NamedQuantity>> utQuantity = lua.new_usertype<LuaProxy<NamedQuantity>>("NamedQuantity", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedQuantity>(utQuantity);
    utQuantity["value"] = [](LuaProxy<NamedQuantity> self) -> double { return self.lock()->value(); };
    utQuantity["setValue"] = [](LuaProxy<NamedQuantity> self, double v) { self.lock()->setValue(v); };
    utQuantity["getUnit"] = [](LuaProxy<NamedQuantity> self) -> std::string { return self.lock()->getUnitSymbol(); };

    // 4. Collections
    using NamedArrayObj = NamedArray<NamedObject>;
    sol::usertype<LuaProxy<NamedArrayObj>> utArray = lua.new_usertype<LuaProxy<NamedArrayObj>>("NamedArray", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedArrayObj>(utArray);
    utArray["size"] = [](LuaProxy<NamedArrayObj> self) -> size_t { return self.lock()->getChildren().size(); };
    utArray["get"] = [](LuaProxy<NamedArrayObj> self, size_t i) -> std::optional<LuaProxy<NamedObject>> {
        auto c = self.lock()->getChild(std::to_string(i - 1));
        return c ? std::make_optional(LuaProxy<NamedObject>(c)) : std::nullopt;
    };

    using NamedMapObj = NamedMap<NamedObject>;
    sol::usertype<LuaProxy<NamedMapObj>> utMap = lua.new_usertype<LuaProxy<NamedMapObj>>("NamedMap", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedMapObj>(utMap);
    utMap["size"] = [](LuaProxy<NamedMapObj> self) -> size_t { return self.lock()->getChildren().size(); };
    utMap["get"] = [](LuaProxy<NamedMapObj> self, const std::string& k) -> std::optional<LuaProxy<NamedObject>> {
        auto c = self.lock()->getChild(k);
        return c ? std::make_optional(LuaProxy<NamedObject>(c)) : std::nullopt;
    };

    // 5. Methods & Services
    sol::usertype<LuaProxy<NamedMethod>> utMethod = lua.new_usertype<LuaProxy<NamedMethod>>("NamedMethod", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedMethod>(utMethod);
    bindMethodMethods<NamedMethod>(utMethod);

    sol::usertype<LuaProxy<NamedLuaMethod>> utLuaMethod = lua.new_usertype<LuaProxy<NamedLuaMethod>>("NamedLuaMethod", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedLuaMethod>(utLuaMethod);
    bindMethodMethods<NamedLuaMethod>(utLuaMethod);
    
    sol::usertype<LuaProxy<NamedService>> utService = lua.new_usertype<LuaProxy<NamedService>>("NamedService", sol::no_constructor, sol::base_classes, sol::bases<ILuaProxy>());
    bindBaseMethods<NamedService>(utService);
    utService["start"] = [](LuaProxy<NamedService> self) { self.lock()->start(); };
    utService["stop"] = [](LuaProxy<NamedService> self) { self.lock()->stop(); };
    utService["isRunning"] = [](LuaProxy<NamedService> self) -> bool { return self.lock()->isRunning(); };

    // 6. Factory methods
    namedTable["createObject"] = [](const std::string& name, sol::object parent, sol::this_state L) -> LuaProxy<NamedObject> {
        auto ptr = NamedObject::create(name, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedObject>(ptr);
    };
    namedTable["createLong"] = [](const std::string& name, int64_t v, sol::object parent, sol::this_state L) -> LuaProxy<NamedLong> {
        auto ptr = NamedLong::create(name, v, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedLong>(ptr);
    };
    namedTable["createDouble"] = [](const std::string& name, double v, sol::object parent, sol::this_state L) -> LuaProxy<NamedDouble> {
        auto ptr = NamedDouble::create(name, v, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedDouble>(ptr);
    };
    namedTable["createBoolean"] = [](const std::string& name, bool v, sol::object parent, sol::this_state L) -> LuaProxy<NamedBoolean> {
        auto ptr = NamedBoolean::create(name, v, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedBoolean>(ptr);
    };
    namedTable["createString"] = [](const std::string& name, const std::string& v, sol::object parent, sol::this_state L) -> LuaProxy<NamedString> {
        auto ptr = NamedString::create(name, v, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedString>(ptr);
    };
    namedTable["createBuffer"] = [](const std::string& name, size_t sz, sol::object parent, sol::this_state L) -> LuaProxy<NamedBuffer> {
        auto ptr = NamedBuffer::create(name, sz, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedBuffer>(ptr);
    };
    namedTable["createArray"] = [](const std::string& name, sol::object parent, sol::this_state L) -> LuaProxy<NamedArrayObj> {
        auto ptr = NamedArrayObj::create(name, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedArrayObj>(ptr);
    };
    namedTable["createMap"] = [](const std::string& name, sol::object parent, sol::this_state L) -> LuaProxy<NamedMapObj> {
        auto ptr = NamedMapObj::create(name, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedMapObj>(ptr);
    };
    namedTable["createService"] = [](const std::string& name, sol::object parent, sol::this_state L) -> LuaProxy<NamedService> {
        auto ptr = NamedService::create(name, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedService>(ptr);
    };
    namedTable["createLuaMethod"] = [](const std::string& name, sol::protected_function f, sol::object parent, sol::this_state L) -> LuaProxy<NamedLuaMethod> {
        auto ptr = NamedLuaMethod::create(name, f, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedLuaMethod>(ptr);
    };

    // 7. Helpers
    std::weak_ptr<LuaService> weakSvc = service;
    quasarTable["resolve"] = [weakSvc](sol::object rootOrPath, sol::object maybePath) -> std::optional<LuaProxy<NamedObject>> {
        std::shared_ptr<NamedObject> root = nullptr;
        if (auto svc = weakSvc.lock()) root = svc->getSelf();
        
        if (maybePath.is<std::string>()) {
             std::shared_ptr<NamedObject> p = resolvePath(extractNamedObject(rootOrPath), maybePath.as<std::string>());
             return p ? std::make_optional(LuaProxy<NamedObject>(p)) : std::nullopt;
        } else if (rootOrPath.is<std::string>()) {
             std::shared_ptr<NamedObject> p = resolvePath(root, rootOrPath.as<std::string>()); 
             return p ? std::make_optional(LuaProxy<NamedObject>(p)) : std::nullopt;
        }
        return std::nullopt;
    };

    quasarTable["sleep"] = [](double ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(ms)));
    };
}

} // namespace quasar::scripting
