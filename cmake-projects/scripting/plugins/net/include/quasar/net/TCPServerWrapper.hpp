#ifndef QUASAR_NET_TCPSERVERWRAPPER_HPP
#define QUASAR_NET_TCPSERVERWRAPPER_HPP

#include "server/asio/tcp_server.h"
#include "quasar/net/EventTrampoline.hpp"
#include <sol/sol.hpp>
#include <memory>
#include <string>

namespace quasar::net {

class LuaTCPSession : public CppServer::Asio::TCPSession {
public:
    using CppServer::Asio::TCPSession::TCPSession;

    sol::function onConnectedCb;
    sol::function onDisconnectedCb;
    sol::function onReceivedCb;
    sol::function onErrorCb;

protected:
    void clearCallbacks() {
        onConnectedCb = sol::function();
        onDisconnectedCb = sol::function();
        onReceivedCb = sol::function();
        onErrorCb = sol::function();
    }

    void onConnected() override {
        if (onConnectedCb.valid() == true) {
            std::string id_str = this->id().string();
            std::shared_ptr<LuaTCPSession> self = std::dynamic_pointer_cast<LuaTCPSession>(shared_from_this());
            EventTrampoline::getInstance().defer([self, id = std::move(id_str)]() {
                try {
                    if (self != nullptr && self->onConnectedCb.valid() == true) self->onConnectedCb(id);
                } catch (const std::exception& e) {
                    fprintf(stderr, "TCPSession onConnected C++ EXCEPTION: %s\n", e.what());
                }
            });
        }
    }

    void onDisconnected() override {
        if (onDisconnectedCb.valid() == true) {
            std::string id_str = this->id().string();
            std::shared_ptr<LuaTCPSession> self = std::dynamic_pointer_cast<LuaTCPSession>(shared_from_this());
            EventTrampoline::getInstance().defer([self, id = std::move(id_str)]() {
                if (self != nullptr && self->onDisconnectedCb.valid() == true) self->onDisconnectedCb(id);
                if (self != nullptr) self->clearCallbacks();
            });
        } else {
            std::shared_ptr<LuaTCPSession> self = std::dynamic_pointer_cast<LuaTCPSession>(shared_from_this());
            EventTrampoline::getInstance().defer([self]() {
                if (self != nullptr) self->clearCallbacks();
            });
        }
    }

    void onReceived(const void* buffer, size_t size) override {
        if (onReceivedCb.valid() == true) {
            std::string id_str = this->id().string();
            std::string data(static_cast<const char*>(buffer), size);
            std::shared_ptr<LuaTCPSession> self = std::dynamic_pointer_cast<LuaTCPSession>(shared_from_this());
            EventTrampoline::getInstance().defer([self, id = std::move(id_str), data = std::move(data)]() {
                try {
                    if (self != nullptr && self->onReceivedCb.valid() == true) self->onReceivedCb(id, data);
                } catch (const std::exception& e) {
                    fprintf(stderr, "TCPSession onReceived C++ EXCEPTION: %s\n", e.what());
                }
            });
        }
    }

    void onError(int error, const std::string& category, const std::string& message) override {
        if (onErrorCb.valid() == true) {
            std::string id_str = this->id().string();
            std::shared_ptr<LuaTCPSession> self = std::dynamic_pointer_cast<LuaTCPSession>(shared_from_this());
            EventTrampoline::getInstance().defer([self, id = std::move(id_str), error, message]() {
                try {
                    if (self != nullptr && self->onErrorCb.valid() == true) self->onErrorCb(id, error, message);
                } catch (const std::exception& e) {
                    fprintf(stderr, "TCPSession onError C++ EXCEPTION: %s\n", e.what());
                }
            });
        }
    }
};

class LuaTCPServer : public CppServer::Asio::TCPServer {
public:
    sol::function onConnectedCb;
    sol::function onDisconnectedCb;
    sol::function onReceivedCb;
    sol::function onErrorCb;

    LuaTCPServer(const std::shared_ptr<CppServer::Asio::Service>& service, int port)
        : CppServer::Asio::TCPServer(service, port) {}

protected:
    std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<CppServer::Asio::TCPServer>& server) override {
        std::shared_ptr<LuaTCPSession> session = std::make_shared<LuaTCPSession>(server);
        session->onConnectedCb = onConnectedCb;
        session->onDisconnectedCb = onDisconnectedCb;
        session->onReceivedCb = onReceivedCb;
        session->onErrorCb = onErrorCb;
        return session;
    }

    void onError(int error, const std::string& category, const std::string& message) override {
        if (onErrorCb.valid() == true) {
            std::shared_ptr<LuaTCPServer> self = std::dynamic_pointer_cast<LuaTCPServer>(shared_from_this());
            EventTrampoline::getInstance().defer([self, error, message]() {
                if (self != nullptr && self->onErrorCb.valid() == true) self->onErrorCb("server", error, message);
            });
        }
    }
};

void bindTCPServer(sol::state_view& lua);

} // namespace quasar::net

#endif // QUASAR_NET_TCPSERVERWRAPPER_HPP
