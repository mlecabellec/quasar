#include "quasar/net/TCPServerWrapper.hpp"

namespace quasar::net {

void bindTCPServer(sol::state_view& lua) {
    auto serverTable = lua["quasar"]["net"]["server"].get_or_create<sol::table>();

    // Since TCPServer needs an ASIO Service running, we expose a simple Service manager
    serverTable.new_usertype<CppServer::Asio::Service>("AsioService",
        sol::factories([](){ return std::make_shared<CppServer::Asio::Service>(); }),
        "start", [](std::shared_ptr<CppServer::Asio::Service> self) { return self->Start(); },
        "stop", [](std::shared_ptr<CppServer::Asio::Service> self) { return self->Stop(); },
        "restart", [](std::shared_ptr<CppServer::Asio::Service> self) { return self->Restart(); }
    );

    serverTable.new_usertype<LuaTCPServer>("TCPServer",
        sol::factories([](const std::shared_ptr<CppServer::Asio::Service>& service, int port) {
            return std::make_shared<LuaTCPServer>(service, port);
        }),
        "start", [](std::shared_ptr<LuaTCPServer> self) { return self->Start(); },
        "stop", [](std::shared_ptr<LuaTCPServer> self) { return self->Stop(); },
        "restart", [](std::shared_ptr<LuaTCPServer> self) { return self->Restart(); },
        "disconnectAll", [](std::shared_ptr<LuaTCPServer> self) { return self->DisconnectAll(); },
        "disconnect", [](std::shared_ptr<LuaTCPServer> self, const std::string& id_str) {
            try {
                CppCommon::UUID id(id_str);
                auto session = self->FindSession(id);
                if (session) {
                    return session->Disconnect();
                }
                return false;
            } catch (...) { return false; }
        },
        "sendAsync", [](std::shared_ptr<LuaTCPServer> self, const std::string& id_str, const std::string& data) {
            try {
                CppCommon::UUID id(id_str);
                auto session = self->FindSession(id);
                if (session) {
                    return session->SendAsync(data.data(), data.size());
                }
                return false;
            } catch (...) { return false; }
        },
        // Multicast
        "multicast", [](std::shared_ptr<LuaTCPServer> self, const std::string& data) {
            return self->Multicast(data.data(), data.size());
        },
        "onConnected", &LuaTCPServer::onConnectedCb,
        "onDisconnected", &LuaTCPServer::onDisconnectedCb,
        "onReceived", &LuaTCPServer::onReceivedCb,
        "onError", &LuaTCPServer::onErrorCb
    );
}

} // namespace quasar::net
