#include "quasar/webui/WebUIService.hpp"
#include <jsoncons/json.hpp>
#include <iostream>
#include <sstream>
#include <string>

namespace quasar::webui {

// InternalHTTPSession Implementation
void InternalHTTPSession::onReceivedRequest(const CppServer::HTTP::HTTPRequest& request) {
    if (m_owner) {
        m_owner->handleRequest(std::static_pointer_cast<InternalHTTPSession>(shared_from_this()), request);
    }
}

void InternalHTTPSession::onReceivedRequestError(const CppServer::HTTP::HTTPRequest& request, const std::string& error) {
    std::cerr << "HTTP Request Error: " << error << std::endl;
}

// WebUIService Implementation
WebUIService::WebUIService(const std::string& name, int port)
    : ActiveEntity(name), m_port(port) {
}

WebUIService::~WebUIService() {
    stop();
}

void WebUIService::initialize() {
    m_asioService = std::make_shared<CppServer::Asio::Service>();
    m_server = std::make_shared<InternalHTTPServer>(m_asioService, m_port, this);
}

void WebUIService::start() {
    m_asioService->Start();
    m_server->Start();
}

void WebUIService::stop() {
    if (m_server) m_server->Stop();
    if (m_asioService) m_asioService->Stop();
}

void WebUIService::reset() {
    stop();
    initialize();
}

void WebUIService::handleRequest(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request) {
    std::string url(request.url());

    if (url.starts_with("/api/v1/walk")) {
        handleWalkApi(session, request);
    } else if (url.starts_with("/api/v1/meta")) {
        handleMetaApi(session, request);
    } else {
        handleStaticFile(session, request);
    }
}

void WebUIService::handleWalkApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request) {
    // [CS-0030.1] Feature: Stateless Tree Discovery
    // Retrieve URL string for query parameter parsing.
    std::string url(request.url());
    
    // Lambda for safe query parameter extraction without 'auto'.
    std::function<std::string(const std::string&, const std::string&, const std::string&)> getQueryParam = 
        [](const std::string& u, const std::string& p, const std::string& def) -> std::string {
        std::string search = p + "=";
        size_t pos = u.find(search);
        // If parameter found, extract value until next ampersand.
        if (pos != std::string::npos) {
            std::string val = u.substr(pos + search.length());
            size_t ampPos = val.find('&');
            // Trim at ampersand if present.
            if (ampPos != std::string::npos) val = val.substr(0, ampPos);
            return val;
        }
        return def;
    };

    // Extract walk configuration from query string.
    std::string path = getQueryParam(url, "path", "/");
    int limit = -1;
    size_t offset = 0;
    
    // Parse integer limits safely.
    try {
        std::string limitStr = getQueryParam(url, "limit", "");
        if (!limitStr.empty()) limit = std::stoi(limitStr);
        std::string offsetStr = getQueryParam(url, "offset", "");
        if (!offsetStr.empty()) offset = std::stoull(offsetStr);
    } catch (...) {
        // Fallback to defaults on parse error.
    }

    // Find the target node starting from root (parent of service).
    std::shared_ptr<quasar::named::NamedObject> current = getParent();
    
    // Simple path traversal with hard iteration limit [CS-0010.37].
    if (path != "/" && !path.empty()) {
        std::string part;
        std::stringstream ss(path);
        int depthLimit = 0;
        // Traverse tree segments delimited by slashes.
        while (std::getline(ss, part, '/') && ++depthLimit < 1024) {
            if (part.empty()) continue;
            // Descend into child if found.
            if (current) current = current->getChild(part);
        }
    }

    // Produce JSON response for the discovered node.
    jsoncons::json j;
    if (current) {
        j["status"] = "ok";
        j["tree_version"] = current->getTreeVersion();
        j["path"] = path;
        j["name"] = current->getName();
        j["type"] = current->getType();
        
        // Prepare nodes array and fetch children list.
        jsoncons::json nodes = jsoncons::json::array();
        std::list<std::shared_ptr<quasar::named::NamedObject>> children = current->getChildren();
        j["total_children"] = (uint64_t)children.size();
        
        // Skip nodes based on offset with hard limit.
        std::list<std::shared_ptr<quasar::named::NamedObject>>::iterator it = children.begin();
        size_t advanced = 0;
        // Advance iterator to offset position.
        while (it != children.end() && advanced < offset && ++advanced < 10000000) {
            ++it;
        }
        
        // Collect nodes up to the specified limit.
        int count = 0;
        // Iterate and serialize child metadata.
        while (it != children.end() && (limit == -1 || count < limit) && ++count < 10000000) {
            const std::shared_ptr<quasar::named::NamedObject>& child = *it;
            jsoncons::json node;
            node["name"] = child->getName();
            node["type"] = child->getType();
            nodes.push_back(node);
            ++it;
        }
        j["nodes"] = nodes;
        j["returned_children"] = (uint64_t)nodes.size();
    } else {
        // Report path resolution failure.
        j["status"] = "error";
        j["message"] = "Path not found";
    }

    // Build and send HTTP response.
    CppServer::HTTP::HTTPResponse response;
    response.SetBegin(200);
    response.SetHeader("Content-Type", "application/json");
    response.SetBody(j.to_string());
    session->SendResponseAsync(response);
}

void WebUIService::handleMetaApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request) {
    // [CS-0030.1] Feature: Metadata Discovery
    // Retrieve URL for path extraction.
    std::string url(request.url());
    
    // Path extraction logic without 'auto'.
    std::string path = "/";
    size_t qPos = url.find("path=");
    
    // Parse path from query parameter if present.
    if (qPos != std::string::npos) {
        path = url.substr(qPos + 5);
        size_t ampPos = path.find('&');
        if (ampPos != std::string::npos) path = path.substr(0, ampPos);
    }

    // Resolve node from the tree root.
    std::shared_ptr<quasar::named::NamedObject> current = getParent();
    
    // Simple path traversal with hard limit [CS-0010.37].
    if (path != "/" && !path.empty()) {
        std::string part;
        std::stringstream ss(path);
        int depthLimit = 0;
        // Step through path segments.
        while (std::getline(ss, part, '/') && ++depthLimit < 1024) {
            if (part.empty()) continue;
            if (current) current = current->getChild(part);
        }
    }

    // Prepare JSON metadata response.
    jsoncons::json j;
    if (current) {
        j["status"] = "ok";
        j["tree_version"] = current->getTreeVersion();
        j["name"] = current->getName();
        j["type"] = current->getType();
        
        // Include related node link if defined.
        std::shared_ptr<quasar::named::NamedObject> rel = current->getRelated();
        if (rel) {
            j["related"] = rel->getName();
        }
    } else {
        // Handle missing node.
        j["status"] = "error";
        j["message"] = "Node not found";
    }

    // Send HTTP response.
    CppServer::HTTP::HTTPResponse response;
    response.SetBegin(200);
    response.SetHeader("Content-Type", "application/json");
    response.SetBody(j.to_string());
    session->SendResponseAsync(response);
}

void WebUIService::handleStaticFile(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request) {
    CppServer::HTTP::HTTPResponse response;
    response.SetBegin(404);
    response.SetBody("Not Found");
    session->SendResponseAsync(response);
}

} // namespace quasar::webui