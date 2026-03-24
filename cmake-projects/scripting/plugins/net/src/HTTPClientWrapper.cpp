#include "quasar/net/HTTPClientWrapper.hpp"

namespace quasar::net {

void bindHTTPClient(sol::state_view& lua) {
    auto clientTable = lua["quasar"]["net"]["client"].get_or_create<sol::table>();

    clientTable.new_usertype<LuaHTTPClient>("HTTPClient",
        sol::factories([](const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port) {
            return std::make_shared<LuaHTTPClient>(service, address, port);
        }),
        "connectAsync", [](LuaHTTPClient& self) { return self.ConnectAsync(); },
        "disconnectAsync", [](LuaHTTPClient& self) { return self.DisconnectAsync(); },
        "reconnectAsync", [](LuaHTTPClient& self) { return self.ReconnectAsync(); },
        "sendRequest", [](LuaHTTPClient& self, const std::string& method, const std::string& url, const std::string& body) {
            CppServer::HTTP::HTTPRequest req;
            req.SetBegin(method, url);
            if (!body.empty()) req.SetBody(body);
            return self.SendRequestAsync(req);
        },
        "isConnected", [](LuaHTTPClient& self) { return self.IsConnected(); },
        "onConnected", &LuaHTTPClient::onConnectedCb,
        "onDisconnected", &LuaHTTPClient::onDisconnectedCb,
        "onReceivedResponse", &LuaHTTPClient::onReceivedResponseCb,
        "onError", &LuaHTTPClient::onErrorCb
    );
}

} // namespace quasar::net
