#pragma once

#include "quasar/named/NamedObject.hpp"
#include <sol/sol.hpp>
#include <memory>
#include <string>

namespace quasar::scripting {

/**
 * @brief A NamedObject that can have its methods overridden/hooked by Lua.
 * 
 * ScriptableNamedObject allows a Lua table to "take over" certain virtual methods.
 * This fulfills the requirement for extendable C++ classes with Lua hooks.
 */
class ScriptableNamedObject : public named::NamedObject {
public:
    /**
     * @brief Factory method.
     */
    static std::shared_ptr<ScriptableNamedObject> create(const std::string& name, std::shared_ptr<named::NamedObject> parent = nullptr);

    /**
     * @brief Sets the Lua table that provides overrides for this object.
     */
    void setLuaSelf(sol::table luaSelf) { m_luaSelf = luaSelf; }
    
    /**
     * @brief Gets the associated Lua table.
     */
    sol::table getLuaSelf() const { return m_luaSelf; }

    // Overridden virtual methods from NamedObject
    
    std::string getType() const override {
        if (m_luaSelf && m_luaSelf["getType"].valid()) {
            sol::protected_function func = m_luaSelf["getType"];
            auto result = func(m_luaSelf);
            if (result.valid()) return result;
        }
        return "ScriptableNamedObject";
    }

    std::shared_ptr<named::NamedObject> clone() const override {
        if (m_luaSelf && m_luaSelf["clone"].valid()) {
            sol::protected_function func = m_luaSelf["clone"];
            auto result = func(m_luaSelf);
            if (result.valid()) return result;
        }
        return named::NamedObject::clone();
    }

    /**
     * @brief Generic event hook for Lua.
     */
    virtual void onEvent(const std::string& eventName, sol::object data) {
        if (m_luaSelf && m_luaSelf["onEvent"].valid()) {
            m_luaSelf["onEvent"](m_luaSelf, eventName, data);
        }
    }

protected:
    ScriptableNamedObject(const std::string& name) : named::NamedObject(name) {}

    void addChild(std::shared_ptr<named::NamedObject> child) override {
        if (m_luaSelf && m_luaSelf["onAddChild"].valid()) {
            m_luaSelf["onAddChild"](m_luaSelf, child);
        }
        named::NamedObject::addChild(child);
    }

private:
    sol::table m_luaSelf;
};

} // namespace quasar::scripting
