#ifndef QUASAR_NET_UDPSERVERWRAPPER_HPP
#define QUASAR_NET_UDPSERVERWRAPPER_HPP

#include "server/asio/udp_server.h"
#include "quasar/net/EventTrampoline.hpp"
#include <sol/sol.hpp>
#include <memory>
#include <string>

namespace quasar::net {

class LuaUDPServer : public CppServer::Asio::UDPServer {
public:
    sol::function onStartedCb;
    sol::function onStoppedCb;
    sol::function onReceivedCb;
    sol::function onErrorCb;

    LuaUDPServer(const std::shared_ptr<CppServer::Asio::Service>& service, int port)
        : CppServer::Asio::UDPServer(service, port) {}

protected:
    void onStarted() override {
        if (onStartedCb) {
            EventTrampoline::getInstance().defer([cb = onStartedCb]() { cb(); });
        }
    }

    void onStopped() override {
        if (onStoppedCb) {
            EventTrampoline::getInstance().defer([cb = onStoppedCb]() { cb(); });
        }
    }

    void onReceived(const asio::ip::udp::endpoint& endpoint, const void* buffer, size_t size) override {
        if (onReceivedCb) {
            std::string data(static_cast<const char*>(buffer), size);
            std::string addr = endpoint.address().to_string();
            int port = endpoint.port();
            EventTrampoline::getInstance().defer([cb = onReceivedCb, addr, port, data = std::move(data)]() {
                cb(addr, port, data);
            });
        }
    }

    void onError(int error, const std::string& category, const std::string& message) override {
        if (onErrorCb) {
            EventTrampoline::getInstance().defer([cb = onErrorCb, error, message]() {
                cb("udpserver", error, message);
            });
        }
    }
};

void bindUDPServer(sol::state_view& lua);

} // namespace quasar::net

#endif // QUASAR_NET_UDPSERVERWRAPPER_HPP
