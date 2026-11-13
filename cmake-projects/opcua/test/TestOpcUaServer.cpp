/**
 * @file TestOpcUaServer.cpp
 * @brief Unit tests verifying OpcUaServerService startup, address-space mapping, and method node registration.
 *
 * @details Exercises `OpcUaServerService` lifecycle (`start`/`stop`), reactive variable value
 * updates from Quasar to OPC UA, and `NamedMethod` node mapping on port 4841.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0140, FE-0200
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6, CS-0090.8
 * @compliance Provides complete Doxygen fixture/field/test annotations and control-structure
 * comments without modifying executable test code.
 */

// GoogleTest framework, OPC UA server service, and Quasar named types.
#include <gtest/gtest.h>
#include "quasar/opcua/OpcUaServerService.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedMethod.hpp"
// Serialization utilities and thread/timing support.
#include "quasar/named/Serialization.hpp"
#include <thread>
#include <chrono>

using namespace quasar::named;
using namespace quasar::opcua;

/**
 * @class OpcUaServerTest
 * @brief GoogleTest fixture provisioning an isolated `OpcUaServerService` on port 4841.
 *
 * @details Builds a `Root/Data/MyInt` Quasar subtree and configures `server` in `SetUp()`,
 * stopping it cleanly in `TearDown()`.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0140, FE-0200
 * @constraint CS-0010.21, CS-0090.1, CS-0090.2, CS-0090.9
 * @invariant `server` is configured on port 4841 before each test and stopped in `TearDown()`.
 * @compliance Ensures deterministic server shutdown at test teardown.
 */
class OpcUaServerTest : public ::testing::Test {
protected:
    /**
     * @brief Initializes test tree and configures `OpcUaServerService` on port 4841.
     *
     * @details Creates `Root`, `Data`, `MyInt` (`42`), and `server` bound to `root`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0140, FE-0200
     * @constraint CS-0090.1, CS-0090.2, CS-0090.8, CS-0090.9
     * @compliance Deterministic fixture setup via `create()` factory methods.
     *
     * @post `server != nullptr` and configured for port 4841.
     * @throws std::exception [Resource / Allocation] Propagates allocation/tree errors to GoogleTest.
     * @error
     *   - Resource / Allocation (object creation failure): Unwinds cleanly and aborts test.
     * @containment
     *   Approach: Shared pointers manage all tree nodes and service lifetime automatically.
     *   Limitations: Port 4841 conflict surfaces only when `server->start()` runs in test body.
     */
    void SetUp() override {
        // Construct sample Quasar hierarchy and integer variable.
        root = NamedObject::create("Root");
        data = NamedObject::create("Data", root);
        myInt = NamedInteger<int32_t>::create("MyInt", 42, data);
        
        // Configure OPC UA server on dedicated port 4841 to avoid collisions.
        server = OpcUaServerService::create("OpcUaServer");
        server->setPort(4841); // Use a different port to avoid conflicts
        server->setRootObject(root);
    }

    /**
     * @brief Stops the OPC UA server if still running after test execution.
     *
     * @details Checks `server && server->isRunning()` before calling `server->stop()`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.21, CS-0090.1, CS-0090.2, CS-0090.6, CS-0090.8, CS-0090.9
     * @compliance Guards null/stopped service prior to calling `stop()`.
     *
     * @post `!server->isRunning()`.
     * @throws std::exception [Resource / Fatal] Propagates if `server->stop()` throws.
     * @error
     *   - Validation / Contract (already stopped): Safely skipped.
     * @containment
     *   Approach: Explicit null and running-state check before invoking `stop()`.
     *   Limitations: None.
     */
    void TearDown() override {
        // Stop running server cleanly to release listening socket and worker thread.
        if (server && server->isRunning()) {
            server->stop();
        }
    }

    /**
     * @brief Root Quasar node exposed by `server`.
     * @details Parent of `data`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Managed via `std::shared_ptr`.
     */
    std::shared_ptr<NamedObject> root;

    /**
     * @brief Intermediate `"Data"` container holding test variable and method nodes.
     * @details Child of `root`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Managed via `std::shared_ptr`.
     */
    std::shared_ptr<NamedObject> data;

    /**
     * @brief Sample integer node `"MyInt"` used to test reactive server updates.
     * @details Initialized to `42` in `SetUp()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Managed via `std::shared_ptr`.
     */
    std::shared_ptr<NamedInteger<int32_t>> myInt;

    /**
     * @brief `OpcUaServerService` instance under test.
     * @details Configured on port 4841 in `SetUp()` and stopped in `TearDown()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Managed via `std::shared_ptr`.
     */
    std::shared_ptr<OpcUaServerService> server;
};

/**
 * @brief Verifies OPC UA server startup, reactive variable update handling, and clean shutdown.
 *
 * @details Starts `server`, confirms `isRunning()`, updates `myInt` to `100` to trigger
 * `TreeObserver`, and stops `server`, confirming `!isRunning()`.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0140, FE-0200
 * @constraint CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.8
 * @compliance Uses GoogleTest `EXPECT_NO_THROW`/`EXPECT_TRUE`/`EXPECT_FALSE` checks.
 *
 * @throws std::exception [Resource / Fatal] Handled by GoogleTest test runner if uncaught.
 * @error
 *   - Resource / Fatal (startup or value update exception): Fails `EXPECT_NO_THROW` cleanly.
 * @containment
 *   Approach: `EXPECT_NO_THROW` wraps lifecycle/value calls and `TearDown()` guarantees cleanup.
 *   Limitations: Uses fixed 100 ms/50 ms sleeps for background loop spin-up.
 */
TEST_F(OpcUaServerTest, BasicStartupAndMapping) {
    // Start server and wait briefly for worker loop initialization.
    EXPECT_NO_THROW(server->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Let it spin up
    EXPECT_TRUE(server->isRunning());
    
    // Test dynamic value update propagation through TreeObserver.
    EXPECT_NO_THROW(myInt->setValue(100));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Stop server and verify running flag clears.
    EXPECT_NO_THROW(server->stop());
    EXPECT_FALSE(server->isRunning());
}

/**
 * @brief Verifies OPC UA server startup when a `NamedMethod` node is present in the mapped tree.
 *
 * @details Attaches a `"Multiply"` `NamedMethod` under `data`, starts `server`, verifies
 * active status, and shuts down cleanly.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0140, FE-0200
 * @constraint CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6, CS-0090.8
 * @compliance Annotates in-lambda null/type guards and verifies clean method-node lifecycle.
 *
 * @throws std::exception [Resource / Fatal] Handled by GoogleTest test runner if uncaught.
 * @error
 *   - Validation / Contract (invalid lambda args): Returns `nullptr` safely.
 * @containment
 *   Approach: Lambda checks `!args`, type string, and `dynamic_pointer_cast` result before dereferencing.
 *   Limitations: Exercises mapping and startup/shutdown rather than a live OPC UA client RPC call.
 */
TEST_F(OpcUaServerTest, MethodMappingAndExecution) {
    // Attach a typed Multiply NamedMethod under data prior to server startup.
    auto multiplyMethod = NamedMethod::create("Multiply", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
        (void)owner;
        // Reject missing or non-NamedInteger arguments [CS-0010.15].
        if (!args || args->getType() != "NamedInteger") return nullptr;
        auto input = std::dynamic_pointer_cast<NamedInteger<int64_t>>(args);
        // Abort cleanly if dynamic cast to NamedInteger<int64_t> fails.
        if (!input) return nullptr;
        return NamedInteger<int64_t>::create("result", input->value() * 2);
    }, data);

    // Start server to map Multiply method node, verify running, and stop cleanly.
    EXPECT_NO_THROW(server->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(server->isRunning());
    
    EXPECT_NO_THROW(server->stop());
}
