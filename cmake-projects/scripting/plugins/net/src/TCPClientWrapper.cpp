#include "quasar/net/TCPClientWrapper.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"

namespace quasar::net {

using namespace quasar::scripting;

void bindTCPClient(sol::state_view& lua) {
    auto clientTable = lua["quasar"]["net"]["client"].get_or_create<sol::table>();

    sol::usertype<LuaProxy<LuaTCPClient>> ut = lua.new_usertype<LuaProxy<LuaTCPClient>>("TCPClient",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    clientTable["TCPClient"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port, sol::this_state L) {
            auto ptr = std::make_shared<LuaTCPClient>(service, address, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaTCPClient>(ptr);
        }
    );

    ut["connectAsync"] = [](LuaProxy<LuaTCPClient> self) { return self.lock()->ConnectAsync(); };
    ut["disconnectAsync"] = [](LuaProxy<LuaTCPClient> self) { return self.lock()->DisconnectAsync(); };
    ut["reconnectAsync"] = [](LuaProxy<LuaTCPClient> self) { return self.lock()->ReconnectAsync(); };
    ut["sendAsync"] = [](LuaProxy<LuaTCPClient> self, const std::string& data) { 
        return self.lock()->SendAsync(data.data(), data.size()); 
    };
    ut["isConnected"] = [](LuaProxy<LuaTCPClient> self) { return self.lock()->IsConnected(); };
    
    // Callbacks
    ut["onConnected"] = sol::property(
        [](LuaProxy<LuaTCPClient>& self) { return self.lock()->onConnectedCb; },
        [](LuaProxy<LuaTCPClient>& self, sol::function cb) { self.lock()->onConnectedCb = cb; }
    );
    ut["onDisconnected"] = sol::property(
        [](LuaProxy<LuaTCPClient>& self) { return self.lock()->onDisconnectedCb; },
        [](LuaProxy<LuaTCPClient>& self, sol::function cb) { self.lock()->onDisconnectedCb = cb; }
    );
    ut["onReceived"] = sol::property(
        [](LuaProxy<LuaTCPClient>& self) { return self.lock()->onReceivedCb; },
        [](LuaProxy<LuaTCPClient>& self, sol::function cb) { self.lock()->onReceivedCb = cb; }
    );
    ut["onError"] = sol::property(
        [](LuaProxy<LuaTCPClient>& self) { return self.lock()->onErrorCb; },
        [](LuaProxy<LuaTCPClient>& self, sol::function cb) { self.lock()->onErrorCb = cb; }
    );
}

} // namespace quasar::net
