#include "quasar/scripting/RegistryBindings_Internal.hpp"
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/ScriptableNamedObject.hpp"

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

void bindCoreTypes(sol::state_view lua, std::shared_ptr<LuaService> service) {
    sol::table quasarTable = lua["quasar"].get_or_create<sol::table>();
    sol::table namedTable = quasarTable["named"].get_or_create<sol::table>();

    lua.new_usertype<ILuaProxy>("ILuaProxy", sol::no_constructor, "isAlive", &ILuaProxy::isAlive);

    // NamedObject
    sol::usertype<LuaProxy<NamedObject>> utNamedObject = lua.new_usertype<LuaProxy<NamedObject>>("NamedObject", 
        sol::no_constructor, 
        sol::base_classes, sol::bases<ILuaProxy>()
    );
    bindBaseMethods<NamedObject>(utNamedObject);
    
    namedTable.set_function("createObject", [](const std::string& name, sol::object parent, sol::this_state L) -> LuaProxy<NamedObject> {
        std::shared_ptr<NamedObject> ptr = NamedObject::create(name, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<NamedObject>(ptr);
    });

    // Scriptable
    sol::usertype<LuaProxy<ScriptableNamedObject>> utScriptable = lua.new_usertype<LuaProxy<ScriptableNamedObject>>("ScriptableNamedObject", 
        sol::no_constructor, 
        sol::base_classes, sol::bases<ILuaProxy>()
    );
    bindBaseMethods<ScriptableNamedObject>(utScriptable);
    utScriptable["setLuaSelf"] = [](LuaProxy<ScriptableNamedObject> self, sol::table t) { self.lock()->setLuaSelf(t); };
    utScriptable["getLuaSelf"] = [](LuaProxy<ScriptableNamedObject> self) { return self.lock()->getLuaSelf(); };
    utScriptable["onEvent"] = [](LuaProxy<ScriptableNamedObject> self, const std::string& ev, sol::object data) { self.lock()->onEvent(ev, data); };
    
    namedTable.set_function("createScriptable", [](const std::string& name, sol::object parent, sol::this_state L) -> LuaProxy<ScriptableNamedObject> {
        std::shared_ptr<ScriptableNamedObject> ptr = ScriptableNamedObject::create(name, extractNamedObject(parent));
        if (ptr && !ptr->getParent()) ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
        return LuaProxy<ScriptableNamedObject>(ptr);
    });
}

} // namespace quasar::scripting
