#include "quasar/scripting/PluginContract.hpp"
#include <sol/sol.hpp>
#include "quasar/net/EventTrampoline.hpp"
#include "quasar/net/TCPServerWrapper.hpp"
#include "quasar/net/UDPServerWrapper.hpp"
#include "quasar/net/WebServerWrapper.hpp"
#include "quasar/net/TCPClientWrapper.hpp"
#include "quasar/net/HTTPClientWrapper.hpp"
#include "quasar/net/SSLContextWrapper.hpp"

namespace quasar::net {

extern "C" QUASAR_PLUGIN_EXPORT void registerPluginComponents(sol::state_view lua) {
    auto netTable = lua["quasar"]["net"].get_or_create<sol::table>();
    netTable.set_function("poll", []() {
        EventTrampoline::getInstance().poll();
    });

    // TSK-20260311-002: Server namespace
    auto serverTable = lua["quasar"]["net"]["server"].get_or_create<sol::table>();
    bindTCPServer(lua);
    bindUDPServer(lua);
    bindWebServer(lua);

    // TSK-20260311-003: Client namespace
    auto clientTable = lua["quasar"]["net"]["client"].get_or_create<sol::table>();
    bindTCPClient(lua);
    bindHTTPClient(lua);

    // TSK-20260311-002: Security namespace
    bindSSLContext(lua);

    // TODO: Register actual components
}

} // namespace quasar::net
