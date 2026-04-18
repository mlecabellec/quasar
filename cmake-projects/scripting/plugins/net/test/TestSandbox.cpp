#include "quasar/scripting/LuaEngine.hpp"
#include <iostream>
#include <thread>
#include <chrono>

extern "C" void registerPluginComponents(sol::state_view& lua);

int main() {
    try {
        // [CS-0010.6] Use factory method for LuaEngine.
        std::shared_ptr<quasar::scripting::LuaEngine> engine = quasar::scripting::LuaEngine::create();
        sol::state& view = engine->getState();
        registerPluginComponents(view);
        
        std::cout << "Running Networking Sandbox Test..." << std::endl;
        auto result = engine->executeString(R"LUA(
            local net = quasar.net
            print("Creating ASIO Service")
            local service = net.server.AsioService.new()
            service:start()

            print("Creating TCP Server on port 8085")
            local server = net.server.TCPServer.new(service, 8085)
            
            server.onConnected = function(id)
                local status, err = pcall(function()
                    print("[Server] Client connected: " .. id); io.flush()
                end)
                if not status then print("[Server] onConnected ERROR: " .. tostring(err)); io.flush() end
            end
            
            server.onReceived = function(id, data)
                local status, err = pcall(function()
                    print("[Server] Received from " .. tostring(id) .. " (type " .. type(id) .. "), data: " .. tostring(data) .. " (type " .. type(data) .. " )"); io.flush()
                    if type(data) == "string" then
                        local sent = server:sendAsync(id, "ECHO: " .. data)
                        if not sent then
                            error("server:sendAsync returned false for session " .. tostring(id))
                        else
                            print("[Server] sendAsync succeeded!"); io.flush()
                        end
                    else
                        error("Expected data to be a string, got " .. type(data))
                    end
                end)
                if not status then print("[Server] onReceived ERROR: " .. tostring(err)); io.flush() end
            end
            
            if not server:start() then
                error("Server failed to start")
            end
            
            print("Creating TCP Client to 127.0.0.1:8085")
            local client = net.client.TCPClient.new(service, "127.0.0.1", 8085)
            
            local received_echo = false
            client.onConnected = function()
                local status, err = pcall(function()
                    print("[Client] Connected to server!"); io.flush()
                    client:sendAsync("Hello Quasar NET!")
                end)
                if not status then print("[Client] onConnected ERROR: " .. tostring(err)); io.flush() end
            end
            
            client.onReceived = function(data)
                local status, err = pcall(function()
                    print("[Client] Received: " .. data); io.flush()
                    if data == "ECHO: Hello Quasar NET!" then
                        received_echo = true
                    end
                end)
                if not status then print("[Client] onReceived ERROR: " .. tostring(err)); io.flush() end
            end
            
            if not client:connectAsync() then
                error("Client failed to connectAsync")
            end
            
            -- Busy poll loop for 2 seconds to process callbacks
            local start = os.clock()
            while os.clock() - start < 1.0 do
                quasar.net.poll()
            end
            
            client:disconnectAsync()
            server:stop()
            
            if received_echo then
                print("Sandbox TEST PASSED")
            else
                print("Sandbox TEST FAILED")
                error("Did not receive echo")
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
