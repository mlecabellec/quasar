#include "quasar/net/WebServerWrapper.hpp"
#include "server/ws/wss_server.h"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/net/EventTrampoline.hpp"
#include <iostream>

namespace quasar::net {

using namespace quasar::scripting;

struct LuaSecureWSServer::Impl {
    class LuaSecureWSSession : public ::CppServer::WS::WSSSession {
    public:
        explicit LuaSecureWSSession(const std::shared_ptr<::CppServer::WS::WSSServer>& server) 
            : ::CppServer::WS::WSSSession(server) {}
        LuaSecureWSServer* m_owner = nullptr;

    protected:
        bool onWSConnecting(const ::CppServer::HTTP::HTTPRequest& request, ::CppServer::HTTP::HTTPResponse& response) override {
            return ::CppServer::WS::WSSSession::onWSConnecting(request, response);
        }

        void onWSConnected(const ::CppServer::HTTP::HTTPRequest& request) override {
            (void)request;
            if (m_owner) {
                m_owner->m_impl->registerSession(id().string(), std::static_pointer_cast<LuaSecureWSSession>(shared_from_this()));
                std::lock_guard<std::recursive_mutex> lock(m_owner->m_callbackMutex);
                if (m_owner->onWSConnected) {
                    std::string sid = id().string();
                    EventTrampoline::getInstance().defer([cb = m_owner->onWSConnected, sid]() { cb(sid); });
                }
            }
        }

        void onWSDisconnected() override {
            if (m_owner) {
                std::lock_guard<std::recursive_mutex> lock(m_owner->m_callbackMutex);
                if (m_owner->onDisconnected) {
                    std::string sid = id().string();
                    EventTrampoline::getInstance().defer([cb = m_owner->onDisconnected, sid]() { cb(sid); });
                }
                m_owner->m_impl->unregisterSession(id().string());
            }
        }

        void onWSReceived(const void* buffer, size_t size) override {
            if (m_owner) {
                std::lock_guard<std::recursive_mutex> lock(m_owner->m_callbackMutex);
                if (m_owner->onWSReceived) {
                    std::string sid = id().string();
                    std::string data((const char*)buffer, size);
                    EventTrampoline::getInstance().defer([cb = m_owner->onWSReceived, sid, data]() { cb(sid, data); });
                }
            }
        }
    };

    class InternalServer : public ::CppServer::WS::WSSServer {
    public:
        InternalServer(const std::shared_ptr<::CppServer::Asio::Service>& service, const std::shared_ptr<::CppServer::Asio::SSLContext>& context, int port)
            : ::CppServer::WS::WSSServer(service, context, port) {}
        LuaSecureWSServer* m_owner = nullptr;

    protected:
        /**
         * @brief Create a new secure session.
         * @param server The server object.
         * @return A shared pointer to the new session.
         */
        std::shared_ptr<::CppServer::Asio::SSLSession> CreateSession(const std::shared_ptr<::CppServer::Asio::SSLServer>& server) override {
            std::shared_ptr<LuaSecureWSSession> session = std::make_shared<LuaSecureWSSession>(std::static_pointer_cast<::CppServer::WS::WSSServer>(server));
            session->m_owner = m_owner;
            return session;
        }
    };

    std::shared_ptr<InternalServer> server;
    std::map<std::string, std::shared_ptr<LuaSecureWSSession>> sessions;
    std::mutex sessionMutex;

    void registerSession(const std::string& id, std::shared_ptr<LuaSecureWSSession> session) {
        std::lock_guard<std::mutex> lock(sessionMutex);
        sessions[id] = session;
    }

    void unregisterSession(const std::string& id) {
        std::lock_guard<std::mutex> lock(sessionMutex);
        sessions.erase(id);
    }
};

LuaSecureWSServer::LuaSecureWSServer(const std::shared_ptr<::CppServer::Asio::Service>& service, const std::shared_ptr<::CppServer::Asio::SSLContext>& context, int port) {
    m_impl = std::make_shared<Impl>();
    m_impl->server = std::make_shared<Impl::InternalServer>(service, context, port);
    m_impl->server->m_owner = this;
}

LuaSecureWSServer::~LuaSecureWSServer() {
    (void)stopAsync();
}

std::expected<void, std::string> LuaSecureWSServer::startAsync() {
    if (m_impl->server->Start()) return {};
    return std::unexpected("Failed to start Secure WebServer");
}

std::expected<void, std::string> LuaSecureWSServer::stopAsync() {
    if (m_impl->server->Stop()) return {};
    return std::unexpected("Failed to stop Secure WebServer");
}

bool LuaSecureWSServer::broadcastText(const std::string& text) {
    return m_impl->server->MulticastText(text);
}

bool LuaSecureWSServer::sendText(const std::string& id, const std::string& text) {
    std::lock_guard<std::mutex> lock(m_impl->sessionMutex);
    auto it = m_impl->sessions.find(id);
    if (it != m_impl->sessions.end()) {
        return it->second->SendTextAsync(text);
    }
    return false;
}

void bindWebServer(sol::state_view& lua) {
    sol::table serverTable = lua["quasar"]["net"]["server"].get_or_create<sol::table>();

    // WebServer Usertype
    sol::usertype<LuaProxy<LuaWSServer>> utWs = lua.new_usertype<LuaProxy<LuaWSServer>>("WebServer",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    utWs["start"] = [](LuaProxy<LuaWSServer> self, sol::this_state L) { 
        auto res = self.lock()->startAsync();
        if (res) return sol::make_object(L, true);
        return sol::make_object(L, res.error());
    };
    utWs["stop"] = [](LuaProxy<LuaWSServer> self, sol::this_state L) { 
        auto res = self.lock()->stopAsync();
        if (res) return sol::make_object(L, true);
        return sol::make_object(L, res.error());
    };
    utWs["broadcastText"] = [](LuaProxy<LuaWSServer> self, const std::string& text) { return self.lock()->broadcastText(text); };
    utWs["sendText"] = [](LuaProxy<LuaWSServer> self, const std::string& id, const std::string& text) { 
        return self.lock()->sendText(id, text); 
    };

    utWs["onWSReceived"] = [](LuaProxy<LuaWSServer> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onWSReceived = cb; 
    };
    utWs["onWSConnected"] = [](LuaProxy<LuaWSServer> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onWSConnected = cb; 
    };
    utWs["onDisconnected"] = [](LuaProxy<LuaWSServer> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onDisconnected = cb; 
    };

    serverTable["WebServer"] = lua.create_table_with(
        "new", [](const std::shared_ptr<::CppServer::Asio::Service>& service, int port, sol::this_state L) {
            std::shared_ptr<LuaWSServer> ptr = std::make_shared<LuaWSServer>(service, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaWSServer>(ptr);
        }
    );

    // SecureWSServer Usertype
    sol::usertype<LuaProxy<LuaSecureWSServer>> utSw = lua.new_usertype<LuaProxy<LuaSecureWSServer>>("SecureWSServer",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    utSw["start"] = [](LuaProxy<LuaSecureWSServer> self, sol::this_state L) { 
        auto res = self.lock()->startAsync();
        if (res) return sol::make_object(L, true);
        return sol::make_object(L, res.error());
    };
    utSw["stop"] = [](LuaProxy<LuaSecureWSServer> self, sol::this_state L) { 
        auto res = self.lock()->stopAsync();
        if (res) return sol::make_object(L, true);
        return sol::make_object(L, res.error());
    };
    utSw["broadcastText"] = [](LuaProxy<LuaSecureWSServer> self, const std::string& text) { return self.lock()->broadcastText(text); };
    utSw["sendText"] = [](LuaProxy<LuaSecureWSServer> self, const std::string& id, const std::string& text) { 
        return self.lock()->sendText(id, text); 
    };

    utSw["onWSReceived"] = [](LuaProxy<LuaSecureWSServer> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onWSReceived = cb; 
    };
    utSw["onWSConnected"] = [](LuaProxy<LuaSecureWSServer> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onWSConnected = cb; 
    };
    utSw["onDisconnected"] = [](LuaProxy<LuaSecureWSServer> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onDisconnected = cb; 
    };

    serverTable["SecureWSServer"] = lua.create_table_with(
        "new", [](const std::shared_ptr<::CppServer::Asio::Service>& service, const std::shared_ptr<::CppServer::Asio::SSLContext>& context, int port, sol::this_state L) {
            std::shared_ptr<LuaSecureWSServer> ptr = std::make_shared<LuaSecureWSServer>(service, context, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaSecureWSServer>(ptr);
        }
    );
}

} // namespace quasar::net
