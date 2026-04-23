#ifndef QUASAR_WEBUI_WEBUISERVICE_HPP
#define QUASAR_WEBUI_WEBUISERVICE_HPP

#include "quasar/named/ActiveEntity.hpp"
#include "server/http/http_server.h"
#include "server/http/http_session.h"
#include <memory>
#include <string>
#include <mutex>
#include <map>

namespace quasar::webui {

class WebUIService;

/**
 * @class InternalHTTPSession
 * @brief Handles HTTP requests for the Web UI.
 */
class InternalHTTPSession : public CppServer::HTTP::HTTPSession {
public:
    InternalHTTPSession(const std::shared_ptr<CppServer::HTTP::HTTPServer>& server, WebUIService* owner)
        : CppServer::HTTP::HTTPSession(server), m_owner(owner) {}

protected:
    void onReceivedRequest(const CppServer::HTTP::HTTPRequest& request) override;
    void onReceivedRequestError(const CppServer::HTTP::HTTPRequest& request, const std::string& error) override;

private:
    WebUIService* m_owner;
};

/**
 * @class InternalHTTPServer
 * @brief Inherits from CppServer to spawn custom sessions.
 */
class InternalHTTPServer : public CppServer::HTTP::HTTPServer {
public:
    InternalHTTPServer(const std::shared_ptr<CppServer::Asio::Service>& service, int port, WebUIService* owner)
        : CppServer::HTTP::HTTPServer(service, port), m_owner(owner) {}

protected:
    std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<CppServer::Asio::TCPServer>& server) override {
        return std::make_shared<InternalHTTPSession>(std::static_pointer_cast<CppServer::HTTP::HTTPServer>(server), m_owner);
    }

private:
    WebUIService* m_owner;
};

/**
 * @class WebUIService
 * @brief ActiveEntity that manages the Web UI server.
 * @feature TSK-20260311-008 Web UI Dashboard and API
 */
class WebUIService : public quasar::named::ActiveEntity {
public:
    WebUIService(const std::string& name, int port);
    ~WebUIService() override;

    void handleRequest(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request);

protected:
    void initialize() override;
    void start() override;
    void stop() override;
    void reset() override;

private:
    int m_port;
    std::shared_ptr<CppServer::Asio::Service> m_asioService;
    std::shared_ptr<InternalHTTPServer> m_server;

    // API Handlers
    void handleWalkApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request);
    void handleMetaApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request);
    void handleStaticFile(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request);
};

} // namespace quasar::webui

#endif // QUASAR_WEBUI_WEBUISERVICE_HPP