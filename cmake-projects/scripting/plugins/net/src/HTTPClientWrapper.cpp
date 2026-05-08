#include "quasar/net/HTTPClientWrapper.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"

namespace quasar::net {

using namespace quasar::scripting;

template <typename ClientType>
void bindClientMethods(sol::usertype<LuaProxy<ClientType>>& ut) {
    ut["connectAsync"] = [](LuaProxy<ClientType> self) { return self.lock()->ConnectAsync(); };
    ut["disconnectAsync"] = [](LuaProxy<ClientType> self) { return self.lock()->DisconnectAsync(); };
    ut["reconnectAsync"] = [](LuaProxy<ClientType> self) { return self.lock()->ReconnectAsync(); };
    ut["sendRequest"] = [](LuaProxy<ClientType> self, const std::string& method, const std::string& url, const std::string& body) {
        CppServer::HTTP::HTTPRequest req;
        req.SetBegin(method, url);
        if (!body.empty()) req.SetBody(body);
        return self.lock()->SendRequestAsync(req);
    };
    ut["get"] = [](LuaProxy<ClientType> self, const std::string& url) {
        CppServer::HTTP::HTTPRequest req;
        req.SetBegin("GET", url);
        return self.lock()->SendRequestAsync(req);
    };
    ut["post"] = [](LuaProxy<ClientType> self, const std::string& url, const std::string& body) {
        CppServer::HTTP::HTTPRequest req;
        req.SetBegin("POST", url);
        if (!body.empty()) req.SetBody(body);
        return self.lock()->SendRequestAsync(req);
    };
    ut["put"] = [](LuaProxy<ClientType> self, const std::string& url, const std::string& body) {
        CppServer::HTTP::HTTPRequest req;
        req.SetBegin("PUT", url);
        if (!body.empty()) req.SetBody(body);
        return self.lock()->SendRequestAsync(req);
    };
    ut["delete"] = [](LuaProxy<ClientType> self, const std::string& url) {
        CppServer::HTTP::HTTPRequest req;
        req.SetBegin("DELETE", url);
        return self.lock()->SendRequestAsync(req);
    };
    ut["isConnected"] = [](LuaProxy<ClientType> self) { return self.lock()->IsConnected(); };
    
    // Callbacks
    ut["onConnected"] = sol::property(
        [](LuaProxy<ClientType>& self) { return self.lock()->onConnectedCb; },
        [](LuaProxy<ClientType>& self, sol::function cb) { self.lock()->onConnectedCb = cb; }
    );
    ut["onDisconnected"] = sol::property(
        [](LuaProxy<ClientType>& self) { return self.lock()->onDisconnectedCb; },
        [](LuaProxy<ClientType>& self, sol::function cb) { self.lock()->onDisconnectedCb = cb; }
    );
    ut["onReceivedResponse"] = sol::property(
        [](LuaProxy<ClientType>& self) { return self.lock()->onReceivedResponseCb; },
        [](LuaProxy<ClientType>& self, sol::function cb) { self.lock()->onReceivedResponseCb = cb; }
    );
    ut["onError"] = sol::property(
        [](LuaProxy<ClientType>& self) { return self.lock()->onErrorCb; },
        [](LuaProxy<ClientType>& self, sol::function cb) { self.lock()->onErrorCb = cb; }
    );
}

void bindHTTPClient(sol::state_view& lua) {
    sol::table clientTable = lua["quasar"]["net"]["client"].get_or_create<sol::table>();

    sol::usertype<LuaProxy<LuaHTTPClient>> ut = lua.new_usertype<LuaProxy<LuaHTTPClient>>("HTTPClient",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    bindClientMethods(ut);

    clientTable["HTTPClient"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port, sol::this_state L) {
            std::shared_ptr<LuaHTTPClient> ptr = std::make_shared<LuaHTTPClient>(service, address, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaHTTPClient>(ptr);
        }
    );

    sol::usertype<LuaProxy<LuaSecureHTTPClient>> utSecure = lua.new_usertype<LuaProxy<LuaSecureHTTPClient>>("SecureHTTPClient",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    bindClientMethods(utSecure);

    clientTable["SecureHTTPClient"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, const std::shared_ptr<CppServer::Asio::SSLContext>& context, const std::string& address, int port, sol::this_state L) {
            std::shared_ptr<LuaSecureHTTPClient> ptr = std::make_shared<LuaSecureHTTPClient>(service, context, address, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaSecureHTTPClient>(ptr);
        }
    );
}

} // namespace quasar::net