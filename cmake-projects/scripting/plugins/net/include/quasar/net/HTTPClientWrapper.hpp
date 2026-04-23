#ifndef QUASAR_NET_HTTPCLIENTWRAPPER_HPP
#define QUASAR_NET_HTTPCLIENTWRAPPER_HPP

#include "server/http/http_client.h"
#include "server/http/https_client.h"
#include "quasar/net/EventTrampoline.hpp"
#include <sol/sol.hpp>
#include <memory>
#include <string>

namespace quasar::net {

class LuaHTTPClient : public CppServer::HTTP::HTTPClient {
public:
    sol::function onConnectedCb;
    sol::function onDisconnectedCb;
    sol::function onReceivedResponseCb;
    sol::function onErrorCb;

    LuaHTTPClient(const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port)
        : CppServer::HTTP::HTTPClient(service, address, port) {}

protected:
    void onConnected() override {
        if (onConnectedCb) {
            EventTrampoline::getInstance().defer([cb = onConnectedCb]() { cb(); });
        }
    }

    void onDisconnected() override {
        if (onDisconnectedCb) {
            EventTrampoline::getInstance().defer([cb = onDisconnectedCb]() { cb(); });
        }
    }

    void onReceivedResponse(const CppServer::HTTP::HTTPResponse& response) override {
        if (onReceivedResponseCb) {
            int status = response.status();
            std::string body = std::string(response.body());
            EventTrampoline::getInstance().defer([cb = onReceivedResponseCb, status, body = std::move(body)]() { cb(status, body); });
        }
    }

    void onError(int error, const std::string& category, const std::string& message) override {
        if (onErrorCb) {
            EventTrampoline::getInstance().defer([cb = onErrorCb, error, message]() { cb(error, message); });
        }
    }
};

class LuaSecureHTTPClient : public CppServer::HTTP::HTTPSClient {
public:
    sol::function onConnectedCb;
    sol::function onDisconnectedCb;
    sol::function onReceivedResponseCb;
    sol::function onErrorCb;

    LuaSecureHTTPClient(const std::shared_ptr<CppServer::Asio::Service>& service, const std::shared_ptr<CppServer::Asio::SSLContext>& context, const std::string& address, int port)
        : CppServer::HTTP::HTTPSClient(service, context, address, port) {}

protected:
    void onConnected() override {
        if (onConnectedCb) {
            EventTrampoline::getInstance().defer([cb = onConnectedCb]() { cb(); });
        }
    }

    void onDisconnected() override {
        if (onDisconnectedCb) {
            EventTrampoline::getInstance().defer([cb = onDisconnectedCb]() { cb(); });
        }
    }

    void onReceivedResponse(const CppServer::HTTP::HTTPResponse& response) override {
        if (onReceivedResponseCb) {
            int status = response.status();
            std::string body = std::string(response.body());
            EventTrampoline::getInstance().defer([cb = onReceivedResponseCb, status, body = std::move(body)]() { cb(status, body); });
        }
    }

    void onError(int error, const std::string& category, const std::string& message) override {
        if (onErrorCb) {
            EventTrampoline::getInstance().defer([cb = onErrorCb, error, message]() { cb(error, message); });
        }
    }
};

void bindHTTPClient(sol::state_view& lua);

} // namespace quasar::net

#endif // QUASAR_NET_HTTPCLIENTWRAPPER_HPP