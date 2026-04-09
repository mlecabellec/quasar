#include "quasar/net/WebServerWrapper.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"

namespace quasar::net {

using namespace quasar::scripting;

void LuaWSSession::onConnected() {
    auto id_str = id().string();
    if (m_serverOwner) m_serverOwner->registerSession(id_str, std::static_pointer_cast<LuaWSSession>(shared_from_this()));
    if (onConnectedCb && *onConnectedCb) {
        EventTrampoline::getInstance().defer([cb = *onConnectedCb, id = std::move(id_str)]() { cb(id); });
    }
}

void LuaWSSession::onDisconnected() {
    auto id_str = id().string();
    if (onDisconnectedCb && *onDisconnectedCb) {
        EventTrampoline::getInstance().defer([cb = *onDisconnectedCb, id = id_str]() { cb(id); });
    }
    if (m_serverOwner) m_serverOwner->unregisterSession(id_str);
}

void LuaWSSession::onReceivedRequest(const CppServer::HTTP::HTTPRequest& request) {
    if (onReceivedRequestCb && *onReceivedRequestCb) {
        auto id_str = id().string();
        std::string method = std::string(request.method());
        std::string url = std::string(request.url());
        std::string body = std::string(request.body());
        EventTrampoline::getInstance().defer([cb = *onReceivedRequestCb, id = std::move(id_str), method = std::move(method), url = std::move(url), body = std::move(body)]() {
            cb(id, method, url, body);
        });
    }
}

void LuaWSSession::onWSConnected(const CppServer::HTTP::HTTPRequest& request) {
    if (onWSConnectedCb && *onWSConnectedCb) {
        auto id_str = id().string();
        std::string url = std::string(request.url());
        EventTrampoline::getInstance().defer([cb = *onWSConnectedCb, id = std::move(id_str), url = std::move(url)]() { cb(id, url); });
    }
}

void LuaWSSession::onWSDisconnected() {
    if (onWSDisconnectedCb && *onWSDisconnectedCb) {
        auto id_str = id().string();
        EventTrampoline::getInstance().defer([cb = *onWSDisconnectedCb, id = std::move(id_str)]() { cb(id); });
    }
}

void LuaWSSession::onWSReceived(const void* buffer, size_t size) {
    if (onWSReceivedCb && *onWSReceivedCb) {
        auto id_str = id().string();
        std::string data(static_cast<const char*>(buffer), size);
        EventTrampoline::getInstance().defer([cb = *onWSReceivedCb, id = std::move(id_str), data = std::move(data)]() { cb(id, data); });
    }
}

void LuaWSSession::onError(int error, const std::string& category, const std::string& message) {
    if (onErrorCb && *onErrorCb) {
        auto id_str = id().string();
        EventTrampoline::getInstance().defer([cb = *onErrorCb, id = std::move(id_str), error, message]() { cb(id, error, message); });
    }
}

void bindWebServer(sol::state_view& lua) {
    auto serverTable = lua["quasar"]["net"]["server"].get_or_create<sol::table>();

    sol::usertype<LuaProxy<LuaWSServer>> ut = lua.new_usertype<LuaProxy<LuaWSServer>>("WebServer",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    serverTable["WebServer"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, int port) {
            auto ptr = std::make_shared<LuaWSServer>(service, port);
            ObjectTracker::getInstance().trackStrong(ptr);
            return LuaProxy<LuaWSServer>(ptr);
        }
    );

    ut["start"] = [](LuaProxy<LuaWSServer> self) { return self.lock()->Start(); };
    ut["stop"] = [](LuaProxy<LuaWSServer> self) { return self.lock()->Stop(); };
    ut["restart"] = [](LuaProxy<LuaWSServer> self) { return self.lock()->Restart(); };
    ut["multicastText"] = [](LuaProxy<LuaWSServer> self, const std::string& data) { 
        return self.lock()->MulticastText(data.data(), data.size()); 
    };
    ut["multicastBinary"] = [](LuaProxy<LuaWSServer> self, const std::string& data) { 
        return self.lock()->MulticastBinary(data.data(), data.size()); 
    };
    ut["sendText"] = [](LuaProxy<LuaWSServer> self, const std::string& id, const std::string& data) {
         auto session = self.lock()->getLuaSession(id);
         if (session) session->SendTextAsync(data.data(), data.size());
    };
    ut["sendBinary"] = [](LuaProxy<LuaWSServer> self, const std::string& id, const std::string& data) {
         auto session = self.lock()->getLuaSession(id);
         if (session) session->SendBinaryAsync(data.data(), data.size());
    };
    ut["sendResponse"] = [](LuaProxy<LuaWSServer> self, const std::string& id, int status, const std::string& message, const std::string& body) {
         auto session = self.lock()->getLuaSession(id);
         if (session) {
             CppServer::HTTP::HTTPResponse response;
             response.SetBegin(status, message);
             response.SetBody(body);
             session->SendResponseAsync(response);
         }
    };
    
    // Callbacks
    ut["onConnected"] = sol::property(
        [](LuaProxy<LuaWSServer>& self) { return self.lock()->onConnectedCb; },
        [](LuaProxy<LuaWSServer>& self, sol::function cb) { self.lock()->onConnectedCb = cb; }
    );
    ut["onDisconnected"] = sol::property(
        [](LuaProxy<LuaWSServer>& self) { return self.lock()->onDisconnectedCb; },
        [](LuaProxy<LuaWSServer>& self, sol::function cb) { self.lock()->onDisconnectedCb = cb; }
    );
    ut["onReceivedRequest"] = sol::property(
        [](LuaProxy<LuaWSServer>& self) { return self.lock()->onReceivedRequestCb; },
        [](LuaProxy<LuaWSServer>& self, sol::function cb) { self.lock()->onReceivedRequestCb = cb; }
    );
    ut["onWSConnected"] = sol::property(
        [](LuaProxy<LuaWSServer>& self) { return self.lock()->onWSConnectedCb; },
        [](LuaProxy<LuaWSServer>& self, sol::function cb) { self.lock()->onWSConnectedCb = cb; }
    );
    ut["onWSDisconnected"] = sol::property(
        [](LuaProxy<LuaWSServer>& self) { return self.lock()->onWSDisconnectedCb; },
        [](LuaProxy<LuaWSServer>& self, sol::function cb) { self.lock()->onWSDisconnectedCb = cb; }
    );
    ut["onWSReceived"] = sol::property(
        [](LuaProxy<LuaWSServer>& self) { return self.lock()->onWSReceivedCb; },
        [](LuaProxy<LuaWSServer>& self, sol::function cb) { self.lock()->onWSReceivedCb = cb; }
    );
    ut["onError"] = sol::property(
        [](LuaProxy<LuaWSServer>& self) { return self.lock()->onErrorCb; },
        [](LuaProxy<LuaWSServer>& self, sol::function cb) { self.lock()->onErrorCb = cb; }
    );
}

} // namespace quasar::net
