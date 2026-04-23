#include "quasar/scripting/LuaEngine.hpp"
#include <sol/sol.hpp>
#include <iostream>
#include <chrono>
#include <thread>

extern "C" void registerPluginComponents(sol::state_view& lua);

int main() {
    try {
        std::shared_ptr<quasar::scripting::LuaEngine> engine = quasar::scripting::LuaEngine::create();
        sol::state& view = engine->getState();
        registerPluginComponents(view);
        
        std::cout << "Running Networking Sandbox Test (Handshake Diagnosis)..." << std::endl;
        sol::protected_function_result result = engine->executeString(R"LUA(
            local net = quasar.net
            local service = net.server.AsioService.new()
            service:start()

            print("[Server] Creating WebServer on port 8086")
            local server = net.server.WebServer.new(service, 8086)
            
            local client_received_echo = false

            server:onWSConnected(function(id)
                print("[Server] WS Handshake Complete for session: " .. tostring(id))
            end)

            server:onWSReceived(function(id, data)
                print("[Server] WS Received from " .. tostring(id) .. ": " .. tostring(data))
                server:sendText(id, "WS_ECHO: " .. data)
            end)
            
            if server:start() ~= true then
                error("Server failed to start")
            end
            
            print("[Client] Creating WSClient to 127.0.0.1:8086")
            local client = net.client.WSClient.new(service, "127.0.0.1", 8086)
            
            client:onWSReceived(function(data)
                print("[Client] WS Received: " .. tostring(data))
                if data == "WS_ECHO: Hello WebSocket!" then
                    client_received_echo = true
                end
            end)
            
            client:onConnected(function()
                print("[Client] TCP Connected, awaiting handshake...")
            end)

            client:onWSConnected(function(status)
                print("[Client] WS Handshake Successful, status: " .. tostring(status))
                client:send("Hello WebSocket!")
            end)

            client:onError(function(err, msg)
                print("[Client] ERROR: " .. tostring(err) .. " - " .. tostring(msg))
            end)

            if client:connect() ~= true then
                error("Client failed to initiate connection")
            end
            
            -- Polling loop with hard iteration limit and manual time tracking
            print("[Info] Starting poll loop (Max 5s)...")
            local success = false
            for i = 1, 5000 do
                net.poll()
                if client_received_echo then
                    success = true
                    break
                end
                if quasar.sleep then quasar.sleep(1) end
            end
            
            client:disconnect()
            server:stop()
            
            if success then
                print("WebSocket Sandbox TEST PASSED")
                return true
            else
                print("WebSocket Sandbox TEST FAILED (Timeout)")
                error("Handshake or Echo failed")
            end
        )LUA");
        
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "Script error: " << err.what() << std::endl;
            engine->shutdown();
            return 1;
        }
        
        engine->shutdown();

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
