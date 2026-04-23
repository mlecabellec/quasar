#include "quasar/scripting/PluginContract.hpp"
#include <sol/sol.hpp>
#include "quasar/net/EventTrampoline.hpp"
#include "quasar/net/TCPServerWrapper.hpp"
#include "quasar/net/UDPServerWrapper.hpp"
#include "quasar/net/WebServerWrapper.hpp"
#include "quasar/net/TCPClientWrapper.hpp"
#include "quasar/net/UDPClientWrapper.hpp"
#include "quasar/net/HTTPClientWrapper.hpp"
#include "quasar/net/WSClientWrapper.hpp"
#include "quasar/net/SSLContextWrapper.hpp"
#include "quasar/named/WebNamedMethod.hpp"

namespace quasar::net {

// [CS-0010.44] Forward declarations for Lua bindings.
void bindWebNamedMethod(sol::state_view& lua);

extern "C" QUASAR_PLUGIN_EXPORT void registerPluginComponents(sol::state_view lua) {
    // Ensure intermediate tables exist before creating sub-tables.
    // sol2 does NOT auto-create intermediate tables via [] — get_or_create is needed at each level.
    sol::table quasarTable  = lua["quasar"].get_or_create<sol::table>();
    sol::table netTable     = quasarTable["net"].get_or_create<sol::table>();

    // TSK-20260311-002: Server namespace
    netTable["server"].get_or_create<sol::table>();
    bindTCPServer(lua);
    bindUDPServer(lua);
    bindWebServer(lua);

    // TSK-20260311-003: Client namespace
    netTable["client"].get_or_create<sol::table>();
    bindTCPClient(lua);
    bindUDPClient(lua);
    bindHTTPClient(lua);
    bindWSClient(lua);

    // TSK-20260311-002: Security namespace
    bindSSLContext(lua);
    
    // TSK-20260311-008: Web API Routing
    bindWebNamedMethod(lua);

    // Register poll LAST so sub-table construction cannot overwrite it.
    netTable = quasarTable["net"].get_or_create<sol::table>();
    netTable.set_function("poll", []() {
        std::cout << "[CPP Debug] net.poll() called" << std::endl;
        EventTrampoline::getInstance().poll();
    });
}

} // namespace quasar::net
