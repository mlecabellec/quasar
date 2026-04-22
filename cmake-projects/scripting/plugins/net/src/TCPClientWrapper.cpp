#include "quasar/net/TCPClientWrapper.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include <thread>

namespace quasar::net {

using namespace quasar::scripting;

/**
 * @brief Helper to convert std::expected to a sol::object for Lua.
 */
template <typename T, typename E>
sol::object to_sol_object(const std::expected<T, E>& res, sol::state_view lua) {
    if (res.has_value()) {
        if constexpr (std::is_same_v<T, void>) {
            return sol::make_object(lua, true);
        } else {
            return sol::make_object(lua, res.value());
        }
    }
    return sol::make_object(lua, res.error());
}

// LuaTCPClient Implementation
LuaTCPClient::LuaTCPClient(const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port)
    : m_attempt(0) {
    m_client = std::make_shared<InternalClient>(service, address, port);
    m_client->owner = this;
}

std::expected<void, std::string> LuaTCPClient::connectAsync() {
    m_attempt = 0;
    if (m_client->ConnectAsync()) {
        return {};
    }
    return std::unexpected("Failed to initiate TCP connection");
}

bool LuaTCPClient::disconnectAsync() {
    return m_client->DisconnectAsync();
}

bool LuaTCPClient::sendAsync(const std::string& data) {
    return m_client->SendAsync(data);
}

void LuaTCPClient::setReconnectionPolicy(const ReconnectionPolicy& policy) {
    m_policy = policy;
}

void LuaTCPClient::InternalClient::onConnected() {
    if (owner && owner->onConnected) {
        EventTrampoline::getInstance().defer([cb = owner->onConnected]() { cb(); });
    }
}

void LuaTCPClient::InternalClient::onDisconnected() {
    if (owner && owner->onDisconnected) {
        EventTrampoline::getInstance().defer([cb = owner->onDisconnected]() { cb(); });
    }

    if (owner && owner->m_policy.enabled && (owner->m_policy.maxAttempts == -1 || owner->m_attempt < owner->m_policy.maxAttempts)) {
        std::chrono::milliseconds delay = owner->m_policy.calculateDelay(owner->m_attempt++);
        std::shared_ptr<LuaTCPClient> self = owner->shared_from_this();
        std::thread([self, delay]() {
            std::this_thread::sleep_for(delay);
            self->m_client->ConnectAsync();
        }).detach();
    }
}

void LuaTCPClient::InternalClient::onReceived(const void* buffer, size_t size) {
    if (owner && owner->onReceived) {
        std::string data(static_cast<const char*>(buffer), size);
        EventTrampoline::getInstance().defer([cb = owner->onReceived, d = std::move(data)]() { cb(d); });
    }
}

void LuaTCPClient::InternalClient::onError(int error, const std::string& category, const std::string& message) {
    (void)category;
    if (owner && owner->onError) {
        EventTrampoline::getInstance().defer([cb = owner->onError, error, msg = message]() { cb(error, msg); });
    }
}

void bindTCPClient(sol::state_view& lua) {
    sol::table clientTable = lua["quasar"]["net"]["client"].get_or_create<sol::table>();

    // TCPClient Usertype
    sol::usertype<LuaProxy<LuaTCPClient>> ut = lua.new_usertype<LuaProxy<LuaTCPClient>>("TCPClient",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    ut["connect"] = [](LuaProxy<LuaTCPClient> self, sol::this_state L) { 
        return to_sol_object(self.lock()->connectAsync(), sol::state_view(L)); 
    };
    ut["disconnect"] = [](LuaProxy<LuaTCPClient> self) { return self.lock()->disconnectAsync(); };
    ut["send"] = [](LuaProxy<LuaTCPClient> self, const std::string& data) { return self.lock()->sendAsync(data); };
    ut["setReconnectionPolicy"] = [](LuaProxy<LuaTCPClient> self, const ReconnectionPolicy& policy) { 
        self.lock()->setReconnectionPolicy(policy); 
    };

    // Callbacks
    ut["onConnected"] = [](LuaProxy<LuaTCPClient> self, sol::function cb) { self.lock()->onConnected = cb; };
    ut["onDisconnected"] = [](LuaProxy<LuaTCPClient> self, sol::function cb) { self.lock()->onDisconnected = cb; };
    ut["onReceived"] = [](LuaProxy<LuaTCPClient> self, sol::function cb) { self.lock()->onReceived = cb; };
    ut["onError"] = [](LuaProxy<LuaTCPClient> self, sol::function cb) { self.lock()->onError = cb; };

    clientTable["TCPClient"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port, sol::this_state L) {
            std::shared_ptr<LuaTCPClient> ptr = std::make_shared<LuaTCPClient>(service, address, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaTCPClient>(ptr);
        }
    );
}

} // namespace quasar::net
