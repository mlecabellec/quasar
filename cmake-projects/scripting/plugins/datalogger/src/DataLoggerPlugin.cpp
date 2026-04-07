#include "quasar/scripting/PluginContract.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include "quasar/scripting/RegistryBindings.hpp"
#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/datalogger/DataLoggerService.hpp"
#include "quasar/datalogger/CsvFileWriter.hpp"
#include "quasar/datalogger/MathFilter.hpp"
#include "quasar/datalogger/ValueThresholdTrigger.hpp"
#include <iostream>
#include <sol/sol.hpp>

using namespace quasar::scripting;
using namespace quasar::named;
using namespace quasar::datalogger;

extern "C" QUASAR_PLUGIN_EXPORT void registerPluginComponents(sol::state_view lua) {
    sol::table quasarTable = lua["quasar"].get_or_create<sol::table>();
    sol::table dlTable = quasarTable["datalogger"].get_or_create<sol::table>();

    // DataLoggerService binding
    sol::usertype<LuaProxy<DataLoggerService>> utServer = lua.new_usertype<LuaProxy<DataLoggerService>>("DataLoggerService", 
        sol::no_constructor, 
        sol::base_classes, sol::bases<ILuaProxy>());
        
    utServer["getName"] = [](LuaProxy<DataLoggerService> self) { return self.lock()->getName(); };
    utServer["start"] = [](LuaProxy<DataLoggerService> self) { self.lock()->start(); };
    utServer["stop"] = [](LuaProxy<DataLoggerService> self) { self.lock()->stop(); };
    utServer["setCycleTime"] = [](LuaProxy<DataLoggerService> self, int ms) { self.lock()->setCycleTime(std::chrono::milliseconds(ms)); };
    utServer["logEvent"] = [](LuaProxy<DataLoggerService> self, int level, const std::string& msg) {
        self.lock()->logEvent(static_cast<LogLevel>(level), msg);
    };

    dlTable["createService"] = [](const std::string& name, size_t capacity, sol::object parent) {
        auto parentPtr = extractNamedObject(parent);
        auto ptr = DataLoggerService::create(name, capacity, parentPtr);
        ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<DataLoggerService>(ptr);
    };

    // CsvFileWriter binding
    sol::usertype<LuaProxy<CsvFileWriter>> utCsv = lua.new_usertype<LuaProxy<CsvFileWriter>>("CsvFileWriter", 
        sol::no_constructor,
        sol::base_classes, sol::bases<ILuaProxy>());
        
    dlTable["createCsvWriter"] = [](const std::string& name, const std::string& path, sol::object parent) {
        std::shared_ptr<quasar::named::NamedObject> parentPtr = extractNamedObject(parent);
        std::shared_ptr<CsvFileWriter> ptr = std::make_shared<CsvFileWriter>(name, path);
        if (parentPtr) {
            ptr->setParent(parentPtr);
        }
        ObjectTracker::getInstance().trackStrong(ptr);
        return LuaProxy<CsvFileWriter>(ptr);
    };

    utServer["addCsvRecorder"] = [](LuaProxy<DataLoggerService> self, LuaProxy<CsvFileWriter> recorder) {
        self.lock()->addRecorder(recorder.lock());
    };

    // MathFilter binding
    dlTable["createMathFilter"] = [](const std::string& path, double scale, double offset) -> std::shared_ptr<IFilter> {
        return std::make_shared<MathFilter>(path, scale, offset);
    };

    // ValueThresholdTrigger binding
    dlTable["createThresholdTrigger"] = [](const std::string& path, double threshold, bool above) -> std::shared_ptr<IFilter> {
        return std::make_shared<ValueThresholdTrigger>(path, threshold, above);
    };

    utServer["addFilter"] = [](LuaProxy<DataLoggerService> self, std::shared_ptr<IFilter> filter) {
        self.lock()->addFilter(filter);
    };
}
;
}
