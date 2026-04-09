-- 24_csv_exporter.lua
-- The Accountant: Exporting to CSV.
-- "Data is beautiful, but only when you can open it in Excel."

io.stdout:setvbuf("no")

local n = quasar.named

print("--- Quasar: CSV Exporter ---")
if not quasar.datalogger then
    print("ERROR: DataLogger plugin not loaded.")
    os.exit(1)
end

local root = n.createObject("ExportSystem")

-- 1. Create the CsvFileWriter
-- "Writing to disk. The ultimate commitment."
print("Creating CSV Exporter for 'sensor_data.csv'...")
local csvWriter = quasar.datalogger.createCsvWriter("CsvExporter", "sensor_data.csv", root)

-- In a complete implementation, the CsvFileWriter would subscribe to an IDataAccessor.
-- Here, we demonstrate the initialization and lifecycle.
print("CSV Exporter initialized successfully.")

-- 2. Mocking Data Generation
print("Simulating data generation...")
local entries = 0
local exporterHook = n.createLuaMethod("MockExport", function(owner, args)
    entries = entries + 1
    print(string.format("[CSV Exporter] Mocking write for row %d: %.2f, %d", entries, 22.5 + entries, entries * 10))
    return nil
end, root)

for i = 1, 3 do
    exporterHook:execute(nil)
end

print("--- CSV Exporter Finished ---")
