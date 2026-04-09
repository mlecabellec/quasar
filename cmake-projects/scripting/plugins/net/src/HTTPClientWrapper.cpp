#include "quasar/net/HTTPClientWrapper.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"

namespace quasar::net {

using namespace quasar::scripting;

void bindHTTPClient(sol::state_view& lua) {
    auto clientTable = lua["quasar"]["net"]["client"].get_or_create<sol::table>();

    sol::usertype<LuaProxy<LuaHTTPClient>> ut = lua.new_usertype<LuaProxy<LuaHTTPClient>>("HTTPClient",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    clientTable["HTTPClient"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port) {
            auto ptr = std::make_shared<LuaHTTPClient>(service, address, port);
            ObjectTracker::getInstance().trackStrong(ptr);
            return LuaProxy<LuaHTTPClient>(ptr);
        }
    );

    ut["connectAsync"] = [](LuaProxy<LuaHTTPClient> self) { return self.lock()->ConnectAsync(); };
    ut["disconnectAsync"] = [](LuaProxy<LuaHTTPClient> self) { return self.lock()->DisconnectAsync(); };
    ut["reconnectAsync"] = [](LuaProxy<LuaHTTPClient> self) { return self.lock()->ReconnectAsync(); };
    ut["sendRequest"] = [](LuaProxy<LuaHTTPClient> self, const std::string& method, const std::string& url, const std::string& body) {
        CppServer::HTTP::HTTPRequest req;
        req.SetBegin(method, url);
        if (!body.empty()) req.SetBody(body);
        return self.lock()->SendRequestAsync(req);
    };
    ut["isConnected"] = [](LuaProxy<LuaHTTPClient> self) { return self.lock()->IsConnected(); };
    
    // Callbacks
    ut["onConnected"] = sol::property(
        [](LuaProxy<LuaHTTPClient>& self) { return self.lock()->onConnectedCb; },
        [](LuaProxy<LuaHTTPClient>& self, sol::function cb) { self.lock()->onConnectedCb = cb; }
    );
    ut["onDisconnected"] = sol::property(
        [](LuaProxy<LuaHTTPClient>& self) { return self.lock()->onDisconnectedCb; },
        [](LuaProxy<LuaHTTPClient>& self, sol::function cb) { self.lock()->onDisconnectedCb = cb; }
    );
    ut["onReceivedResponse"] = sol::property(
        [](LuaProxy<LuaHTTPClient>& self) { return self.lock()->onReceivedResponseCb; },
        [](LuaProxy<LuaHTTPClient>& self, sol::function cb) { self.lock()->onReceivedResponseCb = cb; }
    );
    ut["onError"] = sol::property(
        [](LuaProxy<LuaHTTPClient>& self) { return self.lock()->onErrorCb; },
        [](LuaProxy<LuaHTTPClient>& self, sol::function cb) { self.lock()->onErrorCb = cb; }
    );
}

} // namespace quasar::net
