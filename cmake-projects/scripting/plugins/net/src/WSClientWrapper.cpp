#include "quasar/net/WSClientWrapper.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "string/encoding.h"
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

// LuaWSClient Implementation
LuaWSClient::LuaWSClient(const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port)
    : m_attempt(0) {
    m_client = std::make_shared<LuaWSClient::InternalClient>(service, address, port);
}

std::expected<void, std::string> LuaWSClient::connectAsync() {
    m_attempt = 0;
    m_client->owner = shared_from_this();
    if (m_client->ConnectAsync()) {
        return {};
    }
    return std::unexpected("Failed to initiate WS connection");
}

bool LuaWSClient::disconnectAsync() {
    return m_client->DisconnectAsync();
}

bool LuaWSClient::sendTextAsync(const std::string& text) {
    return m_client->SendTextAsync(text);
}

void LuaWSClient::setReconnectionPolicy(const ReconnectionPolicy& policy) {
    m_policy = policy;
}

void LuaWSClient::InternalClient::onConnected() {
    std::cout << "[CPP Debug] InternalClient::onConnected triggered" << std::endl;
    CppServer::WS::WSClient::onConnected();
    std::shared_ptr<LuaWSClient> o = owner.lock();
    if (o) {
        std::lock_guard<std::recursive_mutex> lock(o->m_callbackMutex);
        if (o->onConnected) {
            EventTrampoline::getInstance().defer([cb = o->onConnected]() { cb(); });
        }
    }
}

void LuaWSClient::InternalClient::onDisconnected() {
    std::cout << "[CPP Debug] InternalClient::onDisconnected triggered" << std::endl;
    CppServer::WS::WSClient::onDisconnected();
    std::shared_ptr<LuaWSClient> o = owner.lock();
    if (o) {
        std::lock_guard<std::recursive_mutex> lock(o->m_callbackMutex);
        if (o->onDisconnected) {
            EventTrampoline::getInstance().defer([cb = o->onDisconnected]() { cb(); });
        }
        
        if (o->m_policy.enabled && (o->m_policy.maxAttempts == -1 || o->m_attempt < o->m_policy.maxAttempts)) {
            std::chrono::milliseconds delay = o->m_policy.calculateDelay(o->m_attempt++);
            std::thread([o, delay]() {
                std::this_thread::sleep_for(delay);
                o->m_client->ConnectAsync();
            }).detach();
        }
    }
}

void LuaWSClient::InternalClient::onWSConnecting(CppServer::HTTP::HTTPRequest& request) {
    std::cout << "[CPP Debug] InternalClient::onWSConnecting triggered" << std::endl;
    request.SetBegin("GET", "/");
    request.SetHeader("Host", address());
    request.SetHeader("Upgrade", "websocket");
    request.SetHeader("Connection", "Upgrade");
    request.SetHeader("Sec-WebSocket-Key", CppCommon::Encoding::Base64Encode(ws_nonce()));
    request.SetHeader("Sec-WebSocket-Version", "13");
    CppServer::WS::WSClient::onWSConnecting(request);
}

void LuaWSClient::InternalClient::onWSConnected(const CppServer::HTTP::HTTPResponse& response) {
    std::cout << "[CPP Debug] InternalClient::onWSConnected triggered for session " << id().string() << ", status: " << response.status() << std::endl;
    std::shared_ptr<LuaWSClient> o = owner.lock();
    if (o) {
        std::lock_guard<std::recursive_mutex> lock(o->m_callbackMutex);
        if (o->onWSConnected) {
            std::string status = std::to_string(response.status());
            EventTrampoline::getInstance().defer([cb = o->onWSConnected, s = std::move(status)]() { cb(s); });
        }
    }
}

void LuaWSClient::InternalClient::onWSDisconnected() {
    std::cout << "[CPP Debug] InternalClient::onWSDisconnected triggered" << std::endl;
    std::shared_ptr<LuaWSClient> o = owner.lock();
    if (o) {
        std::lock_guard<std::recursive_mutex> lock(o->m_callbackMutex);
        if (o->onWSDisconnected) {
            EventTrampoline::getInstance().defer([cb = o->onWSDisconnected]() { cb(); });
        }
    }
}

void LuaWSClient::InternalClient::onWSReceived(const void* buffer, size_t size) {
    std::cout << "[CPP Debug] InternalClient::onWSReceived triggered, size: " << size << std::endl;
    std::shared_ptr<LuaWSClient> o = owner.lock();
    if (o) {
        std::lock_guard<std::recursive_mutex> lock(o->m_callbackMutex);
        if (o->onWSReceived) {
            std::string data(static_cast<const char*>(buffer), size);
            EventTrampoline::getInstance().defer([cb = o->onWSReceived, d = std::move(data)]() { cb(d); });
        }
    }
}

void LuaWSClient::InternalClient::onReceived(const void* buffer, size_t size) {
    CppServer::WS::WSClient::onReceived(buffer, size);
}

void LuaWSClient::InternalClient::onError(int error, const std::string& category, const std::string& message) {
    (void)category;
    std::shared_ptr<LuaWSClient> o = owner.lock();
    if (o) {
        std::lock_guard<std::recursive_mutex> lock(o->m_callbackMutex);
        if (o->onError) {
            EventTrampoline::getInstance().defer([cb = o->onError, error, msg = message]() { cb(error, msg); });
        }
    }
}

// LuaSecureWSClient Implementation
LuaSecureWSClient::LuaSecureWSClient(const std::shared_ptr<CppServer::Asio::Service>& service, const std::shared_ptr<CppServer::Asio::SSLContext>& context, const std::string& address, int port)
    : m_attempt(0) {
    m_client = std::make_shared<LuaSecureWSClient::InternalClient>(service, context, address, port);
}

std::expected<void, std::string> LuaSecureWSClient::connectAsync() {
    m_attempt = 0;
    m_client->owner = shared_from_this();
    if (m_client->ConnectAsync()) {
        return {};
    }
    return std::unexpected("Failed to initiate secure WS connection");
}

bool LuaSecureWSClient::disconnectAsync() {
    return m_client->DisconnectAsync();
}

bool LuaSecureWSClient::sendTextAsync(const std::string& text) {
    return m_client->SendTextAsync(text);
}

void LuaSecureWSClient::setReconnectionPolicy(const ReconnectionPolicy& policy) {
    m_policy = policy;
}

void LuaSecureWSClient::InternalClient::onConnected() {
    CppServer::WS::WSSClient::onConnected();
    std::shared_ptr<LuaSecureWSClient> o = owner.lock();
    if (o) {
        std::lock_guard<std::recursive_mutex> lock(o->m_callbackMutex);
        if (o->onConnected) {
            EventTrampoline::getInstance().defer([cb = o->onConnected]() { cb(); });
        }
    }
}

void LuaSecureWSClient::InternalClient::onDisconnected() {
    CppServer::WS::WSSClient::onDisconnected();
    std::shared_ptr<LuaSecureWSClient> o = owner.lock();
    if (o) {
        std::lock_guard<std::recursive_mutex> lock(o->m_callbackMutex);
        if (o->onDisconnected) {
            EventTrampoline::getInstance().defer([cb = o->onDisconnected]() { cb(); });
        }

        if (o->m_policy.enabled && (o->m_policy.maxAttempts == -1 || o->m_attempt < o->m_policy.maxAttempts)) {
            std::chrono::milliseconds delay = o->m_policy.calculateDelay(o->m_attempt++);
            std::thread([o, delay]() {
                std::this_thread::sleep_for(delay);
                o->m_client->ConnectAsync();
            }).detach();
        }
    }
}

void LuaSecureWSClient::InternalClient::onWSConnecting(CppServer::HTTP::HTTPRequest& request) {
    std::cout << "[CPP Debug] Secure InternalClient::onWSConnecting triggered" << std::endl;
    request.SetBegin("GET", "/");
    request.SetHeader("Host", address());
    request.SetHeader("Upgrade", "websocket");
    request.SetHeader("Connection", "Upgrade");
    request.SetHeader("Sec-WebSocket-Key", CppCommon::Encoding::Base64Encode(ws_nonce()));
    request.SetHeader("Sec-WebSocket-Version", "13");
    CppServer::WS::WSSClient::onWSConnecting(request);
}

void LuaSecureWSClient::InternalClient::onWSConnected(const CppServer::HTTP::HTTPResponse& response) {
    std::shared_ptr<LuaSecureWSClient> o = owner.lock();
    if (o) {
        std::lock_guard<std::recursive_mutex> lock(o->m_callbackMutex);
        if (o->onWSConnected) {
            std::string status = std::to_string(response.status());
            EventTrampoline::getInstance().defer([cb = o->onWSConnected, s = std::move(status)]() { cb(s); });
        }
    }
}

void LuaSecureWSClient::InternalClient::onWSDisconnected() {
    std::shared_ptr<LuaSecureWSClient> o = owner.lock();
    if (o) {
        std::lock_guard<std::recursive_mutex> lock(o->m_callbackMutex);
        if (o->onWSDisconnected) {
            EventTrampoline::getInstance().defer([cb = o->onWSDisconnected]() { cb(); });
        }
    }
}

void LuaSecureWSClient::InternalClient::onWSReceived(const void* buffer, size_t size) {
    std::shared_ptr<LuaSecureWSClient> o = owner.lock();
    if (o) {
        std::lock_guard<std::recursive_mutex> lock(o->m_callbackMutex);
        if (o->onWSReceived) {
            std::string data(static_cast<const char*>(buffer), size);
            EventTrampoline::getInstance().defer([cb = o->onWSReceived, d = std::move(data)]() { cb(d); });
        }
    }
}

void LuaSecureWSClient::InternalClient::onReceived(const void* buffer, size_t size) {
    CppServer::WS::WSSClient::onReceived(buffer, size);
}

void LuaSecureWSClient::InternalClient::onError(int error, const std::string& category, const std::string& message) {
    (void)category;
    std::shared_ptr<LuaSecureWSClient> o = owner.lock();
    if (o) {
        std::lock_guard<std::recursive_mutex> lock(o->m_callbackMutex);
        if (o->onError) {
            EventTrampoline::getInstance().defer([cb = o->onError, error, msg = message]() { cb(error, msg); });
        }
    }
}

void bindWSClient(sol::state_view& lua) {
    sol::table clientTable = lua["quasar"]["net"]["client"].get_or_create<sol::table>();

    // WSClient Usertype
    sol::usertype<LuaProxy<LuaWSClient>> ut = lua.new_usertype<LuaProxy<LuaWSClient>>("WSClient",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    ut["connect"] = [](LuaProxy<LuaWSClient> self, sol::this_state L) { 
        return to_sol_object(self.lock()->connectAsync(), sol::state_view(L)); 
    };
    ut["disconnect"] = [](LuaProxy<LuaWSClient> self) { return self.lock()->disconnectAsync(); };
    ut["send"] = [](LuaProxy<LuaWSClient> self, const std::string& text) { return self.lock()->sendTextAsync(text); };
    ut["setReconnectionPolicy"] = [](LuaProxy<LuaWSClient> self, const ReconnectionPolicy& p) { self.lock()->setReconnectionPolicy(p); };

    // Callbacks
    ut["onConnected"] = [](LuaProxy<LuaWSClient> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onConnected = cb; 
    };
    ut["onDisconnected"] = [](LuaProxy<LuaWSClient> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onDisconnected = cb; 
    };
    ut["onWSConnected"] = [](LuaProxy<LuaWSClient> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onWSConnected = cb; 
    };
    ut["onWSDisconnected"] = [](LuaProxy<LuaWSClient> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onWSDisconnected = cb; 
    };
    ut["onWSReceived"] = [](LuaProxy<LuaWSClient> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onWSReceived = cb; 
    };
    ut["onError"] = [](LuaProxy<LuaWSClient> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onError = cb; 
    };

    clientTable["WSClient"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, const std::string& address, int port, sol::this_state L) {
            std::shared_ptr<LuaWSClient> ptr = std::make_shared<LuaWSClient>(service, address, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaWSClient>(ptr);
        }
    );

    // SecureWSClient Usertype
    sol::usertype<LuaProxy<LuaSecureWSClient>> uts = lua.new_usertype<LuaProxy<LuaSecureWSClient>>("SecureWSClient",
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());

    uts["connect"] = [](LuaProxy<LuaSecureWSClient> self, sol::this_state L) { 
        return to_sol_object(self.lock()->connectAsync(), sol::state_view(L)); 
    };
    uts["disconnect"] = [](LuaProxy<LuaSecureWSClient> self) { return self.lock()->disconnectAsync(); };
    uts["send"] = [](LuaProxy<LuaSecureWSClient> self, const std::string& text) { return self.lock()->sendTextAsync(text); };
    uts["setReconnectionPolicy"] = [](LuaProxy<LuaSecureWSClient> self, const ReconnectionPolicy& p) { self.lock()->setReconnectionPolicy(p); };

    // Callbacks
    uts["onConnected"] = [](LuaProxy<LuaSecureWSClient> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onConnected = cb; 
    };
    uts["onDisconnected"] = [](LuaProxy<LuaSecureWSClient> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onDisconnected = cb; 
    };
    uts["onWSConnected"] = [](LuaProxy<LuaSecureWSClient> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onWSConnected = cb; 
    };
    uts["onWSDisconnected"] = [](LuaProxy<LuaSecureWSClient> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onWSDisconnected = cb; 
    };
    uts["onWSReceived"] = [](LuaProxy<LuaSecureWSClient> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onWSReceived = cb; 
    };
    uts["onError"] = [](LuaProxy<LuaSecureWSClient> self, sol::function cb) { 
        std::lock_guard<std::recursive_mutex> lock(self.lock()->m_callbackMutex);
        self.lock()->onError = cb; 
    };

    clientTable["SecureWSClient"] = lua.create_table_with(
        "new", [](const std::shared_ptr<CppServer::Asio::Service>& service, const std::shared_ptr<CppServer::Asio::SSLContext>& context, const std::string& address, int port, sol::this_state L) {
            std::shared_ptr<LuaSecureWSClient> ptr = std::make_shared<LuaSecureWSClient>(service, context, address, port);
            ObjectTracker::getInstance().trackStrong(getEngineId(L), ptr);
            return LuaProxy<LuaSecureWSClient>(ptr);
        }
    );
}

} // namespace quasar::net
