#include "quasar/scripting/PluginContract.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/opcua/OpcUaServerService.hpp"

#include "quasar/opcua/OpcUaClientService.hpp"
#include <sol/sol.hpp>

using namespace quasar::scripting;
using namespace quasar::named;
using namespace quasar::opcua;

extern "C" QUASAR_PLUGIN_EXPORT void registerPluginComponents(sol::state_view lua) {
    sol::table quasarTable = lua["quasar"].get_or_create<sol::table>();
    sol::table opcuaTable = quasarTable["opcua"].get_or_create<sol::table>();

    // OpcUaServerService binding
    sol::usertype<LuaProxy<OpcUaServerService>> utServer = lua.new_usertype<LuaProxy<OpcUaServerService>>("OpcUaServerService", sol::no_constructor);
    // Bind base NamedService methods (should ideally be done via sol2 base class support, but LuaProxy uses lockdown)
    utServer["start"] = [](LuaProxy<OpcUaServerService> self) { self.lock()->start(); };
    utServer["stop"] = [](LuaProxy<OpcUaServerService> self) { self.lock()->stop(); };
    utServer["setPort"] = [](LuaProxy<OpcUaServerService> self, uint16_t port) { self.lock()->setPort(port); };
    utServer["setRootObject"] = [](LuaProxy<OpcUaServerService> self, sol::object root) { 
        self.lock()->setRootObject(extractNamedObject(root)); 
    };

    opcuaTable["createServer"] = [](const std::string& name, sol::object parent) {
        auto ptr = OpcUaServerService::create(name, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<OpcUaServerService>(ptr);
    };

    // OpcUaClientService binding
    sol::usertype<LuaProxy<OpcUaClientService>> utClient = lua.new_usertype<LuaProxy<OpcUaClientService>>("OpcUaClientService", sol::no_constructor);
    utClient["start"] = [](LuaProxy<OpcUaClientService> self) { self.lock()->start(); };
    utClient["stop"] = [](LuaProxy<OpcUaClientService> self) { self.lock()->stop(); };
    utClient["setUrl"] = [](LuaProxy<OpcUaClientService> self, const std::string& url) { self.lock()->setUrl(url); };

    opcuaTable["createClient"] = [](const std::string& name, sol::object parent) {
        auto ptr = OpcUaClientService::create(name, extractNamedObject(parent));
        if (!ptr->getParent()) ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<OpcUaClientService>(ptr);
    };
}
