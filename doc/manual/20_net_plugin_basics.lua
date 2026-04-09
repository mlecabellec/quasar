-- 20_net_plugin_basics.lua
-- The Operator: Raw TCP Networking.
-- "Sometimes you just need to talk in plain text without the OPC UA bureaucracy."

io.stdout:setvbuf("no")

print("--- Quasar: Net Plugin (TCP Echo Server) ---")
if not quasar.net then
    print("ERROR: Net plugin not loaded. Run with '--plugin build/lib/quasar_net_plugin.so'")
    os.exit(1)
end

-- 1. Create the underlying ASIO Service
local asio = quasar.net.server.AsioService()
asio:start()

-- 2. Create the TCP Server on port 8080
local server = quasar.net.server.TCPServer(asio, 8080)

-- 3. Define Callbacks
-- "A good server always knows who's talking and what they're saying."
server.onConnected = function(session_id)
    print("[Server] Client connected: " .. session_id)
    server:sendAsync(session_id, "Welcome to Quasar TCP Echo Server!\n")
end

server.onDisconnected = function(session_id)
    print("[Server] Client disconnected: " .. session_id)
end

server.onReceived = function(session_id, data)
    -- Remove trailing newlines for cleaner logging
    local cleanData = string.gsub(data, "\r?\n$", "")
    print("[Server] Received from " .. session_id .. ": '" .. cleanData .. "'")
    
    -- Echo it back
    server:sendAsync(session_id, "Echo: " .. data)
    
    -- Hidden feature to stop the server
    if cleanData == "quit" or cleanData == "exit" then
        print("[Server] Quit command received.")
        server:disconnect(session_id)
    end
end

server.onError = function(err)
    print("[Server] ERROR: " .. err)
end

-- 4. Start the Server
print("Starting TCP Server on port 8080...")
local started = server:start()

if started then
    print("Server is listening. (You can test with 'nc localhost 8080')")
    print("Waiting 5 seconds before shutting down...")
    
    -- Keep alive for 5 seconds
    if quasar.sleep then quasar.sleep(5000) else os.execute("sleep 5") end
else
    print("FAILED to start server.")
end

-- 5. Cleanup
print("Shutting down server and ASIO service...")
server:stop()
asio:stop()

print("--- Net Plugin Finished ---")
-- "Connection closed by foreign host. (Which is us, we closed it.)"
