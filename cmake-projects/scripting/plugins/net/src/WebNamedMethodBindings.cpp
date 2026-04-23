#include "quasar/named/WebNamedMethod.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include <sol/sol.hpp>

namespace quasar::net {

using namespace quasar::named;

/**
 * @brief Registers WebNamedMethod usertype and factory in Lua.
 * @param lua The Lua state.
 * @feature TSK-20260311-008 Scripted Web Methods
 * @exposed
 */
void bindWebNamedMethod(sol::state_view& lua) {
    // [CS-0030.2] Exposed interface registration.
    sol::table netTable = lua["quasar"]["net"].get_or_create<sol::table>();

    // Register WebNamedMethod as a sub-proxy.
    sol::usertype<quasar::scripting::LuaProxy<WebNamedMethod>> ut = lua.new_usertype<quasar::scripting::LuaProxy<WebNamedMethod>>("WebNamedMethod",
        sol::no_constructor,
        sol::base_classes, sol::bases<quasar::scripting::ILuaProxy>());

    // Map properties to C++ getters/setters.
    ut["httpVerb"] = sol::property(
        [](quasar::scripting::LuaProxy<WebNamedMethod>& self) { return self.lock()->getHttpVerb(); },
        [](quasar::scripting::LuaProxy<WebNamedMethod>& self, const std::string& v) { self.lock()->setHttpVerb(v); }
    );

    ut["alias"] = sol::property(
        [](quasar::scripting::LuaProxy<WebNamedMethod>& self) { return self.lock()->getAlias(); },
        [](quasar::scripting::LuaProxy<WebNamedMethod>& self, const std::string& a) { self.lock()->setAlias(a); }
    );

    ut["oasSummary"] = sol::property(
        [](quasar::scripting::LuaProxy<WebNamedMethod>& self) { return self.lock()->getOasSummary(); },
        [](quasar::scripting::LuaProxy<WebNamedMethod>& self, const std::string& s) { self.lock()->setOasSummary(s); }
    );

    // Factory method for creating Web-enabled methods from scripts.
    netTable["WebNamedMethod"] = lua.create_table_with(
        "new", [](const std::string& name, sol::function func, const std::string& verb, const std::string& alias, const std::string& summary, sol::this_state L) {
            // [CS-0010.44] Internal bridge between Lua logic and NamedMethod interface.
            quasar::named::NamedMethod::MethodType wrappedFunc = [func](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
                // Invoke Lua function with object proxies.
                sol::protected_function_result res = func(quasar::scripting::LuaProxy<NamedObject>(owner), quasar::scripting::LuaProxy<NamedObject>(args));
                // Extract return value if it represents a NamedObject.
                if (res.valid() && res.get_type() == sol::type::userdata) {
                    return res.get<quasar::scripting::LuaProxy<NamedObject>>().lock();
                }
                return nullptr;
            };

            // Create and track the underlying C++ object.
            std::shared_ptr<WebNamedMethod> ptr = WebNamedMethod::create(name, wrappedFunc, verb, alias, summary);
            quasar::scripting::ObjectTracker::getInstance().trackStrong(quasar::scripting::getEngineId(L), ptr);
            return quasar::scripting::LuaProxy<WebNamedMethod>(ptr);
        }
    );
}

} // namespace quasar::net
