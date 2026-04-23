#ifndef QUASAR_NET_WEBSERVERWRAPPER_HPP
#define QUASAR_NET_WEBSERVERWRAPPER_HPP

#include <sol/sol.hpp>
#include <memory>
#include <string>
#include <mutex>
#include <map>
#include <expected>

// Forward declarations for CppServer types
namespace CppServer::Asio {
    class Service;
    class SSLContext;
}

namespace quasar::net {

class LuaWSServer;
class LuaSecureWSServer;

/**
 * @class LuaWSServer
 * @brief Wrapper for non-secure WebSocket server.
 */
class LuaWSServer : public std::enable_shared_from_this<LuaWSServer> {
public:
    sol::function onWSConnected;
    sol::function onWSReceived;
    sol::function onDisconnected;

    LuaWSServer(const std::shared_ptr<::CppServer::Asio::Service>& service, int port);
    ~LuaWSServer();

    [[nodiscard]] std::expected<void, std::string> startAsync();
    [[nodiscard]] std::expected<void, std::string> stopAsync();
    [[nodiscard]] bool broadcastText(const std::string& text);
    [[nodiscard]] bool sendText(const std::string& id, const std::string& text);

    std::recursive_mutex m_callbackMutex;
    std::mutex m_sessionMutex;

    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @class LuaSecureWSServer
 * @brief Wrapper for secure WebSocket server.
 */
class LuaSecureWSServer : public std::enable_shared_from_this<LuaSecureWSServer> {
public:
    sol::function onWSConnected;
    sol::function onWSReceived;
    sol::function onDisconnected;

    LuaSecureWSServer(const std::shared_ptr<::CppServer::Asio::Service>& service, const std::shared_ptr<::CppServer::Asio::SSLContext>& context, int port);
    ~LuaSecureWSServer();

    [[nodiscard]] std::expected<void, std::string> startAsync();
    [[nodiscard]] std::expected<void, std::string> stopAsync();
    [[nodiscard]] bool broadcastText(const std::string& text);
    [[nodiscard]] bool sendText(const std::string& id, const std::string& text);

    std::recursive_mutex m_callbackMutex;
    std::mutex m_sessionMutex;

    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

void bindWebServer(sol::state_view& lua);

} // namespace quasar::net

#endif // QUASAR_NET_WEBSERVERWRAPPER_HPP
