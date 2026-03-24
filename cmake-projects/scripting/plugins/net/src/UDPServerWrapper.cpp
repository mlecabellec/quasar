#include "quasar/net/UDPServerWrapper.hpp"

namespace quasar::net {

void bindUDPServer(sol::state_view& lua) {
    auto serverTable = lua["quasar"]["net"]["server"].get_or_create<sol::table>();

    serverTable.new_usertype<LuaUDPServer>("UDPServer",
        sol::factories([](const std::shared_ptr<CppServer::Asio::Service>& service, int port) {
            return std::make_shared<LuaUDPServer>(service, port);
        }),
        "start", [](LuaUDPServer& self) { return self.Start(); },
        "stop", [](LuaUDPServer& self) { return self.Stop(); },
        "restart", [](LuaUDPServer& self) { return self.Restart(); },
        "multicast", [](LuaUDPServer& self, const std::string& data) {
             return self.Multicast(data.data(), data.size());
        },
        "send", [](LuaUDPServer& self, const std::string& endpoint, int port, const std::string& data) {
             auto ep = asio::ip::udp::endpoint(asio::ip::make_address(endpoint), port);
             return self.SendAsync(ep, data.data(), data.size());
        },
        "onStarted", &LuaUDPServer::onStartedCb,
        "onStopped", &LuaUDPServer::onStoppedCb,
        "onReceived", &LuaUDPServer::onReceivedCb,
        "onError", &LuaUDPServer::onErrorCb
    );
}

} // namespace quasar::net
