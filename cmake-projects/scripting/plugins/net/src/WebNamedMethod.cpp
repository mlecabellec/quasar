#include "quasar/net/WebNamedMethod.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include <sol/sol.hpp>

namespace quasar::net {

std::shared_ptr<WebNamedMethod> WebNamedMethod::create(
    const std::string& name,
    MethodType method,
    const std::string& httpVerb,
    const std::string& alias,
    const std::string& oasSummary,
    std::shared_ptr<datacodec::ContainerDef> inputSchema,
    std::shared_ptr<datacodec::ContainerDef> outputSchema,
    std::shared_ptr<quasar::named::NamedObject> parent) 
{
    // [CS-0010.44] Factory helper for shared_ptr instantiation.
    struct Helper : public WebNamedMethod {
        Helper(const std::string& n, MethodType m, const std::string& v, const std::string& a, const std::string& oas, 
               std::shared_ptr<datacodec::ContainerDef> in, std::shared_ptr<datacodec::ContainerDef> out)
            : WebNamedMethod(n, m, v, a, oas, in, out) {}
    };

    // Instantiate and initialize weak self reference.
    std::shared_ptr<WebNamedMethod> obj = std::make_shared<Helper>(name, method, httpVerb, alias, oasSummary, inputSchema, outputSchema);
    obj->setSelf(obj);
    
    // Attach to hierarchy if parent provided.
    if (parent) {
        obj->setParent(parent);
    }
    return obj;
}

WebNamedMethod::WebNamedMethod(
    const std::string& name,
    MethodType method,
    const std::string& httpVerb,
    const std::string& alias,
    const std::string& oasSummary,
    std::shared_ptr<datacodec::ContainerDef> inputSchema,
    std::shared_ptr<datacodec::ContainerDef> outputSchema)
    : quasar::named::TypedNamedMethod(name, method, inputSchema, outputSchema),
      m_httpVerb(httpVerb), m_alias(alias), m_oasSummary(oasSummary) 
{
    // [CS-0010.32] Explicit initialization in constructor initializer list.
}

std::string WebNamedMethod::getType() const {
    return "WebNamedMethod";
}

std::string WebNamedMethod::getHttpVerb() const {
    // [CS-0010.46] Guarded access to shared string field.
    std::lock_guard<std::recursive_timed_mutex> lock(m_mutex);
    return m_httpVerb;
}

void WebNamedMethod::setHttpVerb(const std::string& verb) {
    // [CS-0010.46] Guarded modification with structural tracking.
    std::lock_guard<std::recursive_timed_mutex> lock(m_mutex);
    m_httpVerb = verb;
    incrementTreeVersion();
    notifyObservers(getSelf());
}

std::string WebNamedMethod::getAlias() const {
    // [CS-0010.46] Guarded access to URI alias.
    std::lock_guard<std::recursive_timed_mutex> lock(m_mutex);
    return m_alias;
}

void WebNamedMethod::setAlias(const std::string& alias) {
    // [CS-0010.46] Guarded modification of URI alias.
    std::lock_guard<std::recursive_timed_mutex> lock(m_mutex);
    m_alias = alias;
    incrementTreeVersion();
    notifyObservers(getSelf());
}

std::string WebNamedMethod::getOasSummary() const {
    // [CS-0010.46] Guarded access to OpenAPI summary.
    std::lock_guard<std::recursive_timed_mutex> lock(m_mutex);
    return m_oasSummary;
}

void WebNamedMethod::setOasSummary(const std::string& summary) {
    // [CS-0010.46] Guarded modification of documentation metadata.
    std::lock_guard<std::recursive_timed_mutex> lock(m_mutex);
    m_oasSummary = summary;
    incrementTreeVersion();
    notifyObservers(getSelf());
}

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
            quasar::named::NamedMethod::MethodType wrappedFunc = [func](std::shared_ptr<quasar::named::NamedObject> owner, std::shared_ptr<quasar::named::NamedObject> args) -> std::shared_ptr<quasar::named::NamedObject> {
                // Invoke Lua function with object proxies.
                sol::protected_function_result res = func(quasar::scripting::LuaProxy<quasar::named::NamedObject>(owner), quasar::scripting::LuaProxy<quasar::named::NamedObject>(args));
                // Extract return value if it represents a NamedObject.
                if (res.valid() && res.get_type() == sol::type::userdata) {
                    return res.get<quasar::scripting::LuaProxy<quasar::named::NamedObject>>().lock();
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
