/**
 * @file TestOpcUaClient.cpp
 * @brief Integration test suite validating OpcUaClientService mirroring, subscription sync, and remote method execution.
 *
 * @details Spins up an in-process `OpcUaServerService` exposing integer variables and methods,
 * connects an `OpcUaClientService`, and tests remote tree mirroring, subscription value updates,
 * non-void RPC execution, and void RPC execution.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0180, FE-0200
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6, CS-0090.8
 * @compliance Enforces complete Doxygen fixture/field/test annotations, control-flow comments,
 * and <=5-line uncommented density without altering test assertions.
 */

// GoogleTest framework, OPC UA client/server services, and Quasar named primitives.
#include <gtest/gtest.h>
#include "quasar/opcua/OpcUaClientService.hpp"
#include "quasar/opcua/OpcUaServerService.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedMethod.hpp"
// Serialization helper and standard thread/timing headers for async sync waits.
#include "quasar/named/Serialization.hpp"
#include <thread>
#include <chrono>

using namespace quasar::named;
using namespace quasar::opcua;

/**
 * @class OpcUaIntegrationTest
 * @brief GoogleTest fixture setting up a local OPC UA server and client pair on port 4842.
 *
 * @details Builds a sample `ServerRoot/Data` hierarchy containing `MyInt`, `Multiply`, and
 * `VoidMethod`, starts `OpcUaServerService`, and prepares `OpcUaClientService` before each test,
 * stopping both cleanly in `TearDown()`.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0180, FE-0200
 * @constraint CS-0010.21, CS-0090.1, CS-0090.2, CS-0090.9
 * @invariant `server` is running after `SetUp()` and both `client` and `server` are stopped in `TearDown()`.
 * @compliance Deterministic fixture lifecycle teardown guarantees no lingering server/client threads.
 */
class OpcUaIntegrationTest : public ::testing::Test {
protected:
    /**
     * @brief Set up integration test suite.
     *
     * @details Constructs the server `NamedObject` tree (`ServerRoot`, `Data`, `MyInt`,
     * `Multiply`, `VoidMethod`), starts `OpcUaServerService` on TCP port 4842, waits briefly
     * for socket bind, and configures `OpcUaClientService`.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0180, FE-0200
     * @constraint CS-0010.15, CS-0090.1, CS-0090.2, CS-0090.6, CS-0090.8, CS-0090.9
     * @compliance Guards lambda `args` nullness and documents setup failure handling.
     *
     * @post `server->isRunning() == true` and `client` is configured to connect to port 4842.
     * @throws std::exception [Resource / Fatal] Propagates setup failures to GoogleTest harness.
     * @error
     *   - Resource / Fatal (server start or port bind failure): Propagates exception; GoogleTest aborts fixture and runs `TearDown()`.
     * @containment
     *   Approach: Lambda callbacks inside `SetUp` validate `!args` and dynamic casts before accessing integer values.
     *   Limitations: Uses fixed 200 ms startup delay for server socket readiness.
     */
    void SetUp() override {
        // Setup Server hierarchy root, data folder, and integer variable.
        serverRoot = NamedObject::create("ServerRoot");
        serverData = NamedObject::create("Data", serverRoot);
        serverInt = NamedInteger<int32_t>::create("MyInt", 42, serverData);
        
        // Register Multiply RPC method supporting 32-bit or 64-bit integer arguments.
        serverMethod = NamedMethod::create("Multiply", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
            (void)owner;
            // Reject null argument objects before dynamic casting [CS-0010.15].
            if (!args) return nullptr;
            
            int64_t val = 0;
            // Extract 32-bit or 64-bit NamedInteger operand, or return null on type mismatch.
            if (std::shared_ptr<NamedInteger<int32_t>> i32 = std::dynamic_pointer_cast<NamedInteger<int32_t>>(args)) {
                val = i32->value();
            } else if (std::shared_ptr<NamedInteger<int64_t>> i64 = std::dynamic_pointer_cast<NamedInteger<int64_t>>(args)) {
                // Use 64-bit integer argument directly.
                val = i64->value();
            } else {
                // Unsupported argument type yields null RPC result.
                return nullptr;
            }
            
            return NamedInteger<int64_t>::create("result", val * 2);
        }, serverData);

        // Register VoidMethod returning nullptr to exercise void OPC UA RPC handling.
        serverVoidMethod = NamedMethod::create("VoidMethod", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
            (void)owner; (void)args;
            return nullptr;
        }, serverData);

        // Create, configure, and start OPC UA server on integration test port 4842.
        server = OpcUaServerService::create("OpcUaServer");
        server->setPort(4842); // Port for integration test
        server->setRootObject(serverRoot);
        server->start();
        
        // Give server time to bind
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Setup Client targeting local test server endpoint.
        client = OpcUaClientService::create("OpcUaClient");
        client->setUrl("opc.tcp://localhost:4842");
    }

    /**
     * @brief Tear down integration test suite.
     *
     * @details Stops `client` first and then `server` if each service pointer is non-null
     * and running, ensuring orderly disconnect before fixture destruction.
     *
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0180, FE-0200
     * @constraint CS-0010.21, CS-0090.1, CS-0090.2, CS-0090.6, CS-0090.8, CS-0090.9
     * @compliance Enforces client-before-server shutdown order with null/running guards.
     *
     * @post Both `client` and `server` are stopped.
     * @throws std::exception [Resource / Fatal] Propagates if `stop()` throws during teardown.
     * @error
     *   - Validation / Contract (null or already-stopped service): Guarded cleanly as no-op.
     * @containment
     *   Approach: Checks pointer truthiness and `isRunning()` before invoking `stop()`.
     *   Limitations: If `client->stop()` throws, `server->stop()` is skipped until `server` destructor runs.
     */
    void TearDown() override {
        // Disconnect client gracefully while server is still active.
        if (client && client->isRunning()) {
            client->stop();
        }
        // Stop OPC UA server after client disconnect completes.
        if (server && server->isRunning()) {
            server->stop();
        }
    }

    /**
     * @brief Root Quasar object exposed by the test `OpcUaServerService`.
     * @details Hosts `serverData` child hierarchy.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Managed via `std::shared_ptr`.
     */
    std::shared_ptr<NamedObject> serverRoot;

    /**
     * @brief Sub-container `"Data"` holding test variables and methods on the server.
     * @details Child of `serverRoot`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Managed via `std::shared_ptr`.
     */
    std::shared_ptr<NamedObject> serverData;

    /**
     * @brief Server-side integer variable `"MyInt"` initialized to `42`.
     * @details Mutated during `MirroringAndValueSync` to verify subscription updates.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Managed via `std::shared_ptr`.
     */
    std::shared_ptr<NamedInteger<int32_t>> serverInt;

    /**
     * @brief Server-side `"Multiply"` `NamedMethod` doubling an integer argument.
     * @details Exercised remotely in `MethodExecution`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Managed via `std::shared_ptr`.
     */
    std::shared_ptr<NamedMethod> serverMethod;
    
    /**
     * @brief Mirrored void method on the OPC UA server for testing.
     * @details Returns `nullptr` to verify `"void"` sentinel wrapping on client side.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Managed via `std::shared_ptr`.
     */
    std::shared_ptr<NamedMethod> serverVoidMethod;
    
    /**
     * @brief In-process `OpcUaServerService` under test.
     * @details Bound to port 4842 in `SetUp()` and stopped in `TearDown()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0200
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Managed via `std::shared_ptr`.
     */
    std::shared_ptr<OpcUaServerService> server;

    /**
     * @brief In-process `OpcUaClientService` connecting to `server`.
     * @details Configured with `opc.tcp://localhost:4842` in `SetUp()`.
     * @task TSK-20261113-003, TSK-20260311-005
     * @feature FE-0180, FE-0200
     * @constraint CS-0010.6, CS-0090.1, CS-0090.2
     * @compliance Managed via `std::shared_ptr`.
     */
    std::shared_ptr<OpcUaClientService> client;
};

/**
 * @brief Integration test verifying remote address-space mirroring and subscription sync.
 *
 * @details Starts `client`, waits for tree browsing, verifies `ServerRoot/Data/MyInt` initial
 * value `42`, updates `serverInt` to `100`, and confirms client subscription propagation.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0180, FE-0200
 * @constraint CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.8
 * @compliance Uses GoogleTest `EXPECT_*`/`ASSERT_*` checks and documents sync timing assumptions.
 *
 * @throws std::exception [Hardware / Bus / I/O] Caught by GoogleTest runner if client operations throw unexpectedly.
 * @error
 *   - Hardware / Bus / I/O (connection or subscription timeout): Fails `ASSERT_NE`/`EXPECT_EQ` assertions cleanly.
 * @containment
 *   Approach: `EXPECT_NO_THROW` and `ASSERT_NE` prevent null dereference on failed mirror lookup; fixture `TearDown()` stops services.
 *   Limitations: Relies on fixed sleep windows (500 ms mirror, 1000 ms subscription) under OS scheduling.
 */
TEST_F(OpcUaIntegrationTest, MirroringAndValueSync) {
    // Start OPC UA client and wait for initial address-space mirroring.
    EXPECT_NO_THROW(client->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Allow mirroring to complete
    
    // Verify Mirroring of ServerRoot -> Data -> MyInt nodes.
    std::shared_ptr<NamedObject> mirroredRoot = client->getChild("ServerRoot");
    ASSERT_NE(mirroredRoot, nullptr) << "Failed to mirror ServerRoot";
    
    std::shared_ptr<NamedObject> mirroredData = mirroredRoot->getChild("Data");
    ASSERT_NE(mirroredData, nullptr) << "Failed to mirror Data";
    
    std::shared_ptr<NamedObject> mirroredIntObj = mirroredData->getChild("MyInt");
    ASSERT_NE(mirroredIntObj, nullptr) << "Failed to mirror MyInt";
    
    // Downcast mirrored MyInt to strongly typed NamedInteger<int32_t>.
    std::shared_ptr<NamedInteger<int32_t>> mirroredInt = std::dynamic_pointer_cast<NamedInteger<int32_t>>(mirroredIntObj);
    ASSERT_NE(mirroredInt, nullptr) << "MyInt is not a NamedInteger<int32_t>";
    
    // Check initial value mirrored during browse.
    EXPECT_EQ(mirroredInt->value(), 42);
    
    // Test server to client sync by mutating serverInt.
    serverInt->setValue(100);
    
    // Wait for the OPC UA subscription to trigger the client
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Verify subscription data-change notification updated client mirror to 100.
    EXPECT_EQ(mirroredInt->value(), 100) << "Value did not synchronize from server to client";
}

/**
 * @brief Integration test verifying remote OPC UA method invocation with integer argument and return value.
 *
 * @details Starts `client`, resolves mirrored `Multiply` method, invokes it remotely with `21`,
 * and checks that the deserialized `coretypes::Number` result equals `42`.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0180, FE-0200
 * @constraint CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.8
 * @compliance Uses GoogleTest `ASSERT_NE`/`EXPECT_EQ` guards and documents RPC failure behavior.
 *
 * @throws std::exception [Hardware / Bus / I/O] Handled by GoogleTest test runner if uncaught.
 * @error
 *   - Hardware / Bus / I/O (RPC timeout or browse failure): Fails `ASSERT_NE` / `EXPECT_EQ` cleanly.
 * @containment
 *   Approach: Validates non-null mirrored method and result pointers prior to `toInt64()` inspection.
 *   Limitations: Relies on fixed 500 ms mirror stabilization wait after `client->start()`.
 */
TEST_F(OpcUaIntegrationTest, MethodExecution) {
    // Connect client and allow mirroring of server tree and Multiply method.
    EXPECT_NO_THROW(client->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Allow mirroring
    
    // Traverse mirrored ServerRoot -> Data -> Multiply path.
    std::shared_ptr<NamedObject> mirroredRoot = client->getChild("ServerRoot");
    ASSERT_NE(mirroredRoot, nullptr);
    
    std::shared_ptr<NamedObject> mirroredData = mirroredRoot->getChild("Data");
    ASSERT_NE(mirroredData, nullptr);
    
    std::shared_ptr<NamedObject> mirroredMethodObj = mirroredData->getChild("Multiply");
    ASSERT_NE(mirroredMethodObj, nullptr) << "Failed to mirror Multiply method";
    
    // Cast mirrored method proxy to NamedMethod interface.
    std::shared_ptr<NamedMethod> mirroredMethod = std::dynamic_pointer_cast<NamedMethod>(mirroredMethodObj);
    ASSERT_NE(mirroredMethod, nullptr) << "Multiply is not a NamedMethod";
    
    // Execute method remotely with input 21 and verify doubled output 42.
    std::shared_ptr<NamedInteger<int64_t>> args = NamedInteger<int64_t>::create("input", 21);
    std::shared_ptr<NamedObject> resultObj = mirroredMethod->execute(args);
    
    ASSERT_NE(resultObj, nullptr) << "Remote method execution returned nullptr (timed out or failed)";
    
    // Cast JSON-deserialized result to coretypes::Number and verify value == 42.
    std::shared_ptr<quasar::coretypes::Number> resultNum = std::dynamic_pointer_cast<quasar::coretypes::Number>(resultObj);
    ASSERT_NE(resultNum, nullptr) << "Result is not a coretypes::Number";
    
    EXPECT_EQ(resultNum->toInt64(), 42) << "Mathematical operation failed remotely";
}

/**
 * @brief Integration test verifying successful remote execution of void methods over OPC UA.
 *
 * @details Starts `client`, locates mirrored `VoidMethod`, invokes it with `nullptr`, and
 * confirms the returned placeholder `NamedObject` is named `"void"`.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0180, FE-0200
 * @constraint CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.8
 * @compliance Verifies `[FE-0130.4.3]` void-return wrapping over OPC UA with `ASSERT_NE` and `EXPECT_EQ`.
 *
 * @throws std::exception [Hardware / Bus / I/O] Handled by GoogleTest test runner if uncaught.
 * @error
 *   - Hardware / Bus / I/O (browse or RPC timeout): Triggers GoogleTest assertion failure cleanly.
 * @containment
 *   Approach: Checks non-null mirrored method and result handles before inspecting `getName()`.
 *   Limitations: Fixed 500 ms wait after `client->start()` assumes local loopback responsiveness.
 */
TEST_F(OpcUaIntegrationTest, VoidMethodExecution) {
    // Connect client and allow mirroring to populate VoidMethod node.
    EXPECT_NO_THROW(client->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Allow mirroring
    
    // Resolve mirrored ServerRoot -> Data -> VoidMethod object.
    std::shared_ptr<NamedObject> mirroredRoot = client->getChild("ServerRoot");
    ASSERT_NE(mirroredRoot, nullptr);
    
    std::shared_ptr<NamedObject> mirroredData = mirroredRoot->getChild("Data");
    ASSERT_NE(mirroredData, nullptr);
    
    std::shared_ptr<NamedObject> mirroredMethodObj = mirroredData->getChild("VoidMethod");
    ASSERT_NE(mirroredMethodObj, nullptr) << "Failed to mirror VoidMethod method";
    
    // Cast to NamedMethod, execute remotely with nullptr, and verify "void" sentinel.
    std::shared_ptr<NamedMethod> mirroredMethod = std::dynamic_pointer_cast<NamedMethod>(mirroredMethodObj);
    ASSERT_NE(mirroredMethod, nullptr) << "VoidMethod is not a NamedMethod";
    
    std::shared_ptr<NamedObject> resultObj = mirroredMethod->execute(nullptr);
    ASSERT_NE(resultObj, nullptr);
    EXPECT_EQ(resultObj->getName(), "void");
}
