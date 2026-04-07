io.stdout:setvbuf("no")

local function log(msg)
    print("[Lua] " .. msg)
end

log("Starting Lua DataLogger Test")

local root = quasar.named.createObject("root")
local service = quasar.datalogger.createService("MyDataLogger", 1000, root)
local csvWriter = quasar.datalogger.createCsvWriter("CsvWriter", "lua_datalogger_out.csv", service)

service:addCsvRecorder(csvWriter)

local filter = quasar.datalogger.createMathFilter("Sensor1", 2.0, 10.0)
service:addFilter(filter)

service:start()

service:logEvent(1, "Service started from Lua")

os.execute("sleep 1")

service:stop()
log("Test finished")
