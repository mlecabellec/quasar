# Git History Deletion Analysis

The following method signatures were deleted in recent commits and appear to be completely absent from the current codebase.

| Commit | Deleted Signature |
| ------ | ----------------- |
| `12a51795` | `void handleRequest(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `12a51795` | `void handleWalkApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `12a51795` | `void handleMetaApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `12a51795` | `void handleSetApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `12a51795` | `void handleInducedApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `12a51795` | `void handleOpenApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `12a51795` | `void handleStaticFile(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `12a51795` | `void setWebRoot(const std::string& path)` |
| `12a51795` | `void registerWSSession(std::shared_ptr<InternalWSSession> session)` |
| `12a51795` | `void unregisterWSSession(std::shared_ptr<InternalWSSession> session)` |
| `12a51795` | `void handleWSSubscription(std::shared_ptr<InternalWSSession> session, const std::string& path)` |
| `12a51795` | `void deltaWorkerLoop(std::stop_token stopToken)` |
| `12a51795` | `void flushDeltas()` |
| `12a51795` | `void collectOpenApiPaths(std::shared_ptr<quasar::named::NamedObject> root, jsoncons::json& paths, int depth)` |
| `12a51795` | `void WebUIService::deltaWorkerLoop(std::stop_token stopToken)` |
| `12a51795` | `void WebUIService::flushDeltas()` |
| `12a51795` | `void WebUIService::registerWSSession(std::shared_ptr<InternalWSSession> session)` |
| `12a51795` | `void WebUIService::unregisterWSSession(std::shared_ptr<InternalWSSession> session)` |
| `12a51795` | `void WebUIService::handleWSSubscription(std::shared_ptr<InternalWSSession> session, const std::string& path)` |
| `12a51795` | `void WebUIService::handleRequest(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `12a51795` | `void WebUIService::handleWalkApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `12a51795` | `void WebUIService::handleMetaApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `12a51795` | `void WebUIService::handleSetApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `12a51795` | `void WebUIService::handleOpenApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `12a51795` | `void WebUIService::collectOpenApiPaths(std::shared_ptr<NamedObject> root, jsoncons::json& paths, int depth)` |
| `12a51795` | `void WebUIService::handleInducedApi(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `12a51795` | `void WebUIService::setWebRoot(const std::string& path)` |
| `12a51795` | `void WebUIService::handleStaticFile(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `8ee97b52` | `void handleRequest(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `8ee97b52` | `void WebUIService::handleRequest(std::shared_ptr<InternalHTTPSession> session, const CppServer::HTTP::HTTPRequest& request)` |
| `eabe11a5` | `void notifyConnected(const std::string& id)` |
| `eabe11a5` | `void notifyDisconnected(const std::string& id)` |
| `eabe11a5` | `void notifyReceived(const std::string& id, const std::string& data)` |
| `eabe11a5` | `void notifyConnected(const std::string& id)` |
| `eabe11a5` | `void notifyDisconnected(const std::string& id)` |
| `eabe11a5` | `void notifyReceived(const std::string& id, const std::string& data)` |
| `eabe11a5` | `void LuaWSServer::notifyConnected(const std::string& id)` |
| `eabe11a5` | `void LuaWSServer::notifyDisconnected(const std::string& id)` |
| `eabe11a5` | `void LuaWSServer::notifyReceived(const std::string& id, const std::string& data)` |
| `eabe11a5` | `void LuaSecureWSServer::notifyConnected(const std::string& id)` |
| `eabe11a5` | `void LuaSecureWSServer::notifyDisconnected(const std::string& id)` |
| `eabe11a5` | `void LuaSecureWSServer::notifyReceived(const std::string& id, const std::string& data)` |
