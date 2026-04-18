#include "quasar/net/UDPServerWrapper.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"

namespace quasar::net {

using namespace quasar::scripting;

void bindUDPServer(sol::state_view& lua) {
    auto serverTable = lua["quasar"]["net"]["server"].get_or_create<sol::table>();

    sol::usertype<LuaProxy<LuaUDPServer>> ut = lua.new_usertype<LuaProxy<LuaUDPServer>>("UDPServer",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    serverTable["UDPServer"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, int port, sol::this_state L) {
            auto ptr = std::make_shared<LuaUDPServer>(service, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaUDPServer>(ptr);
        }
    );

    ut["start"] = [](LuaProxy<LuaUDPServer> self) { return self.lock()->Start(); };
    ut["stop"] = [](LuaProxy<LuaUDPServer> self) { return self.lock()->Stop(); };
    ut["restart"] = [](LuaProxy<LuaUDPServer> self) { return self.lock()->Restart(); };
    ut["multicast"] = [](LuaProxy<LuaUDPServer> self, const std::string& data) {
         return self.lock()->Multicast(data.data(), data.size());
    };
    ut["send"] = [](LuaProxy<LuaUDPServer> self, const std::string& endpoint, int port, const std::string& data) {
         auto ep = asio::ip::udp::endpoint(asio::ip::make_address(endpoint), port);
         return self.lock()->SendAsync(ep, data.data(), data.size());
    };
    
    // Callbacks
    ut["onStarted"] = sol::property(
        [](LuaProxy<LuaUDPServer>& self) { return self.lock()->onStartedCb; },
        [](LuaProxy<LuaUDPServer>& self, sol::function cb) { self.lock()->onStartedCb = cb; }
    );
    ut["onStopped"] = sol::property(
        [](LuaProxy<LuaUDPServer>& self) { return self.lock()->onStoppedCb; },
        [](LuaProxy<LuaUDPServer>& self, sol::function cb) { self.lock()->onStoppedCb = cb; }
    );
    ut["onReceived"] = sol::property(
        [](LuaProxy<LuaUDPServer>& self) { return self.lock()->onReceivedCb; },
        [](LuaProxy<LuaUDPServer>& self, sol::function cb) { self.lock()->onReceivedCb = cb; }
    );
    ut["onError"] = sol::property(
        [](LuaProxy<LuaUDPServer>& self) { return self.lock()->onErrorCb; },
        [](LuaProxy<LuaUDPServer>& self, sol::function cb) { self.lock()->onErrorCb = cb; }
    );
}

} // namespace quasar::net
