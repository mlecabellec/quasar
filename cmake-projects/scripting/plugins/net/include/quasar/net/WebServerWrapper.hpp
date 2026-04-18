#ifndef QUASAR_NET_WEBSERVERWRAPPER_HPP
#define QUASAR_NET_WEBSERVERWRAPPER_HPP

#include "server/ws/ws_server.h"
#include "server/ws/wss_server.h"
#include "quasar/net/EventTrampoline.hpp"
#include <sol/sol.hpp>
#include <memory>
#include <string>
#include <mutex>
#include <map>

namespace quasar::net {

class LuaWSServer;

class LuaWSSession : public CppServer::WS::WSSession {
public:
    explicit LuaWSSession(const std::shared_ptr<CppServer::WS::WSServer>& server) : CppServer::WS::WSSession(server) {}

    sol::function* onConnectedCb = nullptr;
    sol::function* onDisconnectedCb = nullptr;
    sol::function* onReceivedRequestCb = nullptr;
    sol::function* onWSConnectedCb = nullptr;
    sol::function* onWSDisconnectedCb = nullptr;
    sol::function* onWSReceivedCb = nullptr;
    sol::function* onErrorCb = nullptr;
    LuaWSServer* m_serverOwner = nullptr; 

protected:
    void onConnected() override;
    void onDisconnected() override;
    void onReceivedRequest(const CppServer::HTTP::HTTPRequest& request) override;
    void onWSConnected(const CppServer::HTTP::HTTPRequest& request) override;
    void onWSDisconnected() override;
    void onWSReceived(const void* buffer, size_t size) override;
    void onError(int error, const std::string& category, const std::string& message) override;
};

class LuaWSServer : public CppServer::WS::WSServer {
public:
    sol::function onConnectedCb;
    sol::function onDisconnectedCb;
    sol::function onReceivedRequestCb;
    sol::function onWSConnectedCb;
    sol::function onWSDisconnectedCb;
    sol::function onWSReceivedCb;
    sol::function onErrorCb;

    LuaWSServer(const std::shared_ptr<CppServer::Asio::Service>& service, int port)
        : CppServer::WS::WSServer(service, port) {}

    std::shared_ptr<LuaWSSession> getLuaSession(const std::string& id) {
        std::lock_guard<std::mutex> lock(m_sessionMutex);
        std::map<std::string, std::shared_ptr<LuaWSSession>>::iterator it = m_sessions.find(id);
        if (it != m_sessions.end()) return it->second;
        return nullptr;
    }

    void registerSession(const std::string& id, std::shared_ptr<LuaWSSession> session) {
        std::lock_guard<std::mutex> lock(m_sessionMutex);
        m_sessions[id] = session;
    }

    void unregisterSession(const std::string& id) {
        std::lock_guard<std::mutex> lock(m_sessionMutex);
        m_sessions.erase(id);
    }

protected:
    std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<CppServer::Asio::TCPServer>& server) override {
        std::shared_ptr<LuaWSSession> session = std::make_shared<LuaWSSession>(std::static_pointer_cast<CppServer::WS::WSServer>(server));
        session->onConnectedCb = &onConnectedCb;
        session->onDisconnectedCb = &onDisconnectedCb;
        session->onReceivedRequestCb = &onReceivedRequestCb;
        session->onWSConnectedCb = &onWSConnectedCb;
        session->onWSDisconnectedCb = &onWSDisconnectedCb;
        session->onWSReceivedCb = &onWSReceivedCb;
        session->onErrorCb = &onErrorCb;
        session->m_serverOwner = this;
        return session;
    }

private:
    std::mutex m_sessionMutex;
    std::map<std::string, std::shared_ptr<LuaWSSession>> m_sessions;
};

class LuaSecureWSServer;

class LuaSecureWSSession : public CppServer::WS::WSSSession {
public:
    explicit LuaSecureWSSession(const std::shared_ptr<CppServer::WS::WSSServer>& server) : CppServer::WS::WSSSession(server) {}

    sol::function* onConnectedCb = nullptr;
    sol::function* onDisconnectedCb = nullptr;
    sol::function* onReceivedRequestCb = nullptr;
    sol::function* onWSConnectedCb = nullptr;
    sol::function* onWSDisconnectedCb = nullptr;
    sol::function* onWSReceivedCb = nullptr;
    sol::function* onErrorCb = nullptr;
    LuaSecureWSServer* m_serverOwner = nullptr;

protected:
    void onConnected() override;
    void onDisconnected() override;
    void onReceivedRequest(const CppServer::HTTP::HTTPRequest& request) override;
    void onWSConnected(const CppServer::HTTP::HTTPRequest& request) override;
    void onWSDisconnected() override;
    void onWSReceived(const void* buffer, size_t size) override;
    void onError(int error, const std::string& category, const std::string& message) override;
};

class LuaSecureWSServer : public CppServer::WS::WSSServer {
public:
    sol::function onConnectedCb;
    sol::function onDisconnectedCb;
    sol::function onReceivedRequestCb;
    sol::function onWSConnectedCb;
    sol::function onWSDisconnectedCb;
    sol::function onWSReceivedCb;
    sol::function onErrorCb;

    LuaSecureWSServer(const std::shared_ptr<CppServer::Asio::Service>& service, const std::shared_ptr<CppServer::Asio::SSLContext>& context, int port)
        : CppServer::WS::WSSServer(service, context, port) {}

    std::shared_ptr<LuaSecureWSSession> getLuaSession(const std::string& id) {
        std::lock_guard<std::mutex> lock(m_sessionMutex);
        std::map<std::string, std::shared_ptr<LuaSecureWSSession>>::iterator it = m_sessions.find(id);
        if (it != m_sessions.end()) return it->second;
        return nullptr;
    }

    void registerSession(const std::string& id, std::shared_ptr<LuaSecureWSSession> session) {
        std::lock_guard<std::mutex> lock(m_sessionMutex);
        m_sessions[id] = session;
    }

    void unregisterSession(const std::string& id) {
        std::lock_guard<std::mutex> lock(m_sessionMutex);
        m_sessions.erase(id);
    }

protected:
    std::shared_ptr<CppServer::Asio::SSLSession> CreateSession(const std::shared_ptr<CppServer::Asio::SSLServer>& server) override {
        std::shared_ptr<LuaSecureWSSession> session = std::make_shared<LuaSecureWSSession>(std::static_pointer_cast<CppServer::WS::WSSServer>(server));
        session->onConnectedCb = &onConnectedCb;
        session->onDisconnectedCb = &onDisconnectedCb;
        session->onReceivedRequestCb = &onReceivedRequestCb;
        session->onWSConnectedCb = &onWSConnectedCb;
        session->onWSDisconnectedCb = &onWSDisconnectedCb;
        session->onWSReceivedCb = &onWSReceivedCb;
        session->onErrorCb = &onErrorCb;
        session->m_serverOwner = this;
        return session;
    }

private:
    std::mutex m_sessionMutex;
    std::map<std::string, std::shared_ptr<LuaSecureWSSession>> m_sessions;
};

// Aliases for Lua clarity
using LuaWebServer = LuaWSServer;

void bindWebServer(sol::state_view& lua);

} // namespace quasar::net

#endif // QUASAR_NET_WEBSERVERWRAPPER_HPP
