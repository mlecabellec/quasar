#pragma once

#include <string>
#include <functional>

namespace quasar::scripting {

/**
 * @brief Interactive Lua REPL for real-time diagnostics.
 */
class ScriptConsole {
public:
    /**
     * @brief Runs the REPL loop on standard input/output.
     * @param prompt The prompt string to display.
     * @param executor A callback to execute a single line of Lua.
     */
    static void run(const std::string& prompt, std::function<void(const std::string&)> executor);

private:
    ScriptConsole() = default;
};

} // namespace quasar::scripting
