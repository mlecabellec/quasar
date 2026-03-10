#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/PluginLoader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

/**
 * @brief Main execution entry point for the standalone runner.
 * @details Implements the CLI script execution and dynamic plugin loading.
 *          Contributes to [FE-0140].
 */

void printUsage() {
    std::cout << "Usage: sre [options] [script_file]\n"
              << "Options:\n"
              << "  --plugin <path>   Load a dynamic plugin shared library before executing.\n"
              << "                    This option can be specified multiple times.\n"
              << "\nIf script_file is not provided, reads from stdin.\n";
}

int main(int argc, char** argv) {
    std::vector<std::string> plugins;
    std::string scriptFile;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--plugin") {
            if (i + 1 < argc) {
                plugins.push_back(argv[++i]);
            } else {
                std::cerr << "Error: --plugin requires a path argument.\n";
                printUsage();
                return 1;
            }
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        } else {
            if (scriptFile.empty()) {
                scriptFile = arg;
            } else {
                std::cerr << "Error: Unexpected argument '" << arg << "'\n";
                printUsage();
                return 1;
            }
        }
    }

    quasar::scripting::LuaEngine engine;
    sol::state& lua = engine.getState();

    for (const auto& pluginPath : plugins) {
        if (!quasar::scripting::PluginLoader::loadPlugin(pluginPath, lua)) {
            std::cerr << "Warning: Failed to load plugin " << pluginPath << "\n";
        }
    }

    std::string code;
    if (!scriptFile.empty()) {
        std::ifstream file(scriptFile);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open script file " << scriptFile << "\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        code = buffer.str();
    } else {
        std::stringstream buffer;
        buffer << std::cin.rdbuf();
        code = buffer.str();
    }

    try {
        auto result = engine.executeString(code);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "Lua Execution Error:\n" << err.what() << "\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Execution exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
