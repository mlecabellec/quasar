#include "quasar/net/UDPClientWrapper.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"

namespace quasar::net {

using namespace quasar::scripting;

void bindUDPClient(sol::state_view& lua) {
    sol::table clientTable = lua["quasar"]["net"]["client"].get_or_create<sol::table>();

    sol::usertype<LuaProxy<LuaUDPClient>> ut = lua.new_usertype<LuaProxy<LuaUDPClient>>("UDPClient",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    clientTable["UDPClient"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port, sol::this_state L) {
            std::shared_ptr<LuaUDPClient> ptr = std::make_shared<LuaUDPClient>(service, address, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaUDPClient>(ptr);
        }
    );

    ut["connectAsync"] = [](LuaProxy<LuaUDPClient> self) { return self.lock()->ConnectAsync(); };
    ut["disconnectAsync"] = [](LuaProxy<LuaUDPClient> self) { return self.lock()->DisconnectAsync(); };
    ut["reconnectAsync"] = [](LuaProxy<LuaUDPClient> self) { return self.lock()->ReconnectAsync(); };
    ut["send"] = [](LuaProxy<LuaUDPClient> self, const std::string& data) {
         return self.lock()->SendAsync(data.data(), data.size());
    };
    ut["sendTo"] = [](LuaProxy<LuaUDPClient> self, const std::string& endpoint, int port, const std::string& data) {
         asio::ip::udp::endpoint ep = asio::ip::udp::endpoint(asio::ip::make_address(endpoint), port);
         return self.lock()->SendAsync(ep, data.data(), data.size());
    };
    ut["joinMulticastGroup"] = [](LuaProxy<LuaUDPClient> self, const std::string& address) { self.lock()->JoinMulticastGroupAsync(address); };
    ut["leaveMulticastGroup"] = [](LuaProxy<LuaUDPClient> self, const std::string& address) { self.lock()->LeaveMulticastGroupAsync(address); };
    ut["setupMulticast"] = [](LuaProxy<LuaUDPClient> self, bool enable) { self.lock()->SetupMulticast(enable); };
    
    // Callbacks
    ut["onConnected"] = sol::property(
        [](LuaProxy<LuaUDPClient>& self) { return self.lock()->onConnectedCb; },
        [](LuaProxy<LuaUDPClient>& self, sol::function cb) { self.lock()->onConnectedCb = cb; }
    );
    ut["onDisconnected"] = sol::property(
        [](LuaProxy<LuaUDPClient>& self) { return self.lock()->onDisconnectedCb; },
        [](LuaProxy<LuaUDPClient>& self, sol::function cb) { self.lock()->onDisconnectedCb = cb; }
    );
    ut["onReceived"] = sol::property(
        [](LuaProxy<LuaUDPClient>& self) { return self.lock()->onReceivedCb; },
        [](LuaProxy<LuaUDPClient>& self, sol::function cb) { self.lock()->onReceivedCb = cb; }
    );
    ut["onError"] = sol::property(
        [](LuaProxy<LuaUDPClient>& self) { return self.lock()->onErrorCb; },
        [](LuaProxy<LuaUDPClient>& self, sol::function cb) { self.lock()->onErrorCb = cb; }
    );
    ut["onJoinedMulticastGroup"] = sol::property(
        [](LuaProxy<LuaUDPClient>& self) { return self.lock()->onJoinedMulticastGroupCb; },
        [](LuaProxy<LuaUDPClient>& self, sol::function cb) { self.lock()->onJoinedMulticastGroupCb = cb; }
    );
    ut["onLeftMulticastGroup"] = sol::property(
        [](LuaProxy<LuaUDPClient>& self) { return self.lock()->onLeftMulticastGroupCb; },
        [](LuaProxy<LuaUDPClient>& self, sol::function cb) { self.lock()->onLeftMulticastGroupCb = cb; }
    );
}

} // namespace quasar::net
