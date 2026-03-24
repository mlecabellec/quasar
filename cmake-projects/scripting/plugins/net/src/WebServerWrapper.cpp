#include "quasar/net/WebServerWrapper.hpp"

namespace quasar::net {

void LuaWSSession::onConnected() {
    auto id_str = id().string();
    if (m_serverOwner) m_serverOwner->registerSession(id_str, std::static_pointer_cast<LuaWSSession>(shared_from_this()));
    if (onConnectedCb && *onConnectedCb) {
        EventTrampoline::getInstance().defer([cb = *onConnectedCb, id = std::move(id_str)]() { cb(id); });
    }
}

void LuaWSSession::onDisconnected() {
    auto id_str = id().string();
    if (onDisconnectedCb && *onDisconnectedCb) {
        EventTrampoline::getInstance().defer([cb = *onDisconnectedCb, id = id_str]() { cb(id); });
    }
    if (m_serverOwner) m_serverOwner->unregisterSession(id_str);
}

void LuaWSSession::onReceivedRequest(const CppServer::HTTP::HTTPRequest& request) {
    if (onReceivedRequestCb && *onReceivedRequestCb) {
        auto id_str = id().string();
        std::string method = std::string(request.method());
        std::string url = std::string(request.url());
        std::string body = std::string(request.body());
        EventTrampoline::getInstance().defer([cb = *onReceivedRequestCb, id = std::move(id_str), method = std::move(method), url = std::move(url), body = std::move(body)]() {
            cb(id, method, url, body);
        });
    }
}

void LuaWSSession::onWSConnected(const CppServer::HTTP::HTTPRequest& request) {
    if (onWSConnectedCb && *onWSConnectedCb) {
        auto id_str = id().string();
        std::string url = std::string(request.url());
        EventTrampoline::getInstance().defer([cb = *onWSConnectedCb, id = std::move(id_str), url = std::move(url)]() { cb(id, url); });
    }
}

void LuaWSSession::onWSDisconnected() {
    if (onWSDisconnectedCb && *onWSDisconnectedCb) {
        auto id_str = id().string();
        EventTrampoline::getInstance().defer([cb = *onWSDisconnectedCb, id = std::move(id_str)]() { cb(id); });
    }
}

void LuaWSSession::onWSReceived(const void* buffer, size_t size) {
    if (onWSReceivedCb && *onWSReceivedCb) {
        auto id_str = id().string();
        std::string data(static_cast<const char*>(buffer), size);
        EventTrampoline::getInstance().defer([cb = *onWSReceivedCb, id = std::move(id_str), data = std::move(data)]() { cb(id, data); });
    }
}

void LuaWSSession::onError(int error, const std::string& category, const std::string& message) {
    if (onErrorCb && *onErrorCb) {
        auto id_str = id().string();
        EventTrampoline::getInstance().defer([cb = *onErrorCb, id = std::move(id_str), error, message]() { cb(id, error, message); });
    }
}

void bindWebServer(sol::state_view& lua) {
    auto serverTable = lua["quasar"]["net"]["server"].get_or_create<sol::table>();

    serverTable.new_usertype<LuaWSServer>("WebServer",
        sol::factories([](const std::shared_ptr<CppServer::Asio::Service>& service, int port) {
            return std::make_shared<LuaWSServer>(service, port);
        }),
        "start", [](LuaWSServer& self) { return self.Start(); },
        "stop", [](LuaWSServer& self) { return self.Stop(); },
        "restart", [](LuaWSServer& self) { return self.Restart(); },
        "multicastText", [](LuaWSServer& self, const std::string& data) { return self.MulticastText(data.data(), data.size()); },
        "multicastBinary", [](LuaWSServer& self, const std::string& data) { return self.MulticastBinary(data.data(), data.size()); },
        "sendText", [](LuaWSServer& self, const std::string& id, const std::string& data) {
             auto session = self.getLuaSession(id);
             if (session) session->SendTextAsync(data.data(), data.size());
        },
        "sendBinary", [](LuaWSServer& self, const std::string& id, const std::string& data) {
             auto session = self.getLuaSession(id);
             if (session) session->SendBinaryAsync(data.data(), data.size());
        },
        "sendResponse", [](LuaWSServer& self, const std::string& id, int status, const std::string& message, const std::string& body) {
             auto session = self.getLuaSession(id);
             if (session) {
                 CppServer::HTTP::HTTPResponse response;
                 response.SetBegin(status, message);
                 response.SetBody(body);
                 session->SendResponseAsync(response);
             }
        },
        "onConnected", &LuaWSServer::onConnectedCb,
        "onDisconnected", &LuaWSServer::onDisconnectedCb,
        "onReceivedRequest", &LuaWSServer::onReceivedRequestCb,
        "onWSConnected", &LuaWSServer::onWSConnectedCb,
        "onWSDisconnected", &LuaWSServer::onWSDisconnectedCb,
        "onWSReceived", &LuaWSServer::onWSReceivedCb,
        "onError", &LuaWSServer::onErrorCb
    );
}

} // namespace quasar::net
