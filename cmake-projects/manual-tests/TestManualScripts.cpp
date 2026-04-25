#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <vector>
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/PluginLoader.hpp"
#include <cstdlib>

namespace quasar::scripting {

class ManualScriptTest : public ::testing::TestWithParam<std::filesystem::path> {
protected:
    void SetUp() override {
        // [CS-0010.44] No global setup needed for manual scripts.
    }
};

TEST_P(ManualScriptTest, Execute) {
    std::filesystem::path scriptPath = GetParam();
    std::shared_ptr<LuaEngine> engine = LuaEngine::create();
    sol::state& lua = engine->getState();
    
    std::cout << "[ManualTest] Running: " << scriptPath.filename().string() << std::endl;

    // Load available plugins.
    std::filesystem::path pluginDir(QUASAR_PLUGIN_DIR);
    if (std::filesystem::exists(pluginDir)) {
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(pluginDir)) {
            if (entry.path().extension() == ".so" || entry.path().extension() == ".dll") {
                std::string name = entry.path().filename().string();
                if (name.find("quasar_") != std::string::npos && name.find("test") == std::string::npos) {
                    PluginLoader::loadPlugin(entry.path().string(), lua);
                }
            }
        }
    }

    try {
        lua.script_file(scriptPath.string());
    } catch (const sol::error& e) {
        std::string msg = e.what();
        if (msg.find("LUA_EXIT_ERROR") != std::string::npos) {
             FAIL() << "Script " << scriptPath.filename() << " called os.exit with error: " << msg;
        } else if (msg == "LUA_EXIT_ZERO") {
            // Success exit
        } else {
            FAIL() << "Script " << scriptPath.filename() << " failed:\n" << msg;
        }
    } catch (const std::exception& e) {
        std::string msg = e.what();
        if (msg.find("LUA_EXIT_ERROR") != std::string::npos) {
             FAIL() << "Script " << scriptPath.filename() << " called os.exit with error: " << msg;
        } else if (msg == "LUA_EXIT_ZERO") {
            // Success exit
        } else {
            FAIL() << "Script " << scriptPath.filename() << " threw exception: " << msg;
        }
    }
    
    try {
        engine->executeString("if quasar and quasar.net and quasar.net.clear_trampoline then quasar.net.clear_trampoline() end");
    } catch (...) {}

    engine->shutdown();

    // Flush any pending ASIO events that were generated during shutdown.
    std::filesystem::path pluginPath = pluginDir / "libquasar_net_plugin.so";
    void* handle = PluginLoader::loadLibrary(pluginPath.string());
    
    if (!handle) {
        pluginPath = pluginDir / "quasar_net_plugin.dll";
        handle = PluginLoader::loadLibrary(pluginPath.string());
    }
    
    if (handle) {
        void* sym = PluginLoader::getSymbolAddress(handle, "quasar_net_clear_trampoline");
        if (sym) {
            auto func = reinterpret_cast<void(*)()>(sym);
            func();
        }
    }

    engine.reset(); // Explicitly release the shared_ptr before ending the scope.
}

std::vector<std::filesystem::path> GetManualScripts() {
    std::vector<std::filesystem::path> scripts;
    std::string pathStr = QUASAR_MANUAL_DIR;
    std::filesystem::path manualDir(pathStr);
    if (!std::filesystem::exists(manualDir)) return scripts;

    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(manualDir)) {
        if (entry.path().extension() == ".lua") {
            scripts.push_back(entry.path());
        }
    }
    std::sort(scripts.begin(), scripts.end());
    return scripts;
}

INSTANTIATE_TEST_SUITE_P(
    DocManual,
    ManualScriptTest,
    ::testing::ValuesIn(GetManualScripts()),
    [](const testing::TestParamInfo<ManualScriptTest::ParamType>& info) {
        std::string name = info.param.stem().string();
        if (std::isdigit(name[0])) {
            name = "Script_" + name;
        }
        std::replace_if(name.begin(), name.end(), [](char c) { return !std::isalnum(c); }, '_');
        return name;
    }
);

} // namespace quasar::scripting

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    // Use quick_exit to bypass static destructors and avoid double free corruption 
    // caused by dlopen/sol2/RTTI interactions on process exit.
    std::quick_exit(result);
    return result;
}
