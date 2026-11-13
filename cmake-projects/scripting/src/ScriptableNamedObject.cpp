#include "quasar/scripting/ScriptableNamedObject.hpp"
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"

namespace quasar::scripting {

ScriptableNamedObject::~ScriptableNamedObject() noexcept {
    // [CS-0060.5] Ensure no exception escapes during Lua table detachment.
    invalidate();
}

void ScriptableNamedObject::setLuaSelf(sol::table luaSelf) {
    // [CS-0010.44] Store the Lua override table and bind its lifetime to the owning engine.
    m_luaSelf = luaSelf;
    if (m_luaSelf.valid() && m_luaSelf.lua_state() != nullptr) {
        sol::state_view lua(m_luaSelf.lua_state());
        sol::object engineObj = lua["__quasar_engine"];
        if (engineObj.is<std::weak_ptr<LuaEngine>>()) {
            std::weak_ptr<LuaEngine> weakEngine = engineObj.as<std::weak_ptr<LuaEngine>>();
            m_engine = weakEngine;
            if (std::shared_ptr<LuaEngine> engine = weakEngine.lock()) {
                m_engineId = engine->getId();
                std::shared_ptr<ScriptableNamedObject> selfPtr = std::dynamic_pointer_cast<ScriptableNamedObject>(getSelf());
                if (selfPtr) {
                    ObjectTracker::getInstance().trackScriptable(selfPtr);
                }
            }
        }
    }
}

void ScriptableNamedObject::invalidate() noexcept {
    // [CS-0010.20][CS-0060.5] Reset the Lua table reference safely while the engine state is valid.
    try {
        m_luaSelf = sol::nil;
    } catch (...) {
        // [CS-0060.5] Prevent exception propagation out of teardown paths.
    }
}

std::shared_ptr<ScriptableNamedObject> ScriptableNamedObject::create(const std::string& name, std::shared_ptr<quasar::named::NamedObject> parent) {
    struct Enabler : public ScriptableNamedObject {
        explicit Enabler(const std::string& n) : ScriptableNamedObject(n) {}
    };

    std::shared_ptr<ScriptableNamedObject> obj = std::make_shared<Enabler>(name);
    obj->setSelf(obj);
    if (parent) {
        obj->setParent(parent);
    }
    return obj;
}

void ScriptableNamedObject::addChild(std::shared_ptr<named::NamedObject> child) {
    if (getLuaSelf() && getLuaSelf()["onAddChild"].valid()) {
        getLuaSelf()["onAddChild"](getLuaSelf(), LuaProxy<named::NamedObject>(child));
    }
    named::NamedObject::addChild(child);
}

} // namespace quasar::scripting
