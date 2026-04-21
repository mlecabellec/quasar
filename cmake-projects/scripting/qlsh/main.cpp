/**
 * @file main.cpp
 * @brief Entry point for the Quasar Lua Shell (qlsh).
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260421-001.1] Interactive Shell Executable.
 * - Fulfills [TSK-20260421-001.2] Advanced Line Editing (replxx).
 * - Adheres to Lua 5.4 Spec via Contextual Injection for locals.
 * - Fulfills [CS-0010.45] Doxygen documentation.
 */

#include "quasar/scripting/Environment.hpp"
#include "quasar/datalogger/DataLoggerService.hpp"
#include <replxx.hxx>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <sstream>
#include <csignal>
#include <atomic>
#include <unistd.h>
#include <set>

#include "ShellHighlighter.hpp"
#include "ShellCompleter.hpp"
#include "ShellUtilities.hpp"

using Replxx = replxx::Replxx;
using namespace quasar::scripting;

// Global atomic flags for signal and lifecycle communication
static std::atomic<bool> g_sigint_received(false);
static std::atomic<bool> g_should_exit(false);

/**
 * @brief Signal handler for SIGINT (Ctrl+C).
 */
void handle_sigint(int sig) {
    (void)sig;
    g_sigint_received = true;
}

/**
 * @brief Lua hook to interrupt execution when Ctrl+C is pressed.
 */
void interrupt_hook(lua_State* L, lua_Debug* ar) {
    (void)ar;
    if (g_sigint_received) {
        g_sigint_received = false;
        luaL_error(L, "interrupted!");
    }
}

/**
 * @brief Synchronizes upvalues of global functions with the persistent session state.
 * @details This ensures that closures defined in previous chunks see updates to locals.
 */
const char* SYNC_SCRIPT = R"(
function __qlsh_sync()
    local debug = require('debug')
    for name, val in pairs(_G) do
        if type(val) == 'function' then
            local i = 1
            while true do
                local n, v = debug.getupvalue(val, i)
                if not n then break end
                if __qlsh_locals[n] ~= nil then
                    debug.setupvalue(val, i, __qlsh_locals[n])
                end
                i = i + 1
            end
        end
    end
end
)";

/**
 * @brief Lua function to terminate the shell.
 */
int l_exit(lua_State*) {
    g_should_exit = true;
    return 0;
}

/**
 * @brief Helper to check if a Lua statement is complete.
 */
bool isComplete(lua_State* L, const std::string& code) {
    if (code.empty()) {
        return true;
    }
    int status = luaL_loadstring(L, code.c_str());
    if (status == LUA_ERRSYNTAX) {
        std::string err = lua_tostring(L, -1);
        lua_pop(L, 1);
        if (err.size() >= 5 && err.substr(err.size() - 5) == "<eof>") {
            return false;
        }
    } else {
        lua_pop(L, 1); 
    }
    return true;
}

/**
 * @brief Scans for new 'local' definitions to track them.
 * @details Handles 'local a, b = ...' and 'local function name()'.
 */
void updateTrackedLocals(const std::string& code, std::set<std::string>& tracked) {
    std::stringstream ss(code);
    std::string line;
    while (std::getline(ss, line)) {
        size_t first = line.find_first_not_of(" \t");
        if (first == std::string::npos) continue;
        
        std::string trimmed = line.substr(first);
        if (trimmed.compare(0, 6, "local ") == 0) {
            std::string content = trimmed.substr(6);
            
            // Case: local function name(...)
            if (content.compare(0, 9, "function ") == 0) {
                size_t fStart = 9;
                while (fStart < content.size() && std::isspace(static_cast<unsigned char>(content[fStart]))) fStart++;
                size_t fEnd = content.find_first_of(" \t(", fStart);
                if (fEnd != std::string::npos) {
                    std::string name = content.substr(fStart, fEnd - fStart);
                    if (!name.empty()) tracked.insert(name);
                }
                continue;
            }
            
            // Case: local a, b, c = ...
            size_t eqPos = content.find('=');
            std::string vars = content.substr(0, eqPos);
            std::stringstream vss(vars);
            std::string var;
            while (std::getline(vss, var, ',')) {
                size_t s = var.find_first_not_of(" \t");
                size_t e = var.find_last_not_of(" \t");
                if (s != std::string::npos) {
                    tracked.insert(var.substr(s, e - s + 1));
                }
            }
        }
    }
}

void printUsage() {
    std::cout << "Usage: qlsh [options]\n"
              << "Options:\n"
              << "  --plugin <path>   Load a dynamic plugin shared library.\n"
              << "  --log <path>      Initialize global logger with the given CSV path.\n"
              << "  --help, -h        Display this help message.\n";
}

int main(int argc, char** argv) {
    std::vector<std::string> plugins;
    std::string logPath;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--plugin") {
            if (i + 1 < argc) {
                plugins.push_back(argv[++i]);
            } else {
                std::cerr << "Error: --plugin requires a path.\n";
                return 1;
            }
        } else if (arg == "--log") {
            if (i + 1 < argc) {
                logPath = argv[++i];
            } else {
                std::cerr << "Error: --log requires a path.\n";
                return 1;
            }
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        } else {
            std::cerr << "Error: Unknown argument " << arg << "\n";
            printUsage();
            return 1;
        }
    }

    if (!logPath.empty()) {
        quasar::datalogger::DataLoggerService::initDefault(logPath);
    }

    std::shared_ptr<quasar::scripting::Environment> env = quasar::scripting::Environment::create();
    sol::state& lua = env->getEngine()->getState();
    lua_State* L = lua.lua_state();
    
    // Setup Session Persistence Table
    lua["__qlsh_locals"] = lua.create_table();
    env->executeString(SYNC_SCRIPT);
    std::set<std::string> trackedLocals;

    // Bind shell utilities (ls, help)
    bindShellUtilities(lua);

    // Bind shell control commands
    lua.set_function("exit", l_exit);
    lua.set_function("quit", l_exit);

    // Register the interruption hook (check every 100 instructions)
    lua_sethook(L, interrupt_hook, LUA_MASKCOUNT, 100);

    for (const std::string& pluginPath : plugins) {
        if (!env->loadPlugin(pluginPath)) {
            std::cerr << "Warning: Failed to load plugin " << pluginPath << "\n";
        }
    }

    // Register signal handler
    std::signal(SIGINT, handle_sigint);

    Replxx rx(std::cin, std::cout, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO);
    rx.install_window_change_handler();
    
    ShellHighlighter highlighter;
    ShellCompleter completer(lua);
    rx.set_highlighter_callback(highlighter);
    rx.set_completion_callback(completer);

    rx.bind_key(Replxx::KEY::meta(Replxx::KEY::ENTER), [&](char32_t code) {
        (void)code;
        return rx.invoke(Replxx::ACTION::INSERT_CHARACTER, U'\n');
    });

    char const* home = getenv("HOME");
    std::string historyFile = (std::filesystem::path(home ? home : ".") / ".quasar_qlsh_history").string();
    rx.history_load(historyFile);

    std::cout << "🌌 Quasar Lua Shell (qlsh)\n";
    std::cout << "Type 'exit()' or press Ctrl+D to quit. Use Alt+Enter for multi-line.\n";

    std::string accumulatedInput;
    while (!g_should_exit) {
        g_sigint_received = false; 
        
        std::string prompt = accumulatedInput.empty() ? "qlsh> " : "....> ";
        char const* cinput = rx.input(prompt);
        
        if (cinput == nullptr) {
            if (g_sigint_received) {
                std::cout << "^C\n";
                accumulatedInput.clear();
                g_sigint_received = false;
                continue;
            }
            break; 
        }

        std::string input(cinput);
        if (input.empty() && accumulatedInput.empty()) {
            continue;
        }

        if (!accumulatedInput.empty()) {
            accumulatedInput += "\n";
        }
        accumulatedInput += input;

        if (isComplete(L, accumulatedInput)) {
            rx.history_add(accumulatedInput);
            
            try {
                // 1. Build the Injection Preamble
                std::string preamble = "";
                for (std::string const& name : trackedLocals) {
                    preamble += "local " + name + " = __qlsh_locals['" + name + "']\n";
                }

                // 2. Build the Capture Postamble
                std::string postamble = "\n";
                for (std::string const& name : trackedLocals) {
                    postamble += "__qlsh_locals['" + name + "'] = " + name + "\n";
                }
                // Scan input for NEW locals to track in future iterations
                updateTrackedLocals(accumulatedInput, trackedLocals);
                // Also add capture for these new ones in THIS iteration
                for (std::string const& name : trackedLocals) {
                    postamble += "__qlsh_locals['" + name + "'] = " + name + "\n";
                }

                // 3. Execution logic (Evaluate-and-Print)
                std::string fullCode = preamble + accumulatedInput + postamble;

                // Try as expression first
                std::string expr = "return " + preamble + accumulatedInput;
                sol::protected_function_result result = env->executeString(expr);
                
                if (result.valid()) {
                    // It was an expression. Run it again with postamble to save state.
                    env->executeString(fullCode);
                    env->executeString("__qlsh_sync()");
                    for (int i = 0; i < result.return_count(); ++i) {
                        sol::object obj = result[i];
                        std::cout << lua["tostring"](obj).get<std::string>() << (i < result.return_count() - 1 ? "\t" : "");
                    }
                    if (result.return_count() > 0) std::cout << "\n";
                } else {
                    // Execute as a regular statement block
                    sol::protected_function_result stmtResult = env->executeString(fullCode);
                    if (stmtResult.valid()) {
                        env->executeString("__qlsh_sync()");
                    } else {
                        sol::error err = stmtResult;
                        std::cerr << "Lua Error: " << err.what() << "\n";
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "C++ Exception: " << e.what() << "\n";
            }
            accumulatedInput.clear();
        }
    }

    rx.history_save(historyFile);
    return 0;
}
