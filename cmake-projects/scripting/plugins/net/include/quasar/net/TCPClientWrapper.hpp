#ifndef QUASAR_NET_TCPCLIENTWRAPPER_HPP
#define QUASAR_NET_TCPCLIENTWRAPPER_HPP

#include "server/asio/tcp_client.h"
#include "quasar/net/EventTrampoline.hpp"
#include "quasar/net/ReconnectionPolicy.hpp"
#include <sol/sol.hpp>
#include <memory>
#include <string>
#include <expected>

namespace quasar::net {

/**
 * @class LuaTCPClient
 * @brief High-performance TCP Client for Lua with reconnection support.
 */
class LuaTCPClient : public std::enable_shared_from_this<LuaTCPClient> {
public:
    sol::function onConnected;
    sol::function onDisconnected;
    sol::function onReceived;
    sol::function onError;

    LuaTCPClient(const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port);

    [[nodiscard]] std::expected<void, std::string> connectAsync();
    [[nodiscard]] bool disconnectAsync();
    [[nodiscard]] bool sendAsync(const std::string& data);
    void setReconnectionPolicy(const ReconnectionPolicy& policy);

private:
    class InternalClient : public CppServer::Asio::TCPClient {
    public:
        using CppServer::Asio::TCPClient::TCPClient;
        LuaTCPClient* owner = nullptr;
    protected:
        void onConnected() override;
        void onDisconnected() override;
        void onReceived(const void* buffer, size_t size) override;
        void onError(int error, const std::string& category, const std::string& message) override;
    };

    std::shared_ptr<InternalClient> m_client;
    ReconnectionPolicy m_policy;
    int m_attempt;
};

void bindTCPClient(sol::state_view& lua);

} // namespace quasar::net

#endif // QUASAR_NET_TCPCLIENTWRAPPER_HPP
