#ifndef QUASAR_WEBUI_WEBUISERVICE_HPP
#define QUASAR_WEBUI_WEBUISERVICE_HPP

#include "quasar/named/ActiveEntity.hpp"
#include "quasar/named/IObserver.hpp"
#include "server/http/http_server.h"
#include "server/http/http_session.h"
#include "server/ws/ws_server.h"
#include "server/ws/ws_session.h"
#include <jsoncons/json.hpp>
#include <memory>
#include <string>
#include "quasar/webui/IResourceProvider.hpp"
#include <vector>
#include <memory>
#include <expected>
#include <mutex>
#include <map>
#include <set>
#include <expected>
#include <thread>

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
 * @brief Inherits from CppServer to spawn custom HTTP sessions.
 */
class InternalHTTPServer : public CppServer::HTTP::HTTPServer {
public:
    InternalHTTPServer(const std::shared_ptr<CppServer::Asio::Service>& service, int port, WebUIService* owner)
        : CppServer::HTTP::HTTPServer(service, port), m_owner(owner) {}

protected:
    std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<CppServer::Asio::TCPServer>& server) override;

private:
    WebUIService* m_owner;
};

/**
 * @class InternalWSSession
 * @brief Handles real-time WebSocket communication for the WebUI.
 */
class InternalWSSession : public CppServer::WS::WSSession {
public:
    InternalWSSession(const std::shared_ptr<CppServer::WS::WSServer>& server, WebUIService* owner)
        : CppServer::WS::WSSession(server), m_owner(owner) {}

protected:
    void onWSConnected(const CppServer::HTTP::HTTPRequest& request) override;
    void onWSDisconnected() override;
    void onWSReceived(const void* buffer, size_t size) override;

private:
    WebUIService* m_owner;
};

/**
 * @class InternalWSServer
 * @brief Inherits from CppServer to spawn custom WebSocket sessions.
 */
class InternalWSServer : public CppServer::WS::WSServer {
public:
    InternalWSServer(const std::shared_ptr<CppServer::Asio::Service>& service, int port, WebUIService* owner)
        : CppServer::WS::WSServer(service, port), m_owner(owner) {}

protected:
    std::shared_ptr<CppServer::Asio::TCPSession> CreateSession(const std::shared_ptr<CppServer::Asio::TCPServer>& server) override;

private:
    WebUIService* m_owner;
};

/**
 * @class WebUIService
 * @brief ActiveEntity that manages the Web UI server and WebSocket synchronization.
 * @feature TSK-20260311-008 Web UI Dashboard and API
 */
class WebUIService : public quasar::named::ActiveEntity, public quasar::named::IObserver {
public:
    /**
     * @brief Factory method.
     */
    static std::shared_ptr<WebUIService> create(const std::string& name, int port, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    WebUIService(const std::string& name, int port);
    ~WebUIService() override;

    /** @brief Returns "WebUIService" */
    std::string getType() const override { return "WebUIService"; }

    // ActiveEntity Lifecycle
    void initialize() override;
    void start() override;
    void stop() override;
    void reset() override;

    // IObserver implementation
    /**
     * @brief Pushes updates to WebSocket subscribers when a node changes.
     * @param eventData The changed object.
     * @feature TSK-20260311-008.2 Real-time Sync
     */
    void notify(std::shared_ptr<quasar::named::NamedObject> eventData) override;

    // API Handlers
    void handleRequest(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request);
    void handleWalkApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request);
    void handleMetaApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request);
    void handleSetApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request);
    
    /**
     * @brief Handles requests to the induced API (Dynamic Method Invocation).
     * @param session The HTTP session.
     * @param request The HTTP request.
     * @feature TSK-20260311-008.1 Dynamic Induced Routing
     * @exposed
     */
    void handleInducedApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request);

    /**
     * @brief Generates and serves the OpenAPI 3.0.x specification for the system.
     * @param session The HTTP session.
     * @param request The HTTP request.
     * @feature TSK-20260311-008.1 Self-Documentation
     * @exposed
     */
    void handleOpenApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request);

    /**
     * @brief Serves a static file from the local filesystem.
     */
    void handleStaticFile(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request);

    /**
     * @brief Sets the filesystem path to the static web assets.
     * @param path Path to the directory containing index.html.
     */
    void setWebRoot(const std::string& path);

    /**
     * @brief Adds a resource provider for static files.
     * @param provider The provider to add.
     * @feature TSK-20260424-002.3 Dynamic Resource Discovery
     */
    void addResourceProvider(std::unique_ptr<IResourceProvider> provider);

    // Session Management
    void registerWSSession(std::shared_ptr<InternalWSSession> session);
    void unregisterWSSession(std::shared_ptr<InternalWSSession> session);
    void handleWSSubscription(std::shared_ptr<InternalWSSession> session, const std::string& path);

    /**
     * @brief Resolves a string path to a NamedObject.
     * @param path The path to resolve (relative to parent).
     * @return Expected object or error string.
     * @compliance [CS-0020.48] Error propagation via std::expected.
     */
    [[nodiscard]] std::expected<std::shared_ptr<quasar::named::NamedObject>, std::string> resolvePath(const std::string& path);

    /**
     * @brief Attempts to set a value on a node derived from JSON.
     * @param obj The target object.
     * @param value The JSON value to set.
     * @return Expected void or error string.
     */
    [[nodiscard]] std::expected<void, std::string> setNodeValue(std::shared_ptr<quasar::named::NamedObject> obj, const jsoncons::json& value);

private:
    /**
     * @brief Background worker loop for periodic delta flushing.
     * @compliance [CS-0010.37] Hard iteration limit for safety.
     */
    void deltaWorkerLoop(std::stop_token stopToken);

    /**
     * @brief Flushes pending updates to all subscribed sessions.
     */
    void flushDeltas();

    // Search Helpers
    [[nodiscard]] std::shared_ptr<quasar::named::NamedObject> findWebMethod(std::shared_ptr<quasar::named::NamedObject> root, const std::string& uri, int depth);
    void collectOpenApiPaths(std::shared_ptr<quasar::named::NamedObject> root, jsoncons::json& paths, int depth);

    int m_port;
    std::string m_webRoot;
    std::shared_ptr<CppServer::Asio::Service> m_asioService;
    std::shared_ptr<InternalHTTPServer> m_httpServer;
    std::shared_ptr<InternalWSServer> m_wsServer;

    /** @brief Guard for WebSocket session list. */
    mutable std::recursive_timed_mutex m_wsMutex;
    /** @brief Set of currently active WebSocket sessions. */
    std::set<std::shared_ptr<InternalWSSession>> m_wsSessions;

    /** @brief Map of path subscriptions per session ID. */
    std::map<std::string, std::set<std::string>> m_subscriptions;

    /** @brief Mutex for the delta buffer. */
    mutable std::recursive_timed_mutex m_deltaMutex;
    /** @brief Set of objects that have changed since the last flush. */
    std::set<std::shared_ptr<quasar::named::NamedObject>> m_pendingDeltas;

    /** @brief Mutex for resource providers. */
    mutable std::recursive_timed_mutex m_resourceMutex;
    /** @brief Registered resource providers. [CS-0010.7] unique_ptr for ownership. */
    std::vector<std::unique_ptr<IResourceProvider>> m_resourceProviders;

    /** @brief Background thread for flushing. */
    std::unique_ptr<std::jthread> m_deltaWorker;
};

} // namespace quasar::webui

#endif // QUASAR_WEBUI_WEBUISERVICE_HPP
