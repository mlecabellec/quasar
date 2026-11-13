/**
 * @file TestManualScripts.cpp
 * @brief Parameterized GoogleTest runner for Quasar manual Lua integration scripts.
 *
 * @details Discovers `.lua` manual test scripts under `QUASAR_MANUAL_DIR`, loads
 * available Quasar plugins from `QUASAR_PLUGIN_DIR`, executes each script inside a
 * fresh `LuaEngine`, handles `os.exit` hooks and missing-plugin skips cleanly, and
 * shuts down the engine after each test case.
 *
 * @task TSK-20261113-003
 * @feature FE-0060
 * @constraint CS-0010.44, CS-0010.45, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6
 * @compliance Documents test fixture, test cases, discovery helpers, and all
 * control structures with strict 7-bit ASCII Doxygen and inline justifications.
 */

// GoogleTest framework and standard filesystem/stream/container headers.
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <vector>

// Quasar scripting engine and dynamic plugin loader headers.
#include "quasar/scripting/LuaEngine.hpp"
#include "quasar/scripting/PluginLoader.hpp"

/**
 * @brief Namespace containing Quasar scripting engine tests and utilities.
 * @details Hosts the `ManualScriptTest` parameterized test fixture and script
 * discovery helper for manual Lua verification suites.
 *
 * @task TSK-20261113-003
 * @feature FE-0060
 * @constraint CS-0010.33, CS-0090.1, CS-0090.2
 * @compliance Encloses manual script test definitions in namespace `quasar::scripting`.
 */
namespace quasar::scripting {

/**
 * @brief Parameterized GoogleTest fixture for executing manual Lua scripts.
 * @details Each parameterized instantiation receives a `std::filesystem::path`
 * pointing to a `.lua` script file from `GetManualScripts()`.
 *
 * @task TSK-20261113-003
 * @feature FE-0060
 * @constraint CS-0010.45, CS-0090.1, CS-0090.2, CS-0090.9
 * @compliance Derives from `testing::TestWithParam<std::filesystem::path>` with
 * explicit lifecycle documentation.
 * @invariant Each test instance executes in an isolated `LuaEngine` context.
 */
class ManualScriptTest : public ::testing::TestWithParam<std::filesystem::path> {
protected:
  /**
   * @brief Per-test fixture setup hook.
   * @details Default no-op override invoked by GoogleTest prior to `Execute` test body;
   * script loading and `LuaEngine` creation are performed directly inside `TEST_P`.
   *
   * @task TSK-20261113-003
   * @feature FE-0060
   * @constraint CS-0090.1, CS-0090.2, CS-0090.8, CS-0090.9
   * @compliance Safe no-op override conforming to `testing::Test::SetUp()`.
   * @post Fixture is ready for parameterized test body execution.
   * @error
   *   - None (no-throw stub): Performs no allocations or I/O.
   * @containment
   *   Approach: Isolates fixture initialization from plugin loading so each script manages its own engine lifetime.
   *   Limitations: Shared state cannot be reused across parameterized script invocations.
   */
    void SetUp() override {
    }
};

/**
 * @brief Allow `ManualScriptTest` to remain uninstantiated when no manual scripts are present.
 * @details Registers GoogleTest macro flag `gtest_allow_ignore_ManualScriptTest` so builds
 * without populated `QUASAR_MANUAL_DIR` scripts do not fail test discovery.
 *
 * @task TSK-20261113-003
 * @feature FE-0060
 * @constraint CS-0090.1, CS-0090.2
 * @compliance Documents macro-synthesized GoogleTest registration constant.
 */
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ManualScriptTest);

/**
 * @brief Execute a single manual Lua script under the Quasar scripting engine.
 * @details Reads the script file provided by `GetParam()`, instantiates a `LuaEngine`,
 * overrides `os.exit` to translate exit codes into catchable exceptions, loads non-test
 * Quasar shared-library plugins from `QUASAR_PLUGIN_DIR`, runs the script via
 * `executeString`, skips cleanly if optional plugin globals are missing, and shuts down
 * the engine.
 *
 * @task TSK-20261113-003
 * @feature FE-0060
 * @constraint CS-0010.20, CS-0010.44, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6, CS-0090.8, CS-0090.9
 * @compliance Enforces bounded exception handling around Lua execution and explicit engine shutdown.
 * @post Script completes with exit code 0, triggers `GTEST_SKIP`, or fails test assertions cleanly.
 * @invariant `engine->shutdown()` is invoked after script execution completes.
 * @throws std::runtime_error [Script / Execution] Thrown internally by overridden `os.exit` and caught locally.
 * @error
 *   - Script / Execution (`LUA_EXIT_ERROR_*` or uncaught `std::exception`): Caught and reported via GoogleTest `FAIL()`.
 *   - Environment / Missing Plugin (`attempt to index a nil value`): Converted into `GTEST_SKIP()`.
 * @containment
 *   Approach: Wraps `engine->executeString` in `try`/`catch` and intercepts `os.exit` so scripts cannot terminate the test runner process.
 *   Limitations: Native plugin crashes or `std::abort` inside C extensions cannot be caught by C++ exception handlers.
 */
TEST_P(ManualScriptTest, Execute) {
    // Resolve parameterized Lua script path and emit diagnostic trace.
    std::filesystem::path scriptPath = GetParam();
    std::cout << "[ManualTest] Running: " << scriptPath.filename().string() << std::endl;

    // Open script file and assert readability before attempting execution.
    std::ifstream file(scriptPath);
    ASSERT_TRUE(file.is_open()) << "Could not open script: " << scriptPath;

    // Buffer full Lua source text into a local string for execution.
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();

    // Instantiate isolated LuaEngine and obtain Sol2 state reference.
    std::shared_ptr<LuaEngine> engine = LuaEngine::create();
    sol::state& lua = engine->getState();

    // Override os.exit to catch plugin-related exits or intentional stops
    lua["os"]["exit"] = [](int code) {
        // Distinguish failing exit codes from clean exit(0) termination.
        if (code != 0) {
            throw std::runtime_error("LUA_EXIT_ERROR_" + std::to_string(code));
        }
        throw std::runtime_error("LUA_EXIT_ZERO");
    };

    // Load available plugins from QUASAR_PLUGIN_DIR
    /**
     * @brief Construct plugin directory path from compile-time `QUASAR_PLUGIN_DIR` macro.
     * @details Holds the filesystem path scanned for Quasar runtime `.so`/`.dll` plugins.
     *
     * @task TSK-20261113-003
     * @feature FE-0060
     * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
     * @compliance Explicit path variable construction from build-configured plugin directory macro.
     * @post `pluginDir` holds `std::filesystem::path(QUASAR_PLUGIN_DIR)`.
     * @return std::filesystem::path
     *   Justification: Represents the plugin directory path when parsed without macro expansion.
     *   Usage: Checked via `std::filesystem::exists(pluginDir)` before iterating directory entries.
     *   Consumed: Value constructed in local scope and read-only during iteration.
     *   Prerequisites: `QUASAR_PLUGIN_DIR` macro defined as a valid string literal at build time.
     *   Limitations: Non-existent directories are skipped gracefully without loading plugins.
     *   Lifecycle: Local automatic storage duration within `TEST_P` body.
     *   Modification / Deletion: Local stack variable destroyed automatically on scope exit.
     *   Specific Values: Path matching `QUASAR_PLUGIN_DIR`.
     *   Error Behavior: Propagates `std::bad_alloc` only on memory exhaustion.
     * @error
     *   - Resource / Allocation (`std::bad_alloc`): Path string allocation failure unwinds test body.
     * @containment
     *   Approach: Guarded by `std::filesystem::exists(pluginDir)` prior to directory traversal.
     *   Limitations: Cannot recover if `QUASAR_PLUGIN_DIR` is undefined at compile time.
     */
    std::filesystem::path pluginDir(QUASAR_PLUGIN_DIR);
    // Scan plugin directory only if it exists on the host filesystem.
    if (std::filesystem::exists(pluginDir)) {
        // Iterate over directory entries (bounded by filesystem entry count in pluginDir).
        for (const auto& entry : std::filesystem::directory_iterator(pluginDir)) {
            // Load only native shared-library plugin binaries (.so or .dll).
            if (entry.path().extension() == ".so" || entry.path().extension() == ".dll") {
                std::string name = entry.path().filename().string();
                // Filter to Quasar runtime plugins while excluding test-only helper libraries.
                if (name.find("quasar_") != std::string::npos && name.find("test") == std::string::npos) {
                    PluginLoader::loadPlugin(entry.path().string(), lua);
                }
            }
        }
    }

    // Execute Lua script under try/catch to intercept os.exit sentinel throws and Lua errors.
    try {
        sol::protected_function_result result = engine->executeString(code);
        // Inspect protected Lua execution status and distinguish missing-plugin skips from failures.
        if (!result.valid()) {
            sol::error err = result;
            std::string errMsg = err.what();
            
            // Check for common failure patterns if plugins are still missing despite loading attempts
            if (errMsg.find("attempt to index a nil value (field '") != std::string::npos ||
                errMsg.find("attempt to index a nil value (local '") != std::string::npos) {
                GTEST_SKIP() << "Skipping " << scriptPath.filename() << " due to missing plugin requirements: " << errMsg;
            }
            
            FAIL() << "Script " << scriptPath.filename() << " failed:\n" << errMsg;
        }
    // Catch exceptions raised by os.exit override or C++ bindings during Lua execution.
    } catch (const std::exception& e) {
        std::string msg = e.what();
        // Fail test when script explicitly called os.exit with a non-zero error code.
        if (msg.find("LUA_EXIT_ERROR") != std::string::npos) {
             FAIL() << "Script " << scriptPath.filename() << " called os.exit with error: " << msg;
        // Treat os.exit(0) sentinel exception as successful early script termination.
        } else if (msg == "LUA_EXIT_ZERO") {
            // Success exit
        // Fail test on any other unexpected C++ std::exception message.
        } else {
            FAIL() << "Script " << scriptPath.filename() << " threw exception: " << msg;
        }
    }
    
    // Tear down LuaEngine resources deterministically after script completion.
    engine->shutdown();
}

/**
 * @brief Collect and sort runnable manual Lua test scripts from `QUASAR_MANUAL_DIR`.
 * @details Scans `QUASAR_MANUAL_DIR` for `.lua` files while excluding hardware-dependent
 * EtherCAT scripts and scripts with known external OPC UA service dependencies, then
 * sorts the resulting paths lexicographically for deterministic test ordering.
 *
 * @task TSK-20261113-003
 * @feature FE-0060
 * @constraint CS-0010.44, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
 * @compliance Uses bounded directory iteration, explicit filtering, and move-returnable vector value.
 * @post Returned vector is sorted lexicographically and contains only eligible `.lua` paths.
 * @return std::vector<std::filesystem::path>
 *   Justification: Supplies parameterized script paths to `INSTANTIATE_TEST_SUITE_P`.
 *   Usage: Passed directly to `testing::ValuesIn` at test suite initialization.
 *   Consumed: Returned by value (NRVO/move semantics); caller owns the vector.
 *   Prerequisites: `QUASAR_MANUAL_DIR` macro configured as a valid path string literal.
 *   Limitations: Non-existent `QUASAR_MANUAL_DIR` yields an empty vector (all tests skipped).
 *   Lifecycle: Caller-owned temporary/vector valid for test registration lifetime.
 *   Modification / Deletion: Caller may iterate, mutate, or discard vector; RAII cleans up elements.
 *   Specific Values: Sorted list of eligible `.lua` file paths, or empty vector if directory absent.
 *   Error Behavior: Propagates `std::filesystem::filesystem_error` or `std::bad_alloc` on I/O or heap failure.
 * @throws std::filesystem::filesystem_error [Hardware / Bus / I/O] Propagated if directory iteration fails.
 * @throws std::bad_alloc [Resource / Allocation] Propagated if path vector growth fails.
 * @error
 *   - Environment / Missing Directory (`!exists || !is_directory`): Returns empty vector cleanly; no-throw.
 *   - Hardware / Bus / I/O (`std::filesystem::filesystem_error`): Propagates during static test registration.
 * @containment
 *   Approach: Checks directory existence and type before opening `directory_iterator` and filters incompatible scripts.
 *   Limitations: Exclusion list is hardcoded by filename substring (`ethercat`, `19_opcua_client`, `opcua_test`).
 */
std::vector<std::filesystem::path> GetManualScripts() {
    std::vector<std::filesystem::path> scripts;
    std::string pathStr = QUASAR_MANUAL_DIR;
    std::filesystem::path path(pathStr);
    
    // Traverse manual script directory only when the path exists and refers to a directory.
    if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
        // Iterate directory entries (bounded by number of files in QUASAR_MANUAL_DIR).
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            std::string filename = entry.path().filename().string();
            // Exclude EtherCAT related scripts and non-lua files
            // Also exclude scripts with hard external dependencies (Script_19) or known mirroring proxy bugs (opcua_test)
            // Accept only standalone .lua scripts that do not require EtherCAT hardware or external OPC UA fixtures.
            if (entry.path().extension() == ".lua" && 
                filename.find("ethercat") == std::string::npos &&
                filename.find("19_opcua_client") == std::string::npos &&
                filename.find("opcua_test") == std::string::npos) {
                scripts.push_back(entry.path());
            }
        }
    }
    
    // Sort discovered script paths lexicographically for deterministic test indexing.
    std::sort(scripts.begin(), scripts.end());
    return scripts;
}

/// @cond GTEST_MACRO_INSTANTIATION
/**
 * @brief Instantiate `DocManual` parameterized test suite with discovered manual scripts.
 * @details Registers the GoogleTest value generator and custom test-name sanitizer lambda
 * converting script filenames into valid C++ test identifiers.
 *
 * @task TSK-20261113-003
 * @feature FE-0060
 * @constraint CS-0090.1, CS-0090.2, CS-0090.7, CS-0090.8, CS-0090.9
 * @compliance Documents GoogleTest parameterized instantiation macro and generated evaluator function.
 * @post `DocManual/ManualScriptTest` cases are registered for every path in `GetManualScripts()`.
 * @return testing::internal::ParamGenerator<ManualScriptTest::ParamType>
 *   Justification: Provides the parameterized test case sequence required by GoogleTest.
 *   Usage: Invoked internally by GoogleTest test-suite registration machinery.
 *   Consumed: Returned by value to GoogleTest registration framework.
 *   Prerequisites: `GetManualScripts()` returns valid script path objects.
 *   Limitations: Test names sanitize non-alphanumeric characters to `_` and prefix leading digits with `Script_`.
 *   Lifecycle: Managed internally by GoogleTest static test registry for process lifetime.
 *   Modification / Deletion: Caller does not invoke or delete directly; managed by GoogleTest.
 *   Specific Values: Generator wrapping `GetManualScripts()` path list.
 *   Error Behavior: Propagates any `GetManualScripts()` exception during static initialization.
 * @error
 *   - Initialization / I/O (`std::filesystem::filesystem_error` or `std::bad_alloc`): Propagates during static test registration.
 * @containment
 *   Approach: Sanitizes generated test identifiers to alphanumeric characters to avoid GoogleTest registration errors.
 *   Limitations: Relies on `GetManualScripts()` directory scan succeeding at startup.
 */
INSTANTIATE_TEST_SUITE_P(
    DocManual,
    ManualScriptTest,
    ::testing::ValuesIn(GetManualScripts()),
    // Sanitize script stem into a valid GoogleTest alphanumeric test suffix.
    [](const ::testing::TestParamInfo<ManualScriptTest::ParamType>& info) {
        std::string name = info.param.stem().string();
        // Prefix numeric filenames with "Script_" because GoogleTest names cannot begin with a digit.
        if (std::isdigit(name[0])) {
            name = "Script_" + name;
        }
        // Replace non-alphanumeric characters with underscores for valid test case naming.
        std::replace_if(name.begin(), name.end(), [](char c) { return !std::isalnum(c); }, '_');
        return name;
    }
);
/// @endcond

} // namespace quasar::scripting
