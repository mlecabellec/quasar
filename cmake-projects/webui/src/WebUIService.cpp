#include "quasar/webui/WebUIService.hpp"
#include <jsoncons/json.hpp>
#include <iostream>

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
    jsoncons::json j;
    j["status"] = "ok";
    j["nodes"] = jsoncons::json::array();

    CppServer::HTTP::HTTPResponse response;
    response.SetBegin(200);
    response.SetHeader("Content-Type", "application/json");
    response.SetBody(j.to_string());
    session->SendResponseAsync(response);
}

void WebUIService::handleMetaApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request) {
    jsoncons::json j;
    j["status"] = "ok";

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