#include "quasar/net/TCPClientWrapper.hpp"

namespace quasar::net {

void bindTCPClient(sol::state_view& lua) {
    auto clientTable = lua["quasar"]["net"]["client"].get_or_create<sol::table>();

    clientTable.new_usertype<LuaTCPClient>("TCPClient",
        sol::factories([](const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port) {
            return std::make_shared<LuaTCPClient>(service, address, port);
        }),
        "connectAsync", [](LuaTCPClient& self) { return self.ConnectAsync(); },
        "disconnectAsync", [](LuaTCPClient& self) { return self.DisconnectAsync(); },
        "reconnectAsync", [](std::shared_ptr<LuaTCPClient> self) { return self->ReconnectAsync(); },
        "sendAsync", [](std::shared_ptr<LuaTCPClient> self, const std::string& data) { return self->SendAsync(data.data(), data.size()); },
        "isConnected", [](std::shared_ptr<LuaTCPClient> self) { return self->IsConnected(); },
        "onConnected", &LuaTCPClient::onConnectedCb,
        "onDisconnected", &LuaTCPClient::onDisconnectedCb,
        "onReceived", &LuaTCPClient::onReceivedCb,
        "onError", &LuaTCPClient::onErrorCb
    );
}

} // namespace quasar::net
