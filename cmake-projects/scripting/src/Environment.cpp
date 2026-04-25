#include "quasar/scripting/Environment.hpp"
#include "quasar/scripting/PluginLoader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace quasar {
namespace scripting {

/**
 * @brief Enabler struct to allow std::make_shared to call protected constructor.
 */
struct EnvironmentEnabler : public Environment {};

/**
 * @brief Factory method for creating an Environment.
 * @return std::shared_ptr<Environment> instance.
 */
std::shared_ptr<Environment> Environment::create() {
    // [CS-0010.10] Use make_shared instead of raw new.
    return std::make_shared<EnvironmentEnabler>();
}

/**
 * @brief Environment constructor.
 * @details Initializes the underlying LuaEngine instance.
 */
Environment::Environment() {
    m_engine = LuaEngine::create();
}

/**
 * @brief Environment destructor.
 * @details Triggers engine shutdown for clean resource release.
 */
Environment::~Environment() {
    if (m_engine) {
        m_engine->shutdown();
    }
}

/**
 * @brief Loads a plugin via the PluginLoader.
 * @param path Path to the shared library.
 * @return bool true if loaded successfully.
 */
bool Environment::loadPlugin(const std::string& path) {
    return PluginLoader::loadPlugin(path, m_engine->getState());
}

/**
 * @brief Safely executes a code string.
 * @param code The string to execute.
 * @return sol::protected_function_result containing the execution outcome.
 */
sol::protected_function_result Environment::executeString(const std::string& code) {
    return m_engine->executeString(code);
}

/**
 * @brief Safely executes a Lua file.
 * @param path The file path.
 * @return sol::protected_function_result containing the execution outcome.
 */
sol::protected_function_result Environment::executeFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open script file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return m_engine->executeString(buffer.str());
}

} // namespace scripting
} // namespace quasar
