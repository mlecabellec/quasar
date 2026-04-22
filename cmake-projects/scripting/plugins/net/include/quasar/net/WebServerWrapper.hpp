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
#include <expected>

namespace quasar::net {

class LuaWSServer;

/**
 * @class LuaWSSession
 * @brief WebSocket Session wrapper.
 */
class LuaWSSession : public CppServer::WS::WSSession {
public:
    explicit LuaWSSession(const std::shared_ptr<CppServer::WS::WSServer>& server);

    LuaWSServer* m_serverOwner = nullptr;

protected:
    void onWSConnected(const CppServer::HTTP::HTTPRequest& request) override;
    void onWSDisconnected() override;
    void onWSReceived(const void* buffer, size_t size) override;
};

/**
 * @class LuaWSServer
 */
class LuaWSServer : public std::enable_shared_from_this<LuaWSServer> {
public:
    sol::function onWSConnected;
    sol::function onWSReceived;
    sol::function onDisconnected;

    LuaWSServer(const std::shared_ptr<CppServer::Asio::Service>& service, int port);

    [[nodiscard]] std::expected<void, std::string> startAsync();
    [[nodiscard]] std::expected<void, std::string> stopAsync();
    [[nodiscard]] bool broadcastText(const std::string& text);
    [[nodiscard]] bool sendText(const std::string& id, const std::string& text);

    void registerSession(const std::string& id, std::shared_ptr<LuaWSSession> session);
    void unregisterSession(const std::string& id);
    void notifyConnected(const std::string& id);
    void notifyDisconnected(const std::string& id);
    void notifyReceived(const std::string& id, const std::string& data);

private:
    class InternalServer : public CppServer::WS::WSServer {
    public:
        using CppServer::WS::WSServer::WSServer;
        std::weak_ptr<LuaWSServer> owner;
    protected:
        std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<CppServer::Asio::TCPServer>& server) override;
    };

    std::shared_ptr<InternalServer> m_server;
    std::mutex m_sessionMutex;
    std::map<std::string, std::shared_ptr<LuaWSSession>> m_sessions;
};

class LuaSecureWSServer;

/**
 * @class LuaSecureWSSession
 */
class LuaSecureWSSession : public CppServer::WS::WSSSession {
public:
    explicit LuaSecureWSSession(const std::shared_ptr<CppServer::WS::WSSServer>& server);

    LuaSecureWSServer* m_serverOwner = nullptr;

protected:
    void onWSConnected(const CppServer::HTTP::HTTPRequest& request) override;
    void onWSDisconnected() override;
    void onWSReceived(const void* buffer, size_t size) override;
};

/**
 * @class LuaSecureWSServer
 */
class LuaSecureWSServer : public std::enable_shared_from_this<LuaSecureWSServer> {
public:
    sol::function onWSConnected;
    sol::function onWSReceived;
    sol::function onDisconnected;

    LuaSecureWSServer(const std::shared_ptr<CppServer::Asio::Service>& service, const std::shared_ptr<CppServer::Asio::SSLContext>& context, int port);

    [[nodiscard]] std::expected<void, std::string> startAsync();
    [[nodiscard]] std::expected<void, std::string> stopAsync();
    [[nodiscard]] bool broadcastText(const std::string& text);
    [[nodiscard]] bool sendText(const std::string& id, const std::string& text);

    void registerSession(const std::string& id, std::shared_ptr<LuaSecureWSSession> session);
    void unregisterSession(const std::string& id);
    void notifyConnected(const std::string& id);
    void notifyDisconnected(const std::string& id);
    void notifyReceived(const std::string& id, const std::string& data);

private:
    class InternalServer : public CppServer::WS::WSSServer { 
    public:
        InternalServer(const std::shared_ptr<CppServer::Asio::Service>& service, const std::shared_ptr<CppServer::Asio::SSLContext>& context, int port)
            : CppServer::WS::WSSServer(service, context, port) {}
        std::weak_ptr<LuaSecureWSServer> owner;
    protected:
        std::shared_ptr<CppServer::Asio::SSLSession> CreateSession(const std::shared_ptr<CppServer::Asio::SSLServer>& server) override;
    };

    std::shared_ptr<InternalServer> m_server;
    std::mutex m_sessionMutex;
    std::map<std::string, std::shared_ptr<LuaSecureWSSession>> m_sessions;
};

void bindWebServer(sol::state_view& lua);

} // namespace quasar::net

#endif // QUASAR_NET_WEBSERVERWRAPPER_HPP
