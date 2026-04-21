/**
 * @file ShellCompleter.hpp
 * @brief Context-aware autocompletion for qlsh.
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260421-001.3] Intelligent Autocompletion.
 * - Fulfills [CS-0010.45] Doxygen documentation.
 */

#pragma once
#include <replxx.hxx>
#include <sol/sol.hpp>
#include <string>
#include <vector>

namespace quasar::scripting {

/**
 * @brief Provides intelligent autocompletion for qlsh.
 * @details Bridges the Lua global environment and the live NamedObject hierarchy.
 */
class ShellCompleter {
public:
    /**
     * @brief Constructs the completer with a reference to the active Lua state.
     * @param lua The managed sol::state.
     */
    explicit ShellCompleter(sol::state& lua);

    /**
     * @brief Callback for replxx to generate completion suggestions.
     * @param input The current input line.
     * @param contextLen Output parameter indicating the length of the prefix being completed.
     * @return Vector of completion strings.
     */
    replxx::Replxx::completions_t operator()(std::string const& input, int& contextLen);

private:
    /**
     * @brief Internal helper to find the last identifier-like token.
     */
    std::string findCurrentToken(const std::string& input, int cursorIndex) const;

    sol::state& m_lua;
};

} // namespace quasar::scripting
