#include "quasar/net/TCPServerWrapper.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"

namespace quasar::net {

using namespace quasar::scripting;

void bindTCPServer(sol::state_view& lua) {
    sol::table serverTable = lua["quasar"]["net"]["server"].get_or_create<sol::table>();

    // Since TCPServer needs an ASIO Service running, we expose a simple Service manager
    serverTable.new_usertype<CppServer::Asio::Service>("AsioService",
        sol::factories([](){ return std::make_shared<CppServer::Asio::Service>(); }),
        "start", [](std::shared_ptr<CppServer::Asio::Service> self) { return self->Start(); },
        "stop", [](std::shared_ptr<CppServer::Asio::Service> self) { return self->Stop(); },
        "restart", [](std::shared_ptr<CppServer::Asio::Service> self) { return self->Restart(); }
    );

    sol::usertype<LuaProxy<LuaTCPServer>> ut = lua.new_usertype<LuaProxy<LuaTCPServer>>("TCPServer",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    serverTable["TCPServer"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, int port, sol::this_state L) {
            std::shared_ptr<LuaTCPServer> ptr = std::make_shared<LuaTCPServer>(service, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaTCPServer>(ptr);
        }
    );

    ut["start"] = [](LuaProxy<LuaTCPServer> self) { return self.lock()->Start(); };
    ut["stop"] = [](LuaProxy<LuaTCPServer> self) { return self.lock()->Stop(); };
    ut["restart"] = [](LuaProxy<LuaTCPServer> self) { return self.lock()->Restart(); };
    ut["disconnectAll"] = [](LuaProxy<LuaTCPServer> self) { return self.lock()->DisconnectAll(); };
    ut["disconnect"] = [](LuaProxy<LuaTCPServer> self, const std::string& id_str) {
        try {
            CppCommon::UUID id(id_str);
            std::shared_ptr<CppServer::Asio::TCPSession> session = self.lock()->FindSession(id);
            if (session) {
                return session->Disconnect();
            }
            return false;
        } catch (...) { return false; }
    };
    ut["sendAsync"] = [](LuaProxy<LuaTCPServer> self, const std::string& id_str, const std::string& data) {
        try {
            CppCommon::UUID id(id_str);
            std::shared_ptr<CppServer::Asio::TCPSession> session = self.lock()->FindSession(id);
            if (session) {
                return session->SendAsync(data.data(), data.size());
            }
            return false;
        } catch (...) { return false; }
    };
    ut["multicast"] = [](LuaProxy<LuaTCPServer> self, const std::string& data) {
        return self.lock()->Multicast(data.data(), data.size());
    };
    
    // Callbacks
    ut["onConnected"] = sol::property(
        [](LuaProxy<LuaTCPServer>& self) { return self.lock()->onConnectedCb; },
        [](LuaProxy<LuaTCPServer>& self, sol::function cb) { self.lock()->onConnectedCb = cb; }
    );
    ut["onDisconnected"] = sol::property(
        [](LuaProxy<LuaTCPServer>& self) { return self.lock()->onDisconnectedCb; },
        [](LuaProxy<LuaTCPServer>& self, sol::function cb) { self.lock()->onDisconnectedCb = cb; }
    );
    ut["onReceived"] = sol::property(
        [](LuaProxy<LuaTCPServer>& self) { return self.lock()->onReceivedCb; },
        [](LuaProxy<LuaTCPServer>& self, sol::function cb) { self.lock()->onReceivedCb = cb; }
    );
    ut["onError"] = sol::property(
        [](LuaProxy<LuaTCPServer>& self) { return self.lock()->onErrorCb; },
        [](LuaProxy<LuaTCPServer>& self, sol::function cb) { self.lock()->onErrorCb = cb; }
    );
}

} // namespace quasar::net
