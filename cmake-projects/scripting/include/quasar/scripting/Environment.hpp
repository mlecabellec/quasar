/**
 * @file Environment.hpp
 * @brief Shared execution environment for script runner and shell.
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260421-001.1] Interactive Shell Executable.
 * - Fulfills [CS-0010.45] Doxygen documentation.
 */

#pragma once

#include "quasar/scripting/LuaEngine.hpp"
#include <memory>
#include <string>
#include <vector>

namespace quasar {
namespace scripting {

/**
 * @brief Manages the shared execution environment for both sre and qlsh.
 * @details Encapsulates the LuaEngine lifecycle, plugin loading, and object tracking.
 *          This ensures parity between standalone script execution and interactive use.
 * 
 * @contribution [TSK-20260421-001.1] Interactive Shell Executable
 */
class Environment {
public:
    /**
     * @brief Factory method to create an Environment.
     * @return Shared pointer to the new environment.
     */
    static std::shared_ptr<Environment> create();

    /**
     * @brief Destructor ensures engine shutdown.
     */
    ~Environment();

    /**
     * @brief Loads a dynamic plugin shared library into the Lua context.
     * @param path Path to the shared library (.so, .dll).
     * @return true if successful.
     */
    bool loadPlugin(const std::string& path);

    /**
     * @brief Executes a Lua string within the managed engine.
     * @param code The Lua source code to execute.
     * @return Result of the execution.
     */
    sol::protected_function_result executeString(const std::string& code);

    /**
     * @brief Executes a Lua file.
     * @param path Path to the script file.
     * @return Result of the execution.
     */
    sol::protected_function_result executeFile(const std::string& path);

    /**
     * @brief Provides access to the underlying engine.
     * @return Shared pointer to the LuaEngine.
     */
    std::shared_ptr<LuaEngine> getEngine() const { return m_engine; }

private:
    /**
     * @brief Private constructor to enforce factory usage.
     */
    Environment();

    std::shared_ptr<LuaEngine> m_engine;
};

} // namespace scripting
} // namespace quasar
