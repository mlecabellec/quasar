#include "quasar/webui/WebUIService.hpp"
#include "quasar/named/WebNamedMethod.hpp"
#include "quasar/named/NamedMethod.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedString.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <functional>
#include <expected>
#include <fstream>
#include <filesystem>

namespace quasar::webui {

using namespace quasar::named;

// --- InternalHTTPSession Implementation ---

void InternalHTTPSession::onReceivedRequest(const CppServer::HTTP::HTTPRequest& request) {
    if (m_owner) {
        // [CS-0010.44] Delegate HTTP request handling.
        m_owner->handleRequest(std::static_pointer_cast<InternalHTTPSession>(shared_from_this()), request);
    }
}

void InternalHTTPSession::onReceivedRequestError(const CppServer::HTTP::HTTPRequest& request, const std::string& error) {
    (void)request;
    std::cerr << "[WebUI] HTTP Request Error: " << error << std::endl;
}

// --- InternalHTTPServer Implementation ---

std::shared_ptr<CppServer::Asio::TCPSession> InternalHTTPServer::CreateSession(const std::shared_ptr<CppServer::Asio::TCPServer>& server) {
    // [CS-0010.44] Factory for HTTP sessions.
    return std::make_shared<InternalHTTPSession>(std::static_pointer_cast<InternalHTTPServer>(server), m_owner);
}

// --- InternalWSSession Implementation ---

void InternalWSSession::onWSConnected(const CppServer::HTTP::HTTPRequest& request) {
    (void)request;
    if (m_owner) {
        // [CS-0010.44] Register new WebSocket client.
        m_owner->registerWSSession(std::static_pointer_cast<InternalWSSession>(shared_from_this()));
    }
}

void InternalWSSession::onWSDisconnected() {
    if (m_owner) {
        // [CS-0010.44] Unregister disconnected WebSocket client.
        m_owner->unregisterWSSession(std::static_pointer_cast<InternalWSSession>(shared_from_this()));
    }
}

void InternalWSSession::onWSReceived(const void* buffer, size_t size) {
    // [CS-0030.1] Feature: WebSocket Command Processing
    try {
        std::string payload(static_cast<const char*>(buffer), size);
        jsoncons::json j = jsoncons::json::parse(payload);
        
        if (!j.contains("action")) { return; }
        std::string action = j["action"].as_string();

        if (action == "subscribe") {
            // Handle subscription commands.
            if (m_owner && j.contains("path")) {
                m_owner->handleWSSubscription(std::static_pointer_cast<InternalWSSession>(shared_from_this()), j["path"].as_string());
            }
        } else if (action == "set") {
            // [CS-0030.1] Feature: Acknowledged Async Set
            if (m_owner && j.contains("path") && j.contains("value")) {
                std::string path = j["path"].as_string();
                std::string reqId = j.contains("requestID") ? j["requestID"].as_string() : "unknown";
                
                // Resolve the target node via helper.
                std::expected<std::shared_ptr<NamedObject>, std::string> res = m_owner->resolvePath(path);
                
                jsoncons::json ack;
                ack["action"] = "set_ack";
                ack["requestID"] = reqId;

                if (res.has_value()) {
                    // Node found, attempt to set value.
                    std::expected<void, std::string> setRes = m_owner->setNodeValue(res.value(), j["value"]);
                    if (setRes.has_value()) {
                        ack["status"] = "ok";
                    } else {
                        ack["status"] = "error";
                        ack["message"] = setRes.error();
                    }
                } else {
                    ack["status"] = "error";
                    ack["message"] = res.error();
                }
                
                SendTextAsync(ack.to_string());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[WebUI] WS Parse Error: " << e.what() << std::endl;
    }
}

// --- InternalWSServer Implementation ---

std::shared_ptr<CppServer::Asio::TCPSession> InternalWSServer::CreateSession(const std::shared_ptr<CppServer::Asio::TCPServer>& server) {
    // [CS-0010.44] Factory for WebSocket sessions.
    return std::make_shared<InternalWSSession>(std::static_pointer_cast<InternalWSServer>(server), m_owner);
}

// --- WebUIService Implementation ---

std::shared_ptr<WebUIService> WebUIService::create(const std::string& name, int port, std::shared_ptr<NamedObject> parent) {
    // [CS-0010.44] Factory helper for shared_ptr instantiation with self-reference.
    struct Helper : public WebUIService {
        Helper(const std::string& n, int p) : WebUIService(n, p) {}
    };
    std::shared_ptr<Helper> service = std::make_shared<Helper>(name, port);
    service->setSelf(service);
    if (parent) {
        service->setParent(parent);
    }
    return service;
}

WebUIService::WebUIService(const std::string& name, int port)
    : ActiveEntity(name), m_port(port) {
}

WebUIService::~WebUIService() {
    stop();
}

void WebUIService::initialize() {
    // [CS-0010.44] Setup network infrastructure.
    m_asioService = std::make_shared<CppServer::Asio::Service>();
    m_httpServer = std::make_shared<InternalHTTPServer>(m_asioService, m_port, this);
    m_wsServer = std::make_shared<InternalWSServer>(m_asioService, m_port + 1, this);
}

void WebUIService::start() {
    m_asioService->Start();
    m_httpServer->Start();
    m_wsServer->Start();
    
    // [CS-0010.44] Subscribe to the parent hierarchy for reflexive updates.
    std::shared_ptr<NamedObject> root = getParent();
    if (root) {
        root->subscribe(std::static_pointer_cast<WebUIService>(this->getSelf()));
    }

    // [CS-0030.1] Feature: Throttled Delta Engine
    m_deltaWorker = std::make_unique<std::jthread>([this](std::stop_token st) {
        this->deltaWorkerLoop(st);
    });
}

void WebUIService::stop() {
    if (m_deltaWorker) {
        m_deltaWorker->request_stop();
        m_deltaWorker->join();
    }
    if (m_httpServer) { m_httpServer->Stop(); }
    if (m_wsServer) { m_wsServer->Stop(); }
    if (m_asioService) { m_asioService->Stop(); }
}

void WebUIService::reset() {
    stop();
    initialize();
}

void WebUIService::notify(std::shared_ptr<NamedObject> obj) {
    if (!obj) { return; }
    std::lock_guard<std::recursive_timed_mutex> lock(m_deltaMutex);
    m_pendingDeltas.insert(obj);
}

void WebUIService::deltaWorkerLoop(std::stop_token stopToken) {
    uint64_t iterationCount = 0;
    while (!stopToken.stop_requested() && ++iterationCount < 10000000000ULL) {
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            flushDeltas();
        } catch (const std::exception& e) {
            std::cerr << "[WebUI] EXCEPTION in Delta Worker loop: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "[WebUI] UNKNOWN EXCEPTION in Delta Worker loop." << std::endl;
        }
    }
}

void WebUIService::flushDeltas() {
    std::set<std::shared_ptr<NamedObject>> deltasToFlush;
    {
        std::lock_guard<std::recursive_timed_mutex> lock(m_deltaMutex);
        if (m_pendingDeltas.empty()) { return; }
        deltasToFlush.swap(m_pendingDeltas);
    }

    jsoncons::json batch;
    batch["action"] = "batch_update";
    jsoncons::json updates = jsoncons::json::array();

    for (const std::shared_ptr<NamedObject>& obj : deltasToFlush) {
        jsoncons::json nodeUpdate;
        nodeUpdate["name"] = obj->getName();
        nodeUpdate["type"] = obj->getType();
        nodeUpdate["path"] = "/"; 
        updates.push_back(nodeUpdate);
    }
    batch["updates"] = updates;

    std::string payload = batch.to_string();
    std::lock_guard<std::recursive_timed_mutex> lock(m_wsMutex);
    for (const std::shared_ptr<InternalWSSession>& session : m_wsSessions) {
        session->SendTextAsync(payload);
    }
}

void WebUIService::registerWSSession(std::shared_ptr<InternalWSSession> session) {
    std::lock_guard<std::recursive_timed_mutex> lock(m_wsMutex);
    m_wsSessions.insert(session);
}

void WebUIService::unregisterWSSession(std::shared_ptr<InternalWSSession> session) {
    std::lock_guard<std::recursive_timed_mutex> lock(m_wsMutex);
    m_wsSessions.erase(session);
    m_subscriptions.erase(session->id().string());
}

void WebUIService::handleWSSubscription(std::shared_ptr<InternalWSSession> session, const std::string& path) {
    std::lock_guard<std::recursive_timed_mutex> lock(m_wsMutex);
    m_subscriptions[session->id().string()].insert(path);
    jsoncons::json ack;
    ack["action"] = "subscribed";
    ack["path"] = path;
    session->SendTextAsync(ack.to_string());
}

std::expected<std::shared_ptr<NamedObject>, std::string> WebUIService::resolvePath(const std::string& path) {
    std::shared_ptr<NamedObject> current = getParent();
    if (!current) { return std::unexpected("Root not available"); }
    if (path == "/" || path.empty()) { return current; }
    std::string part;
    std::stringstream ss(path);
    int depthLimit = 0;
    while (std::getline(ss, part, '/') && ++depthLimit < 1024) {
        if (part.empty()) { continue; }
        std::shared_ptr<NamedObject> child = current->getChild(part);
        if (!child) { return std::unexpected("Node not found: " + part); }
        current = child;
    }
    return current;
}

std::expected<void, std::string> WebUIService::setNodeValue(std::shared_ptr<NamedObject> obj, const jsoncons::json& value) {
    std::string type = obj->getType();
    if (type == "NamedInteger") {
        std::shared_ptr<NamedInteger<int64_t>> intNode = std::dynamic_pointer_cast<NamedInteger<int64_t>>(obj);
        if (intNode && value.is_int64()) { intNode->setValue(value.as_integer<int64_t>()); return {}; }
    } else if (type == "NamedBoolean") {
        std::shared_ptr<NamedBoolean> boolNode = std::dynamic_pointer_cast<NamedBoolean>(obj);
        if (boolNode && value.is_bool()) { boolNode->setValue(value.as_bool()); return {}; }
    } else if (type == "NamedFloatingPoint") {
        std::shared_ptr<NamedFloatingPoint<double>> fpNode = std::dynamic_pointer_cast<NamedFloatingPoint<double>>(obj);
        if (fpNode && value.is_double()) { fpNode->setValue(value.as_double()); return {}; }
    }
    return std::unexpected("Value type mismatch or node not settable: " + type);
}

void WebUIService::handleRequest(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request) {
    // [CS-0010.20] Global resilience boundary.
    try {
        std::string url(request.url());
        
        // Log incoming request for diagnostics.
        // std::cout << "[WebUI] Request: " << request.method() << " " << url << std::endl;

        // [CS-0010.44] Route requests based on URI prefixes.
        if (url.starts_with("/api/v1/walk")) {
            handleWalkApi(session, request);
        } else if (url.starts_with("/api/v1/meta")) {
            handleMetaApi(session, request);
        } else if (url.starts_with("/api/v1/set")) {
            handleSetApi(session, request);
        } else if (url.starts_with("/api/v1/induced")) {
            handleInducedApi(session, request);
        } else if (url == "/openapi.json" || url == "/api/v1/openapi.json") {
            handleOpenApi(session, request);
        } else {
            handleStaticFile(session, request);
        }
    } catch (const std::exception& e) {
        // [CS-0010.20] Catch all to prevent service crash.
        std::cerr << "[WebUI] CRITICAL FAULT during request handling: " << e.what() << std::endl;
        
        // Return 500 error to client.
        CppServer::HTTP::HTTPResponse response;
        response.SetBegin(500);
        response.SetHeader("Content-Type", "application/json");
        response.SetBody("{\"status\":\"error\",\"message\":\"Internal Server Error: check logs\"}");
        session->SendResponseAsync(response);
    } catch (...) {
        std::cerr << "[WebUI] UNKNOWN CRITICAL FAULT during request handling." << std::endl;
    }
}

void WebUIService::handleWalkApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request) {
    std::string url(request.url());
    std::function<std::string(const std::string&, const std::string&, const std::string&)> getQueryParam = 
        [](const std::string& u, const std::string& p, const std::string& def) -> std::string {
        std::string search = p + "=";
        size_t pos = u.find(search);
        if (pos != std::string::npos) {
            std::string val = u.substr(pos + search.length());
            size_t ampPos = val.find('&');
            if (ampPos != std::string::npos) { val = val.substr(0, ampPos); }
            return val;
        }
        return def;
    };
    std::string path = getQueryParam(url, "path", "/");
    int limit = -1;
    size_t offset = 0;
    try {
        std::string limitStr = getQueryParam(url, "limit", "");
        if (!limitStr.empty()) { limit = std::stoi(limitStr); }
        std::string offsetStr = getQueryParam(url, "offset", "");
        if (!offsetStr.empty()) { offset = std::stoull(offsetStr); }
    } catch (...) {}
    std::shared_ptr<NamedObject> current = getParent();
    if (path != "/" && !path.empty()) {
        std::string part;
        std::stringstream ss(path);
        int depthLimit = 0;
        while (std::getline(ss, part, '/') && ++depthLimit < 1024) {
            if (part.empty()) { continue; }
            if (current) { current = current->getChild(part); }
        }
    }
    jsoncons::json j;
    if (current) {
        j["status"] = "ok";
        j["tree_version"] = current->getTreeVersion();
        j["path"] = path;
        j["name"] = current->getName();
        j["type"] = current->getType();
        jsoncons::json nodes = jsoncons::json::array();
        std::list<std::shared_ptr<NamedObject>> children = current->getChildren();
        j["total_children"] = (uint64_t)children.size();
        std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin();
        size_t advanced = 0;
        while (it != children.end() && advanced < offset && ++advanced < 10000000) { ++it; }
        int count = 0;
        while (it != children.end() && (limit == -1 || count < limit) && ++count < 10000000) {
            jsoncons::json node;
            node["name"] = (*it)->getName();
            node["type"] = (*it)->getType();
            nodes.push_back(node);
            ++it;
        }
        j["nodes"] = nodes;
        j["returned_children"] = (uint64_t)nodes.size();
    } else {
        j["status"] = "error";
        j["message"] = "Path not found";
    }
    CppServer::HTTP::HTTPResponse response;
    response.SetBegin(200);
    response.SetHeader("Content-Type", "application/json");
    response.SetBody(j.to_string());
    session->SendResponseAsync(response);
}

void WebUIService::handleMetaApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request) {
    std::string url(request.url());
    std::string path = "/";
    size_t qPos = url.find("path=");
    if (qPos != std::string::npos) {
        path = url.substr(qPos + 5);
        size_t ampPos = path.find('&');
        if (ampPos != std::string::npos) { path = path.substr(0, ampPos); }
    }
    std::shared_ptr<NamedObject> current = getParent();
    if (path != "/" && !path.empty()) {
        std::string part;
        std::stringstream ss(path);
        int depthLimit = 0;
        while (std::getline(ss, part, '/') && ++depthLimit < 1024) {
            if (part.empty()) { continue; }
            if (current) { current = current->getChild(part); }
        }
    }
    jsoncons::json j;
    if (current) {
        j["status"] = "ok";
        j["tree_version"] = current->getTreeVersion();
        j["name"] = current->getName();
        j["type"] = current->getType();

        // [CS-0010.44] Export value for primitive types.
        std::string type = current->getType();
        if (type == "NamedInteger") {
             std::shared_ptr<NamedInteger<int64_t>> n = std::dynamic_pointer_cast<NamedInteger<int64_t>>(current);
             if (n) j["value"] = n->value();
        } else if (type == "NamedBoolean") {
             std::shared_ptr<NamedBoolean> n = std::dynamic_pointer_cast<NamedBoolean>(current);
             if (n) j["value"] = n->booleanValue();
        } else if (type == "NamedFloatingPoint") {
             std::shared_ptr<NamedFloatingPoint<double>> n = std::dynamic_pointer_cast<NamedFloatingPoint<double>>(current);
             if (n) j["value"] = n->value();
        } else if (type == "NamedString") {
             std::shared_ptr<NamedString> n = std::dynamic_pointer_cast<NamedString>(current);
             if (n) j["value"] = n->toString();
        }

        std::shared_ptr<NamedObject> rel = current->getRelated();
        if (rel) j["related"] = rel->getName();
    } else {
        j["status"] = "error";
        j["message"] = "Node not found";
    }
    CppServer::HTTP::HTTPResponse response;
    response.SetBegin(200);
    response.SetHeader("Content-Type", "application/json");
    response.SetBody(j.to_string());
    session->SendResponseAsync(response);
}

void WebUIService::handleSetApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request) {
    // [CS-0030.1] Feature: REST Set API
    std::string url(request.url());
    std::function<std::string(const std::string&, const std::string&, const std::string&)> getQueryParam = 
        [](const std::string& u, const std::string& p, const std::string& def) -> std::string {
        std::string search = p + "=";
        size_t pos = u.find(search);
        if (pos != std::string::npos) {
            std::string val = u.substr(pos + search.length());
            size_t ampPos = val.find('&');
            if (ampPos != std::string::npos) { val = val.substr(0, ampPos); }
            return val;
        }
        return def;
    };
    
    std::string path = getQueryParam(url, "path", "/");
    jsoncons::json responseBody;
    
    try {
        std::string body(request.body());
        jsoncons::json j = jsoncons::json::parse(body);
        
        if (j.contains("value")) {
            std::expected<std::shared_ptr<NamedObject>, std::string> res = resolvePath(path);
            if (res.has_value()) {
                std::expected<void, std::string> setRes = setNodeValue(res.value(), j["value"]);
                if (setRes.has_value()) {
                    responseBody["status"] = "ok";
                } else {
                    responseBody["status"] = "error";
                    responseBody["message"] = setRes.error();
                }
            } else {
                responseBody["status"] = "error";
                responseBody["message"] = res.error();
            }
        } else {
            responseBody["status"] = "error";
            responseBody["message"] = "Missing 'value' field in request body";
        }
    } catch (const std::exception& e) {
        responseBody["status"] = "error";
        responseBody["message"] = std::string("Parse Error: ") + e.what();
    }

    CppServer::HTTP::HTTPResponse response;
    response.SetBegin(200);
    response.SetHeader("Content-Type", "application/json");
    response.SetBody(responseBody.to_string());
    session->SendResponseAsync(response);
}

void WebUIService::handleOpenApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request) {
    (void)request;
    jsoncons::json oas;
    oas["openapi"] = "3.0.0";
    oas["info"]["title"] = "Quasar Reflexive API";
    oas["info"]["version"] = "1.0.0";
    jsoncons::json paths = jsoncons::json::object();
    collectOpenApiPaths(getParent(), paths, 0);
    oas["paths"] = paths;
    CppServer::HTTP::HTTPResponse response;
    response.SetBegin(200);
    response.SetHeader("Content-Type", "application/json");
    response.SetHeader("Access-Control-Allow-Origin", "*");
    response.SetBody(oas.to_string());
    session->SendResponseAsync(response);
}

void WebUIService::collectOpenApiPaths(std::shared_ptr<NamedObject> root, jsoncons::json& paths, int depth) {
    if (depth > 1024 || !root) { return; }
    if (root->getType() == "WebNamedMethod") {
        std::shared_ptr<WebNamedMethod> webMethod = std::dynamic_pointer_cast<WebNamedMethod>(root);
        if (webMethod) {
            std::string alias = webMethod->getAlias();
            if (alias.empty()) { alias = "/" + root->getName(); }
            std::string verb = webMethod->getHttpVerb();
            std::transform(verb.begin(), verb.end(), verb.begin(), [](unsigned char c) { return (unsigned char)std::tolower(c); });
            std::string fullPath = "/api/v1/induced" + alias;
            jsoncons::json operation;
            operation["summary"] = webMethod->getOasSummary();
            operation["responses"]["200"]["description"] = "Successful execution";
            paths[fullPath][verb] = operation;
        }
    }
    std::list<std::shared_ptr<NamedObject>> children = root->getChildren();
    std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin();
    int limit = 0;
    while (it != children.end() && ++limit < 1000000) {
        collectOpenApiPaths(*it, paths, depth + 1);
        ++it;
    }
}

void WebUIService::handleInducedApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request) {
    std::string url(request.url());
    std::string prefix = "/api/v1/induced";
    std::string targetUri = "";
    if (url.length() >= prefix.length()) { targetUri = url.substr(prefix.length()); }
    size_t qPos = targetUri.find('?');
    if (qPos != std::string::npos) { targetUri = targetUri.substr(0, qPos); }
    std::shared_ptr<NamedObject> methodNode = findWebMethod(getParent(), targetUri, 0);
    jsoncons::json responseBody;
    if (methodNode) {
        std::shared_ptr<WebNamedMethod> webMethod = std::dynamic_pointer_cast<WebNamedMethod>(methodNode);
        std::string reqMethod(request.method());
        bool verbMatch = true;
        if (webMethod && reqMethod != webMethod->getHttpVerb()) { verbMatch = false; }
        if (verbMatch) {
            std::shared_ptr<ICommand> cmd = std::dynamic_pointer_cast<ICommand>(methodNode);
            if (cmd) {
                std::shared_ptr<NamedObject> result = cmd->execute(nullptr);
                responseBody["status"] = "ok";
                if (result) {
                    responseBody["result"] = result->getName();
                    responseBody["result_type"] = result->getType();
                }
            } else {
                responseBody["status"] = "error";
                responseBody["message"] = "Node is not executable";
            }
        } else {
            responseBody["status"] = "error";
            responseBody["message"] = "Method Not Allowed";
        }
    } else {
        responseBody["status"] = "error";
        responseBody["message"] = "Endpoint Not Found";
    }
    CppServer::HTTP::HTTPResponse response;
    response.SetBegin(200);
    response.SetHeader("Content-Type", "application/json");
    response.SetBody(responseBody.to_string());
    session->SendResponseAsync(response);
}

std::shared_ptr<NamedObject> WebUIService::findWebMethod(std::shared_ptr<NamedObject> root, const std::string& uri, int depth) {
    if (depth > 1024 || !root) { return nullptr; }
    if (root->getType() == "WebNamedMethod") {
        std::shared_ptr<WebNamedMethod> webMethod = std::dynamic_pointer_cast<WebNamedMethod>(root);
        if (webMethod && webMethod->getAlias() == uri) { return root; }
    }
    std::list<std::shared_ptr<NamedObject>> children = root->getChildren();
    std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin();
    int limit = 0;
    while (it != children.end() && ++limit < 1000000) {
        std::shared_ptr<NamedObject> found = findWebMethod(*it, uri, depth + 1);
        if (found) { return found; }
        ++it;
    }
    return nullptr;
}

void WebUIService::setWebRoot(const std::string& path) {
    m_webRoot = path;
}

void WebUIService::handleStaticFile(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request) {
    // [CS-0010.44] Resolve physical path on disk.
    std::string urlPath(request.url());
    // Strip query parameters.
    size_t qPos = urlPath.find('?');
    if (qPos != std::string::npos) { urlPath = urlPath.substr(0, qPos); }
    
    // Default to index.html for root.
    if (urlPath == "/" || urlPath.empty()) { urlPath = "/index.html"; }

    std::filesystem::path fullPath = std::filesystem::path(m_webRoot) / urlPath.substr(1);
    
    if (std::filesystem::exists(fullPath) && std::filesystem::is_regular_file(fullPath)) {
        // [CS-0010.22] RAII for file handle.
        std::ifstream file(fullPath, std::ios::binary);
        if (file) {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            
            CppServer::HTTP::HTTPResponse response;
            response.SetBegin(200);
            
            // Simple MIME detection based on extension.
            std::string ext = fullPath.extension().string();
            if (ext == ".html") response.SetHeader("Content-Type", "text/html");
            else if (ext == ".js") response.SetHeader("Content-Type", "application/javascript");
            else if (ext == ".css") response.SetHeader("Content-Type", "text/css");
            else if (ext == ".json") response.SetHeader("Content-Type", "application/json");
            else if (ext == ".svg") response.SetHeader("Content-Type", "image/svg+xml");
            else if (ext == ".png") response.SetHeader("Content-Type", "image/png");
            
            response.SetBody(content);
            session->SendResponseAsync(response);
            return;
        }
    }

    // Fallback for SPA routing: serve index.html for non-file paths.
    std::filesystem::path indexPath = std::filesystem::path(m_webRoot) / "index.html";
    if (urlPath != "/index.html" && std::filesystem::exists(indexPath)) {
         std::ifstream file(indexPath, std::ios::binary);
         if (file) {
             std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
             CppServer::HTTP::HTTPResponse response;
             response.SetBegin(200);
             response.SetHeader("Content-Type", "text/html");
             response.SetBody(content);
             session->SendResponseAsync(response);
             return;
         }
    }

    CppServer::HTTP::HTTPResponse response;
    response.SetBegin(404);
    response.SetBody("Not Found");
    session->SendResponseAsync(response);
}

} // namespace quasar::webui
