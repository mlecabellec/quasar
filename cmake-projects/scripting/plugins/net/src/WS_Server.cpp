#include "quasar/net/WebServerWrapper.hpp"
#include "server/ws/ws_server.h"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/net/EventTrampoline.hpp"
#include <iostream>

namespace quasar::net {

using namespace quasar::scripting;

struct LuaWSServer::Impl {
    class LuaWSSession;

    class LuaWSSession : public ::CppServer::WS::WSSession {
    public:
        explicit LuaWSSession(const std::shared_ptr<::CppServer::WS::WSServer>& server) 
            : ::CppServer::WS::WSSession(server) {
            std::cout << "[CPP Debug] LuaWSSession created: " << (void*)this << std::endl;
        }
        LuaWSServer* m_owner = nullptr;

    protected:
        void onConnected() override {
            std::cout << "[CPP Debug] LuaWSSession::onConnected (TCP level) for " << (void*)this << std::endl;
            ::CppServer::WS::WSSession::onConnected();
        }

        void onDisconnected() override {
            std::cout << "[CPP Debug] LuaWSSession::onDisconnected (TCP level) for " << (void*)this << std::endl;
            ::CppServer::WS::WSSession::onDisconnected();
        }

        void onReceived(const void* buffer, size_t size) override {
            std::cout << "[CPP Debug] LuaWSSession::onReceived (TCP level) size: " << size << " for " << (void*)this << std::endl;
            ::CppServer::WS::WSSession::onReceived(buffer, size);
        }

        bool onWSConnecting(const ::CppServer::HTTP::HTTPRequest& request, ::CppServer::HTTP::HTTPResponse& response) override {
            std::cout << "[CPP Debug] LuaWSSession::onWSConnecting triggered for " << (void*)this << std::endl;
            return ::CppServer::WS::WSSession::onWSConnecting(request, response);
        }

        void onWSConnected(const ::CppServer::HTTP::HTTPRequest& request) override {
            (void)request;
            std::cout << "[CPP Debug] LuaWSSession::onWSConnected triggered for " << static_cast<void*>(this) << std::endl;
            if (m_owner != nullptr) {
                m_owner->m_impl->registerSession(id().string(), std::static_pointer_cast<LuaWSSession>(shared_from_this()));
                std::lock_guard<std::recursive_mutex> lock(m_owner->m_callbackMutex);
                if (m_owner->onWSConnected.valid() == true) {
                    std::string sid = id().string();
                    EventTrampoline::getInstance().defer([cb = m_owner->onWSConnected, sid]() { cb(sid); });
                }
            }
        }

        void onWSDisconnected() override {
            std::cout << "[CPP Debug] LuaWSSession::onWSDisconnected triggered for " << static_cast<void*>(this) << std::endl;
            if (m_owner != nullptr) {
                std::lock_guard<std::recursive_mutex> lock(m_owner->m_callbackMutex);
                if (m_owner->onDisconnected.valid() == true) {
                    std::string sid = id().string();
                    EventTrampoline::getInstance().defer([cb = m_owner->onDisconnected, sid]() { cb(sid); });
                }
                m_owner->m_impl->unregisterSession(id().string());
            }
        }

        void onWSReceived(const void* buffer, size_t size) override {
            std::cout << "[CPP Debug] LuaWSSession::onWSReceived triggered, size: " << size << std::endl;
            if (m_owner != nullptr) {
                std::lock_guard<std::recursive_mutex> lock(m_owner->m_callbackMutex);
                if (m_owner->onWSReceived.valid() == true) {
                    std::string sid = id().string();
                    std::string data(static_cast<const char*>(buffer), size);
                    EventTrampoline::getInstance().defer([cb = m_owner->onWSReceived, sid, data]() { cb(sid, data); });
                }
            }
        }

        void onError(int error, const std::string& category, const std::string& message) override {
            std::cerr << "[CPP Error] LuaWSSession ERROR: " << error << " (" << category << "): " << message << std::endl;
        }
    };

    class InternalServer : public ::CppServer::WS::WSServer {
    public:
        InternalServer(const std::shared_ptr<::CppServer::Asio::Service>& service, int port)
            : ::CppServer::WS::WSServer(service, port) {}
        LuaWSServer* m_owner = nullptr;

    protected:
        void onConnected(std::shared_ptr<::CppServer::Asio::TCPSession>& session) override {
            std::cout << "[CPP Debug] InternalServer::onConnected triggered" << std::endl;
            ::CppServer::WS::WSServer::onConnected(session);
        }

        void onDisconnected(std::shared_ptr<::CppServer::Asio::TCPSession>& session) override {
            std::cout << "[CPP Debug] InternalServer::onDisconnected triggered" << std::endl;
            ::CppServer::WS::WSServer::onDisconnected(session);
        }

        /**
         * @brief Create a new session.
         * @param server The server object.
         * @return A shared pointer to the new session.
         */
        std::shared_ptr<::CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<::CppServer::Asio::TCPServer>& server) override {
            std::cout << "[CPP Debug] InternalServer::CreateSession triggered" << std::endl;
            std::shared_ptr<LuaWSSession> session = std::make_shared<LuaWSSession>(std::static_pointer_cast<::CppServer::WS::WSServer>(server));
            session->m_owner = m_owner;
            return session;
        }

        void onError(int error, const std::string& category, const std::string& message) override {
            std::cerr << "[CPP Error] InternalServer ERROR: " << error << " (" << category << "): " << message << std::endl;
        }
    };

    std::shared_ptr<InternalServer> server;
    std::map<std::string, std::shared_ptr<LuaWSSession>> sessions;
    std::mutex sessionMutex;

    void registerSession(const std::string& id, std::shared_ptr<LuaWSSession> session) {
        std::lock_guard<std::mutex> lock(sessionMutex);
        sessions[id] = session;
    }

    void unregisterSession(const std::string& id) {
        std::lock_guard<std::mutex> lock(sessionMutex);
        sessions.erase(id);
    }
};

LuaWSServer::LuaWSServer(const std::shared_ptr<::CppServer::Asio::Service>& service, int port) {
    m_impl = std::make_shared<Impl>();
    m_impl->server = std::make_shared<Impl::InternalServer>(service, port);
    m_impl->server->m_owner = this;
}

LuaWSServer::~LuaWSServer() {
    (void)stopAsync();
}

std::expected<void, std::string> LuaWSServer::startAsync() {
    std::cout << "[CPP Debug] LuaWSServer::startAsync() called" << std::endl;
    if (m_impl->server->Start()) return {};
    return std::unexpected("Failed to start WebServer");
}

std::expected<void, std::string> LuaWSServer::stopAsync() {
    if (m_impl->server->Stop()) return {};
    return std::unexpected("Failed to stop WebServer");
}

bool LuaWSServer::broadcastText(const std::string& text) {
    return m_impl->server->MulticastText(text);
}

bool LuaWSServer::sendText(const std::string& id, const std::string& text) {
    std::lock_guard<std::mutex> lock(m_impl->sessionMutex);
    std::map<std::string, std::shared_ptr<Impl::LuaWSSession>>::iterator it = m_impl->sessions.find(id);
    if (it != m_impl->sessions.end()) {
        return it->second->SendTextAsync(text);
    }
    return false;
}

} // namespace quasar::net
