#include "quasar/scripting/Environment.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

/**
 * @brief Main execution entry point for the standalone runner.
 * @details Implements the CLI script execution and dynamic plugin loading.
 *          Refactored to use the shared Environment class.
 * 
 * @reference [TSK-20260310-001.1] Standalone Executable
 * @reference [FE-0140] Standalone Script Runner and Plug-in System
 * @contribution [TSK-20260421-001.1] Interactive Shell Executable
 */

void printUsage() {
    std::cout << "Usage: sre [options] [script_file]\n"
              << "Options:\n"
              << "  --plugin <path>   Load a dynamic plugin shared library before executing.\n"
              << "                    This option can be specified multiple times.\n"
              << "  --help, -h        Display this help message.\n"
              << "\nIf script_file is not provided, reads from stdin.\n";
}

int main(int argc, char** argv) {
    std::vector<std::string> plugins;
    std::string scriptFile;

    // Parse command line arguments
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

    // Initialize the shared execution environment
    std::shared_ptr<quasar::scripting::Environment> env = quasar::scripting::Environment::create();

    // Load plugins
    for (const std::string& pluginPath : plugins) {
        if (!env->loadPlugin(pluginPath)) {
            std::cerr << "Warning: Failed to load plugin " << pluginPath << "\n";
        }
    }

    // Execute the script
    try {
        sol::protected_function_result result;
        if (!scriptFile.empty()) {
            // Execute from file
            result = env->executeFile(scriptFile);
        } else {
            // Execute from stdin
            std::stringstream buffer;
            buffer << std::cin.rdbuf();
            result = env->executeString(buffer.str());
        }

        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "Lua Execution Error:\n" << err.what() << "\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Execution exception: " << e.what() << "\n";
        return 1;
    }

    // Environment destructor automatically handles engine shutdown
    return 0;
}
