#include "quasar/scripting/LuaEngine.hpp"
#include <iostream>
#include <thread>
#include <chrono>

extern "C" void registerPluginComponents(sol::state_view& lua);

int main() {
    try {
        std::shared_ptr<quasar::scripting::LuaEngine> engine = quasar::scripting::LuaEngine::create();
        sol::state& view = engine->getState();
        registerPluginComponents(view);
        
        std::cout << "Running Networking Sandbox Test (with WebSockets)..." << std::endl;
        sol::protected_function_result result = engine->executeString(R"LUA(
            local net = quasar.net
            local service = net.server.AsioService.new()
            service:start()

            print("[Server] Creating WebServer on port 8086")
            local server = net.server.WebServer.new(service, 8086)
            
            local ws_received_data = ""
            server.onWSConnected = function(id, url)
                print("[Server] WS Connected: " .. id .. " at " .. url)
            end
            server.onWSReceived = function(id, data)
                print("[Server] WS Received from " .. id .. ": " .. data)
                ws_received_data = data
                server:sendText(id, "WS_ECHO: " .. data)
            end
            
            if server:start() ~= true then
                error("Server failed to start")
            end
            
            print("[Client] Creating WSClient to 127.0.0.1:8086")
            local client = net.client.WSClient.new(service, "127.0.0.1", 8086)
            
            local client_received_echo = false
            client:onWSReceived(function(data)
                print("[Client] WS Received: " .. data)
                if data == "WS_ECHO: Hello WebSocket!" then
                    client_received_echo = true
                end
            end)
            
            client:onConnected(function()
                print("[Client] TCP Connected")
            end)

            client:onWSConnected(function(url)
                print("[Client] WS Connected to: " .. url)
                client:send("Hello WebSocket!")
            end)

            if client:connect() ~= true then
                error("Client failed to connect")
            end
            
            -- Poll for 1 second
            local start = os.clock()
            while os.clock() - start < 1.0 do
                quasar.net.poll()
            end
            
            client:disconnect()
            server:stop()
            
            if client_received_echo then
                print("WebSocket Sandbox TEST PASSED")
            else
                print("WebSocket Sandbox TEST FAILED")
                error("Did not receive WS echo")
            end
            
            return true
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
