#include "quasar/net/WebServerWrapper.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"

namespace quasar::net {

using namespace quasar::scripting;

/**
 * @brief Helper to convert std::expected to a sol::object for Lua.
 */
template <typename T, typename E>
sol::object to_sol_object(const std::expected<T, E>& res, sol::state_view lua) {
    if (res.has_value()) {
        if constexpr (std::is_same_v<T, void>) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, res.value());
        }
    }
    return sol::make_object(lua, res.error());
}

// LuaWSSession Implementation
LuaWSSession::LuaWSSession(const std::shared_ptr<CppServer::WS::WSServer>& server) : CppServer::WS::WSSession(server) {}

void LuaWSSession::onWSConnected(const CppServer::HTTP::HTTPRequest& request) {
    (void)request;
    if (m_serverOwner) {
        // Can't use shared_from_this() in constructor or immediately after without careful casting, 
        // but here we are in a callback, so it's already managed by a shared_ptr.
        m_serverOwner->registerSession(id().string(), std::static_pointer_cast<LuaWSSession>(shared_from_this()));
        m_serverOwner->notifyConnected(id().string());
    }
}

void LuaWSSession::onWSDisconnected() {
    if (m_serverOwner) {
        m_serverOwner->notifyDisconnected(id().string());
        m_serverOwner->unregisterSession(id().string());
    }
}

void LuaWSSession::onWSReceived(const void* buffer, size_t size) {
    if (m_serverOwner) {
        std::string data(static_cast<const char*>(buffer), size);
        m_serverOwner->notifyReceived(id().string(), data);
    }
}

// LuaWSServer Implementation
LuaWSServer::LuaWSServer(const std::shared_ptr<CppServer::Asio::Service>& service, int port) {
    m_server = std::make_shared<InternalServer>(service, port);
}

std::expected<void, std::string> LuaWSServer::startAsync() {
    if (m_server->Start()) {
        return {};
    }
    return std::unexpected("Failed to start WebServer");
}

std::expected<void, std::string> LuaWSServer::stopAsync() {
    if (m_server->Stop()) {
        return {};
    }
    return std::unexpected("Failed to stop WebServer");
}

bool LuaWSServer::broadcastText(const std::string& text) {
    return m_server->MulticastText(text);
}

bool LuaWSServer::sendText(const std::string& id, const std::string& text) {
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    auto it = m_sessions.find(id);
    if (it != m_sessions.end()) {
        return it->second->SendTextAsync(text);
    }
    return false;
}

void LuaWSServer::registerSession(const std::string& id, std::shared_ptr<LuaWSSession> session) {
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    m_sessions[id] = session;
}

void LuaWSServer::unregisterSession(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    m_sessions.erase(id);
}

void LuaWSServer::notifyConnected(const std::string& id) {
    if (onWSConnected) {
        EventTrampoline::getInstance().defer([cb = onWSConnected, i = id]() { cb(i); });
    }
}

void LuaWSServer::notifyDisconnected(const std::string& id) {
    if (onDisconnected) {
        EventTrampoline::getInstance().defer([cb = onDisconnected, i = id]() { cb(i); });
    }
}

void LuaWSServer::notifyReceived(const std::string& id, const std::string& data) {
    if (onWSReceived) {
        EventTrampoline::getInstance().defer([cb = onWSReceived, i = id, d = data]() { cb(i, d); });
    }
}

std::shared_ptr<CppServer::Asio::TCPSession> LuaWSServer::InternalServer::CreateSession(const std::shared_ptr<CppServer::Asio::TCPServer>& server) {
    auto session = std::make_shared<LuaWSSession>(std::static_pointer_cast<CppServer::WS::WSServer>(server));
    auto o = owner.lock();
    if (o) {
        session->m_serverOwner = o.get();
    }
    return session;
}

// LuaSecureWSSession Implementation
LuaSecureWSSession::LuaSecureWSSession(const std::shared_ptr<CppServer::WS::WSSServer>& server) : CppServer::WS::WSSSession(server) {}

void LuaSecureWSSession::onWSConnected(const CppServer::HTTP::HTTPRequest& request) {
    (void)request;
    if (m_serverOwner) {
        m_serverOwner->registerSession(id().string(), std::static_pointer_cast<LuaSecureWSSession>(shared_from_this()));
        m_serverOwner->notifyConnected(id().string());
    }
}

void LuaSecureWSSession::onWSDisconnected() {
    if (m_serverOwner) {
        m_serverOwner->notifyDisconnected(id().string());
        m_serverOwner->unregisterSession(id().string());
    }
}

void LuaSecureWSSession::onWSReceived(const void* buffer, size_t size) {
    if (m_serverOwner) {
        std::string data(static_cast<const char*>(buffer), size);
        m_serverOwner->notifyReceived(id().string(), data);
    }
}

// LuaSecureWSServer Implementation
LuaSecureWSServer::LuaSecureWSServer(const std::shared_ptr<CppServer::Asio::Service>& service, const std::shared_ptr<CppServer::Asio::SSLContext>& context, int port) {
    m_server = std::make_shared<InternalServer>(service, context, port);
}

std::expected<void, std::string> LuaSecureWSServer::startAsync() {
    if (m_server->Start()) {
        return {};
    }
    return std::unexpected("Failed to start SecureWebServer");
}

std::expected<void, std::string> LuaSecureWSServer::stopAsync() {
    if (m_server->Stop()) {
        return {};
    }
    return std::unexpected("Failed to stop SecureWebServer");
}

bool LuaSecureWSServer::broadcastText(const std::string& text) {
    return m_server->MulticastText(text);
}

bool LuaSecureWSServer::sendText(const std::string& id, const std::string& text) {
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    auto it = m_sessions.find(id);
    if (it != m_sessions.end()) {
        return it->second->SendTextAsync(text);
    }
    return false;
}

void LuaSecureWSServer::registerSession(const std::string& id, std::shared_ptr<LuaSecureWSSession> session) {
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    m_sessions[id] = session;
}

void LuaSecureWSServer::unregisterSession(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_sessionMutex);
    m_sessions.erase(id);
}

void LuaSecureWSServer::notifyConnected(const std::string& id) {
    if (onWSConnected) {
        EventTrampoline::getInstance().defer([cb = onWSConnected, i = id]() { cb(i); });
    }
}

void LuaSecureWSServer::notifyDisconnected(const std::string& id) {
    if (onDisconnected) {
        EventTrampoline::getInstance().defer([cb = onDisconnected, i = id]() { cb(i); });
    }
}

void LuaSecureWSServer::notifyReceived(const std::string& id, const std::string& data) {
    if (onWSReceived) {
        EventTrampoline::getInstance().defer([cb = onWSReceived, i = id, d = data]() { cb(i, d); });
    }
}

std::shared_ptr<CppServer::Asio::SSLSession> LuaSecureWSServer::InternalServer::CreateSession(const std::shared_ptr<CppServer::Asio::SSLServer>& server) {
    auto session = std::make_shared<LuaSecureWSSession>(std::static_pointer_cast<CppServer::WS::WSSServer>(server));
    auto o = owner.lock();
    if (o) {
        session->m_serverOwner = o.get();
    }
    return session;
}

void bindWebServer(sol::state_view& lua) {
    sol::table serverTable = lua["quasar"]["net"]["server"].get_or_create<sol::table>();

    // WebServer (Non-secure)
    sol::usertype<LuaProxy<LuaWSServer>> utWs = lua.new_usertype<LuaProxy<LuaWSServer>>("WebServer",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    utWs["start"] = [](LuaProxy<LuaWSServer> self, sol::this_state L) { 
        return to_sol_object(self.lock()->startAsync(), sol::state_view(L)); 
    };
    utWs["stop"] = [](LuaProxy<LuaWSServer> self, sol::this_state L) { 
        return to_sol_object(self.lock()->stopAsync(), sol::state_view(L)); 
    };
    utWs["broadcastText"] = [](LuaProxy<LuaWSServer> self, const std::string& text) { 
        return self.lock()->broadcastText(text); 
    };
    utWs["sendText"] = [](LuaProxy<LuaWSServer> self, const std::string& id, const std::string& text) {
        return self.lock()->sendText(id, text);
    };

    utWs["onWSReceived"] = [](LuaProxy<LuaWSServer> self, sol::function cb) { self.lock()->onWSReceived = cb; };
    utWs["onWSConnected"] = [](LuaProxy<LuaWSServer> self, sol::function cb) { self.lock()->onWSConnected = cb; };
    utWs["onDisconnected"] = [](LuaProxy<LuaWSServer> self, sol::function cb) { self.lock()->onDisconnected = cb; };

    serverTable["WebServer"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, int port, sol::this_state L) {
            std::shared_ptr<LuaWSServer> ptr = std::make_shared<LuaWSServer>(service, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaWSServer>(ptr);
        }
    );

    // SecureWebServer
    sol::usertype<LuaProxy<LuaSecureWSServer>> utWss = lua.new_usertype<LuaProxy<LuaSecureWSServer>>("SecureWebServer",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    utWss["start"] = [](LuaProxy<LuaSecureWSServer> self, sol::this_state L) { 
        return to_sol_object(self.lock()->startAsync(), sol::state_view(L)); 
    };
    utWss["stop"] = [](LuaProxy<LuaSecureWSServer> self, sol::this_state L) { 
        return to_sol_object(self.lock()->stopAsync(), sol::state_view(L)); 
    };
    utWss["broadcastText"] = [](LuaProxy<LuaSecureWSServer> self, const std::string& text) { 
        return self.lock()->broadcastText(text); 
    };
    utWss["sendText"] = [](LuaProxy<LuaSecureWSServer> self, const std::string& id, const std::string& text) {
        return self.lock()->sendText(id, text);
    };

    utWss["onWSReceived"] = [](LuaProxy<LuaSecureWSServer> self, sol::function cb) { self.lock()->onWSReceived = cb; };
    utWss["onWSConnected"] = [](LuaProxy<LuaSecureWSServer> self, sol::function cb) { self.lock()->onWSConnected = cb; };
    utWss["onDisconnected"] = [](LuaProxy<LuaSecureWSServer> self, sol::function cb) { self.lock()->onDisconnected = cb; };

    serverTable["SecureWebServer"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, const std::shared_ptr<CppServer::Asio::SSLContext>& context, int port, sol::this_state L) {
            std::shared_ptr<LuaSecureWSServer> ptr = std::make_shared<LuaSecureWSServer>(service, context, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaSecureWSServer>(ptr);
        }
    );
}

} // namespace quasar::net
