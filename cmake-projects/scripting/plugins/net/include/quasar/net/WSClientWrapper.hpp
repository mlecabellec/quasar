#ifndef QUASAR_NET_WSCLIENTWRAPPER_HPP
#define QUASAR_NET_WSCLIENTWRAPPER_HPP

#include "server/ws/ws_client.h"
#include "server/ws/wss_client.h"
#include "quasar/net/EventTrampoline.hpp"
#include "quasar/net/ReconnectionPolicy.hpp"
#include <sol/sol.hpp>
#include <memory>
#include <string>
#include <expected>

namespace quasar::net {

/**
 * @class LuaWSClient
 * @brief WebSocket Client wrapper using composition to avoid inheritance complexity.
 */
class LuaWSClient : public std::enable_shared_from_this<LuaWSClient> {
public:
    sol::function onConnected;
    sol::function onDisconnected;
    sol::function onWSConnected;
    sol::function onWSDisconnected;
    sol::function onWSReceived;
    sol::function onError;

    LuaWSClient(const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port);

    [[nodiscard]] std::expected<void, std::string> connectAsync();
    [[nodiscard]] bool disconnectAsync();
    [[nodiscard]] bool sendTextAsync(const std::string& text);
    void setReconnectionPolicy(const ReconnectionPolicy& policy);

private:
    class InternalClient : public CppServer::WS::WSClient {
    public:
        using CppServer::WS::WSClient::WSClient;
        std::weak_ptr<LuaWSClient> owner;
    protected:
        void onConnected() override;
        void onDisconnected() override;
        void onWSConnected(const CppServer::HTTP::HTTPResponse& response) override;
        void onWSDisconnected() override;
        void onWSReceived(const void* buffer, size_t size) override;
        void onError(int error, const std::string& category, const std::string& message) override;
    };

    std::shared_ptr<InternalClient> m_client;
    ReconnectionPolicy m_policy;
    int m_attempt;
};

/**
 * @class LuaSecureWSClient
 */
class LuaSecureWSClient : public std::enable_shared_from_this<LuaSecureWSClient> {
public:
    sol::function onConnected;
    sol::function onDisconnected;
    sol::function onWSConnected;
    sol::function onWSDisconnected;
    sol::function onWSReceived;
    sol::function onError;

    LuaSecureWSClient(const std::shared_ptr<CppServer::Asio::Service>& service, const std::shared_ptr<CppServer::Asio::SSLContext>& context, const std::string& address, int port);

    [[nodiscard]] std::expected<void, std::string> connectAsync();
    [[nodiscard]] bool disconnectAsync();
    [[nodiscard]] bool sendTextAsync(const std::string& text);
    void setReconnectionPolicy(const ReconnectionPolicy& policy);

private:
    class InternalClient : public CppServer::WS::WSSClient {
    public:
        using CppServer::WS::WSSClient::WSSClient;
        std::weak_ptr<LuaSecureWSClient> owner;
    protected:
        void onConnected() override;
        void onDisconnected() override;
        void onWSConnected(const CppServer::HTTP::HTTPResponse& response) override;
        void onWSDisconnected() override;
        void onWSReceived(const void* buffer, size_t size) override;
        void onError(int error, const std::string& category, const std::string& message) override;
    };

    std::shared_ptr<InternalClient> m_client;
    ReconnectionPolicy m_policy;
    int m_attempt;
};

void bindWSClient(sol::state_view& lua);

} // namespace quasar::net

#endif // QUASAR_NET_WSCLIENTWRAPPER_HPP
