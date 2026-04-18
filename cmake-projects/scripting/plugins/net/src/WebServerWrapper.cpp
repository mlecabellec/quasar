#include "quasar/net/WebServerWrapper.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"

namespace quasar::net {

using namespace quasar::scripting;

void LuaWSSession::onConnected() {
    std::string id_str = id().string();
    if (m_serverOwner) {
        m_serverOwner->registerSession(id_str, std::static_pointer_cast<LuaWSSession>(shared_from_this()));
    }
    if (onConnectedCb && *onConnectedCb) {
        EventTrampoline::getInstance().defer([cb = *onConnectedCb, id = std::move(id_str)]() { cb(id); });
    }
}

void LuaWSSession::onDisconnected() {
    std::string id_str = id().string();
    if (onDisconnectedCb && *onDisconnectedCb) {
        EventTrampoline::getInstance().defer([cb = *onDisconnectedCb, id = id_str]() { cb(id); });
    }
    if (m_serverOwner) {
        m_serverOwner->unregisterSession(id_str);
    }
}

void LuaWSSession::onReceivedRequest(const CppServer::HTTP::HTTPRequest& request) {
    if (onReceivedRequestCb && *onReceivedRequestCb) {
        std::string id_str = id().string();
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
        std::string id_str = id().string();
        std::string url = std::string(request.url());
        EventTrampoline::getInstance().defer([cb = *onWSConnectedCb, id = std::move(id_str), url = std::move(url)]() { cb(id, url); });
    }
}

void LuaWSSession::onWSDisconnected() {
    if (onWSDisconnectedCb && *onWSDisconnectedCb) {
        std::string id_str = id().string();
        EventTrampoline::getInstance().defer([cb = *onWSDisconnectedCb, id = std::move(id_str)]() { cb(id); });
    }
}

void LuaWSSession::onWSReceived(const void* buffer, size_t size) {
    if (onWSReceivedCb && *onWSReceivedCb) {
        std::string id_str = id().string();
        std::string data(static_cast<const char*>(buffer), size);
        EventTrampoline::getInstance().defer([cb = *onWSReceivedCb, id = std::move(id_str), data = std::move(data)]() { cb(id, data); });
    }
}

void LuaWSSession::onError(int error, const std::string& category, const std::string& message) {
    if (onErrorCb && *onErrorCb) {
        std::string id_str = id().string();
        EventTrampoline::getInstance().defer([cb = *onErrorCb, id = std::move(id_str), error, message]() { cb(id, error, message); });
    }
}

// LuaSecureWSSession implementation
void LuaSecureWSSession::onConnected() {
    std::string id_str = id().string();
    if (m_serverOwner) {
        m_serverOwner->registerSession(id_str, std::static_pointer_cast<LuaSecureWSSession>(shared_from_this()));
    }
    if (onConnectedCb && *onConnectedCb) {
        EventTrampoline::getInstance().defer([cb = *onConnectedCb, id = std::move(id_str)]() { cb(id); });
    }
}

void LuaSecureWSSession::onDisconnected() {
    std::string id_str = id().string();
    if (onDisconnectedCb && *onDisconnectedCb) {
        EventTrampoline::getInstance().defer([cb = *onDisconnectedCb, id = id_str]() { cb(id); });
    }
    if (m_serverOwner) {
        m_serverOwner->unregisterSession(id_str);
    }
}

void LuaSecureWSSession::onReceivedRequest(const CppServer::HTTP::HTTPRequest& request) {
    if (onReceivedRequestCb && *onReceivedRequestCb) {
        std::string id_str = id().string();
        std::string method = std::string(request.method());
        std::string url = std::string(request.url());
        std::string body = std::string(request.body());
        EventTrampoline::getInstance().defer([cb = *onReceivedRequestCb, id = std::move(id_str), method = std::move(method), url = std::move(url), body = std::move(body)]() {
            cb(id, method, url, body);
        });
    }
}

void LuaSecureWSSession::onWSConnected(const CppServer::HTTP::HTTPRequest& request) {
    if (onWSConnectedCb && *onWSConnectedCb) {
        std::string id_str = id().string();
        std::string url = std::string(request.url());
        EventTrampoline::getInstance().defer([cb = *onWSConnectedCb, id = std::move(id_str), url = std::move(url)]() { cb(id, url); });
    }
}

void LuaSecureWSSession::onWSDisconnected() {
    if (onWSDisconnectedCb && *onWSDisconnectedCb) {
        std::string id_str = id().string();
        EventTrampoline::getInstance().defer([cb = *onWSDisconnectedCb, id = std::move(id_str)]() { cb(id); });
    }
}

void LuaSecureWSSession::onWSReceived(const void* buffer, size_t size) {
    if (onWSReceivedCb && *onWSReceivedCb) {
        std::string id_str = id().string();
        std::string data(static_cast<const char*>(buffer), size);
        EventTrampoline::getInstance().defer([cb = *onWSReceivedCb, id = std::move(id_str), data = std::move(data)]() { cb(id, data); });
    }
}

void LuaSecureWSSession::onError(int error, const std::string& category, const std::string& message) {
    if (onErrorCb && *onErrorCb) {
        std::string id_str = id().string();
        EventTrampoline::getInstance().defer([cb = *onErrorCb, id = std::move(id_str), error, message]() { cb(id, error, message); });
    }
}

void bindWebServer(sol::state_view& lua) {
    sol::table serverTable = lua["quasar"]["net"]["server"].get_or_create<sol::table>();

    // WebSocket Server usertype (Non-secure)
    sol::usertype<LuaProxy<LuaWSServer>> utWs = lua.new_usertype<LuaProxy<LuaWSServer>>("WebServer",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    utWs["start"] = [](LuaProxy<LuaWSServer> self) { return self.lock()->Start(); };
    utWs["stop"] = [](LuaProxy<LuaWSServer> self) { return self.lock()->Stop(); };

    serverTable["WebServer"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, int port, sol::this_state L) {
            std::shared_ptr<LuaWSServer> ptr = std::make_shared<LuaWSServer>(service, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaWSServer>(ptr);
        }
    );

    // Secure WebSocket Server usertype
    sol::usertype<LuaProxy<LuaSecureWSServer>> utWss = lua.new_usertype<LuaProxy<LuaSecureWSServer>>("SecureWebServer",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    utWss["start"] = [](LuaProxy<LuaSecureWSServer> self) { return self.lock()->Start(); };
    utWss["stop"] = [](LuaProxy<LuaSecureWSServer> self) { return self.lock()->Stop(); };

    serverTable["SecureWebServer"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, const std::shared_ptr<CppServer::Asio::SSLContext>& context, int port, sol::this_state L) {
            std::shared_ptr<LuaSecureWSServer> ptr = std::make_shared<LuaSecureWSServer>(service, context, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaSecureWSServer>(ptr);
        }
    );
}

} // namespace quasar::net
