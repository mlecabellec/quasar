#ifndef QUASAR_NET_UDPCLIENTWRAPPER_HPP
#define QUASAR_NET_UDPCLIENTWRAPPER_HPP

#include "server/asio/udp_client.h"
#include "quasar/net/EventTrampoline.hpp"
#include <sol/sol.hpp>
#include <memory>
#include <string>

namespace quasar::net {

/**
 * @class LuaUDPClient
 * @brief UDP Client wrapper exposed to Lua
 */
class LuaUDPClient : public CppServer::Asio::UDPClient {
public:
    sol::function onConnectedCb;
    sol::function onDisconnectedCb;
    sol::function onReceivedCb;
    sol::function onErrorCb;
    sol::function onJoinedMulticastGroupCb;
    sol::function onLeftMulticastGroupCb;

    LuaUDPClient(const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port)
        : CppServer::Asio::UDPClient(service, address, port) {}

protected:
    void onConnected() override {
        if (onConnectedCb) {
            EventTrampoline::getInstance().defer([cb = onConnectedCb]() { cb(); });
        }
        ReceiveAsync();
    }

    void onDisconnected() override {
        if (onDisconnectedCb) {
            EventTrampoline::getInstance().defer([cb = onDisconnectedCb]() { cb(); });
        }
    }

    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override {
        if (onReceivedCb) {
            std::string data(static_cast<const char*>(buffer), size);
            std::string ep_addr = endpoint.address().to_string();
            int ep_port = endpoint.port();
            EventTrampoline::getInstance().defer([cb = onReceivedCb, ep_addr, ep_port, d = std::move(data)]() { 
                cb(ep_addr, ep_port, d); 
            });
        }
        ReceiveAsync();
    }

    void onError(int error, const std::string& category, const std::string& message) override {
        if (onErrorCb) {
            EventTrampoline::getInstance().defer([cb = onErrorCb, error, msg = message]() { cb(error, msg); });
        }
    }

    void onJoinedMulticastGroup(const std::string& address) override {
        if (onJoinedMulticastGroupCb) {
            EventTrampoline::getInstance().defer([cb = onJoinedMulticastGroupCb, addr = address]() { cb(addr); });
        }
    }

    void onLeftMulticastGroup(const std::string& address) override {
        if (onLeftMulticastGroupCb) {
            EventTrampoline::getInstance().defer([cb = onLeftMulticastGroupCb, addr = address]() { cb(addr); });
        }
    }
};

void bindUDPClient(sol::state_view& lua);

} // namespace quasar::net

#endif // QUASAR_NET_UDPCLIENTWRAPPER_HPP
