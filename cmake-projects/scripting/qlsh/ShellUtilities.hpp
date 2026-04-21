/**
 * @file ShellUtilities.hpp
 * @brief Shell-specific diagnostic utilities (ls, help).
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260421-001.5] Reflexive Introspection Utilities.
 * - Fulfills [CS-0010.45] Doxygen documentation.
 */

#pragma once
#include <sol/sol.hpp>

namespace quasar::scripting {

/**
 * @brief Binds shell-specific diagnostic utilities to the Lua environment.
 * @details Adds global functions like ls() and help() for tree visualization.
 * 
 * @contribution [TSK-20260421-001.5] Reflexive Introspection Utilities
 */
void bindShellUtilities(sol::state& lua);

} // namespace quasar::scripting
