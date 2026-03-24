#ifndef QUASAR_NET_TCPCLIENTWRAPPER_HPP
#define QUASAR_NET_TCPCLIENTWRAPPER_HPP

#include "server/asio/tcp_client.h"
#include "quasar/net/EventTrampoline.hpp"
#include <sol/sol.hpp>
#include <memory>
#include <string>

namespace quasar::net {

class LuaTCPClient : public CppServer::Asio::TCPClient {
public:
    sol::function onConnectedCb;
    sol::function onDisconnectedCb;
    sol::function onReceivedCb;
    sol::function onErrorCb;

    LuaTCPClient(const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port)
        : CppServer::Asio::TCPClient(service, address, port) {}

protected:
    void clearCallbacks() {
        onConnectedCb = sol::function();
        onDisconnectedCb = sol::function();
        onReceivedCb = sol::function();
        onErrorCb = sol::function();
    }

    void onConnected() override {
        if (onConnectedCb) {
            auto self = std::dynamic_pointer_cast<LuaTCPClient>(shared_from_this());
            EventTrampoline::getInstance().defer([self]() { 
                try {
                    if (self && self->onConnectedCb) self->onConnectedCb(); 
                } catch (const std::exception& e) {
                    fprintf(stderr, "TCPClient onConnected C++ EXCEPTION: %s\n", e.what());
                }
            });
        }
    }

    void onDisconnected() override {
        if (onDisconnectedCb) {
            auto self = std::dynamic_pointer_cast<LuaTCPClient>(shared_from_this());
            EventTrampoline::getInstance().defer([self]() { 
                if (self && self->onDisconnectedCb) self->onDisconnectedCb();
                if (self) self->clearCallbacks();
            });
        } else {
            auto self = std::dynamic_pointer_cast<LuaTCPClient>(shared_from_this());
            EventTrampoline::getInstance().defer([self]() { 
                if (self) self->clearCallbacks();
            });
        }
    }

    void onReceived(const void* buffer, size_t size) override {
        if (onReceivedCb) {
            std::string data(static_cast<const char*>(buffer), size);
            auto self = std::dynamic_pointer_cast<LuaTCPClient>(shared_from_this());
            EventTrampoline::getInstance().defer([self, data = std::move(data)]() { 
                try {
                    if (self && self->onReceivedCb) self->onReceivedCb(data); 
                } catch (const std::exception& e) {
                    fprintf(stderr, "TCPClient onReceived C++ EXCEPTION: %s\n", e.what());
                }
            });
        }
    }

    void onError(int error, const std::string& category, const std::string& message) override {
        if (onErrorCb) {
            auto self = std::dynamic_pointer_cast<LuaTCPClient>(shared_from_this());
            EventTrampoline::getInstance().defer([self, error, message]() { 
                try {
                    if (self && self->onErrorCb) self->onErrorCb(error, message); 
                } catch (const std::exception& e) {
                    fprintf(stderr, "TCPClient onError C++ EXCEPTION: %s\n", e.what());
                }
            });
        }
    }
};

void bindTCPClient(sol::state_view& lua);

} // namespace quasar::net

#endif // QUASAR_NET_TCPCLIENTWRAPPER_HPP
