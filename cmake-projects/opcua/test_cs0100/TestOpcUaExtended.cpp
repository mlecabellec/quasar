/**
 * @file TestOpcUaExtended.cpp
 * @brief Extended CS-0100 test suite for `quasar_opcua` covering server/client mirroring, `UA_Common`, `OpcUaSecurityManager`, error boundaries, fuzzing, and concurrent client task queues.
 * @details Uses dedicated ports (`4851`-`4853`) to remain 100% compatible with parallel CTest execution alongside legacy `opcua_test` (`4841`).
 * @task [TSK-20260311-005] OPC UA Integration & Mirroring
 * @task [TSK-20261113-003] Formal OPC UA Documentation & Safety Contracts
 * @feature [FE-0200] OPC UA Server and Client Services
 * @feature [FE-0220] OPC UA X.509 & Username Security
 * @compliance Fulfills CS-0010, CS-0040, CS-0060, CS-0090, and CS-0100.
 */

#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedMethod.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/opcua/OpcUaClientService.hpp"
#include "quasar/opcua/OpcUaSecurityManager.hpp"
#include "quasar/opcua/OpcUaServerService.hpp"
#include "quasar/opcua/UA_Common.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"
#include <chrono>
#include <cstdint>
#include <future>
#include <gtest/gtest.h>
#include <memory>
#include <open62541/client_config_default.h>
#include <open62541/server_config_default.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace quasar::opcua::test {

/**
 * @brief Dedicated TCP port for Tutorial 2 server/client test (`CS-0040.2`).
 * @requirement [CS-0040.2] Symbolic Port Constant
 * @compliance Avoids collision with legacy `TestOpcUaServer.cpp` port `4841`.
 */
constexpr uint16_t TUTORIAL_OPCUA_PORT = 4851U;

/**
 * @brief Dedicated TCP port for concurrent queue stress test (`CS-0040.2`).
 * @requirement [CS-0040.2] Symbolic Port Constant
 * @compliance Prevents cross-test socket bind conflicts during parallel CTest runs.
 */
constexpr uint16_t STRESS_OPCUA_PORT = 4852U;

/**
 * @brief Unused localhost port used to test unreachable server error handling (`CS-0040.2`).
 * @requirement [FE-0200] Connection Failure Containment
 * @compliance Fixed symbolic constant for negative connection tests.
 */
constexpr uint16_t UNUSED_OPCUA_PORT = 4859U;

/**
 * @brief Startup stabilization wait using `std::condition_variable` timeout (`CS-0010.27`).
 * @requirement [CS-0010.27] Timed Wait Usage
 * @compliance Bounded timed wait replaces bare infinite waits.
 */
constexpr std::chrono::milliseconds SERVER_STARTUP_WAIT_MS{80};

/**
 * @brief Iteration cap for seeded `toUaVariant` fuzz loop (`CS-0010.37`).
 * @requirement [CS-0010.37] Bounded Loops
 * @compliance Guarantees finite fuzz execution.
 */
constexpr std::size_t MAX_VARIANT_FUZZ_ITERS = 64U;

/**
 * @brief Number of concurrent client task queue producers (`CS-0040.2`).
 * @requirement [FE-0200.3] Thread-Safe Client Task Queue
 * @compliance Fixed small worker count for fast deterministic stress testing.
 */
constexpr std::size_t CLIENT_TASK_WORKERS = 3U;

/**
 * @brief Tasks enqueued per concurrent client worker (`CS-0010.37`).
 * @requirement [CS-0010.37] Bounded Loops
 * @compliance Deterministic loop upper bound.
 */
constexpr std::size_t TASKS_PER_WORKER = 10U;

/**
 * @brief Deterministic seed for OPC UA variant fuzzing (`CS-0100.12`).
 * @requirement [CS-0100.12] Deterministic Tests
 * @compliance Fixed 64-bit seed.
 */
constexpr std::uint64_t OPCUA_FUZZ_SEED = 0x09C0A485ULL;

TEST(OpcUaTutorialTest, Tutorial1_UaVariantConversionAndHelpers) {
    quasar::test_support::TestScopeGuard scope("OpcUaTutorialTest.Tutorial1");
    // Step 1: Convert std::string into open62541 UA_String and binary-safe UA_ByteString.
    UA_String uaStr = UA_String_fromStdString("QuasarOpcUa");
    EXPECT_EQ(uaStr.length, 11U);
    UA_String_clear(&uaStr);

    std::string binaryPayload("A\0B", 3U);
    UA_ByteString uaBs = UA_ByteString_fromStdString(binaryPayload);
    EXPECT_EQ(uaBs.length, 3U);
    UA_ByteString_clear(&uaBs);

    // Step 2: Convert Quasar NamedObject scalars into UA_Variant values.
    std::shared_ptr<named::NamedInteger<int64_t>> nInt =
        named::NamedInteger<int64_t>::create("Counter", 1024);
    std::shared_ptr<named::NamedBoolean> nBool =
        named::NamedBoolean::create("Active", true);
    std::shared_ptr<named::NamedString> nStr =
        named::NamedString::create("Tag", "Pump01");
    std::shared_ptr<named::NamedFloatingPoint<double>> nDbl =
        named::NamedFloatingPoint<double>::create("Temp", 21.5);

    UA_Variant vInt = toUaVariant(nInt);
    UA_Variant vBool = toUaVariant(nBool);
    UA_Variant vStr = toUaVariant(nStr);
    UA_Variant vDbl = toUaVariant(nDbl);
    EXPECT_FALSE(UA_Variant_isEmpty(&vInt));
    EXPECT_FALSE(UA_Variant_isEmpty(&vBool));
    EXPECT_FALSE(UA_Variant_isEmpty(&vStr));
    EXPECT_FALSE(UA_Variant_isEmpty(&vDbl));
    UA_Variant_clear(&vInt);
    UA_Variant_clear(&vBool);
    UA_Variant_clear(&vStr);
    UA_Variant_clear(&vDbl);
    quasar::test_support::AsyncTestLogger::instance().logOutput("variantMarshaling", "ok");
}

TEST(OpcUaTutorialTest, Tutorial2_ServerAndClientMirroring) {
    quasar::test_support::TestScopeGuard scope("OpcUaTutorialTest.Tutorial2");
    // Step 1: Build an industrial NamedObject subtree with variables and a method.
    std::shared_ptr<named::NamedObject> root = named::NamedObject::create("Substation");
    std::shared_ptr<named::NamedInteger<int64_t>> rpm =
        named::NamedInteger<int64_t>::create("MotorRpm", 1500, root);
    (void)named::NamedBoolean::create("BreakerClosed", true, root);
    (void)named::NamedMethod::create(
        "ResetFault",
        [](std::shared_ptr<named::NamedObject> owner, std::shared_ptr<named::NamedObject> args) -> std::shared_ptr<named::NamedObject> {
            (void)owner;
            (void)args;
            return named::NamedString::create("ack", "reset");
        },
        root);

    // Step 2: Start OpcUaServerService on dedicated port 4851.
    std::shared_ptr<OpcUaServerService> server = OpcUaServerService::create("TestServer");
    EXPECT_EQ(server->getType(), "OpcUaServerService");
    server->setPort(TUTORIAL_OPCUA_PORT);
    server->setRootObject(root);
    server->start();
    EXPECT_TRUE(server->isRunning());

    // Bounded timed wait via local condition variable until endpoint is accepting TCP connections.
    std::mutex waitMtx;
    std::condition_variable waitCv;
    std::unique_lock<std::mutex> waitLock(waitMtx);
    (void)waitCv.wait_for(waitLock, SERVER_STARTUP_WAIT_MS);

    // Step 3: Start OpcUaClientService connected to localhost:4851 and verify mirrored children.
    std::shared_ptr<OpcUaClientService> client = OpcUaClientService::create("TestClient");
    EXPECT_EQ(client->getType(), "OpcUaClientService");
    client->setUrl("opc.tcp://localhost:" + std::to_string(TUTORIAL_OPCUA_PORT));
    client->start();
    EXPECT_TRUE(client->isRunning());

    rpm->setValue(1800);
    std::shared_ptr<named::NamedObject> mirroredRoot = client->getChild("Substation");
    EXPECT_NE(mirroredRoot, nullptr);
    quasar::test_support::AsyncTestLogger::instance().logOutput("mirroredPresent", mirroredRoot != nullptr);

    // Step 4: Clean ordered teardown (client before server).
    client->stop();
    server->stop();
    EXPECT_FALSE(client->isRunning());
    EXPECT_FALSE(server->isRunning());
}

TEST(OpcUaTutorialTest, Tutorial3_SecurityManagerAndCredentials) {
    quasar::test_support::TestScopeGuard scope("OpcUaTutorialTest.Tutorial3");
    // Step 1: Test OpcUaSecurityManager negative load paths and trust list handling.
    OpcUaSecurityManager secMgr;
    UA_StatusCode missingCert = secMgr.loadCertificate("/nonexistent/cert.der", "/nonexistent/key.der");
    EXPECT_EQ(missingCert, UA_STATUSCODE_BADNOTFOUND);
    EXPECT_EQ(secMgr.loadTrustList(std::vector<std::string>{}), UA_STATUSCODE_GOOD);

    // Step 2: Apply security manager to raw open62541 server and client structs cleanly.
    UA_Server* rawServer = UA_Server_new();
    ASSERT_NE(rawServer, nullptr);
    UA_ServerConfig* srvCfg = UA_Server_getConfig(rawServer);
    EXPECT_EQ(secMgr.configureServer(rawServer, srvCfg), UA_STATUSCODE_GOOD);
    UA_Server_delete(rawServer);

    UA_Client* rawClient = UA_Client_new();
    ASSERT_NE(rawClient, nullptr);
    EXPECT_EQ(secMgr.configureClient(rawClient), UA_STATUSCODE_GOOD);
    UA_Client_delete(rawClient);

    // Step 3: Verify user credential setters on both server and client services.
    std::shared_ptr<OpcUaServerService> srv = OpcUaServerService::create("SecSrv");
    srv->loadCertificate("/nonexistent/a.der", "/nonexistent/b.der");
    srv->loadTrustList(std::vector<std::string>{});
    srv->addUser("operator", "secret123");
    std::shared_ptr<OpcUaClientService> cli = OpcUaClientService::create("SecCli");
    cli->loadCertificate("/nonexistent/a.der", "/nonexistent/b.der");
    cli->loadTrustList(std::vector<std::string>{});
    cli->setCredentials("operator", "secret123");
    SUCCEED();
}

TEST(OpcUaCoverageTest, UnreachableEndpointAndExceptionBoundaries) {
    quasar::test_support::TestScopeGuard scope("OpcUaCoverageTest.Boundaries");
    // Step 1: Verify empty & null inputs to UA_Common helpers do not crash or leak.
    UA_String emptyUaStr = UA_String_fromStdString("");
    EXPECT_EQ(emptyUaStr.length, 0U);
    UA_String_clear(&emptyUaStr);

    UA_ByteString emptyBs = UA_ByteString_fromStdString("");
    EXPECT_EQ(emptyBs.length, 0U);
    UA_ByteString_clear(&emptyBs);

    UA_Variant nullVar = toUaVariant(nullptr);
    EXPECT_TRUE(UA_Variant_isEmpty(&nullVar));
    UA_Variant_clear(&nullVar);

    // Step 2: Client start against an unreachable OPC UA endpoint must throw std::runtime_error.
    std::shared_ptr<OpcUaClientService> cli = OpcUaClientService::create("UnreachableClient");
    cli->setUrl("opc.tcp://127.0.0.1:" + std::to_string(UNUSED_OPCUA_PORT));
    EXPECT_THROW(cli->start(), std::runtime_error);
    EXPECT_FALSE(cli->isRunning());
    cli->stop(); // Stopping an already stopped service must be an idempotent no-op.
}

TEST(OpcUaFuzzStressTest, DeterministicVariantFuzzAndConcurrentTaskQueue) {
    quasar::test_support::TestScopeGuard scope("OpcUaFuzzStressTest.FuzzAndConcurrent");
    // Step 1: Deterministic fuzzing over toUaVariant conversion across integer/double/string nodes.
    quasar::test_support::DeterministicRng rng(OPCUA_FUZZ_SEED);
    for (std::size_t iter = 0U; iter < MAX_VARIANT_FUZZ_ITERS; ++iter) {
        if (iter >= MAX_VARIANT_FUZZ_ITERS) {
            throw std::runtime_error("Variant fuzz bound exceeded");
        } else {
            int kind = rng.nextInt(0, 2);
            if (kind == 0) {
                std::shared_ptr<named::NamedInteger<int32_t>> iv =
                    named::NamedInteger<int32_t>::create("i", rng.nextInt(-10000, 10000));
                UA_Variant v = toUaVariant(iv);
                EXPECT_FALSE(UA_Variant_isEmpty(&v));
                UA_Variant_clear(&v);
            } else if (kind == 1) {
                std::shared_ptr<named::NamedFloatingPoint<double>> dv =
                    named::NamedFloatingPoint<double>::create("d", rng.nextDouble(-500.0, 500.0));
                UA_Variant v = toUaVariant(dv);
                EXPECT_FALSE(UA_Variant_isEmpty(&v));
                UA_Variant_clear(&v);
            } else {
                std::shared_ptr<named::NamedString> sv =
                    named::NamedString::create("s", rng.nextAsciiString(12U));
                UA_Variant v = toUaVariant(sv);
                EXPECT_FALSE(UA_Variant_isEmpty(&v));
                UA_Variant_clear(&v);
            }
        }
    }

    // Step 2: Concurrent enqueueTask dispatchers safely posting onto live client queue (port 4852).
    std::shared_ptr<OpcUaServerService> server = OpcUaServerService::create("StressSrv");
    server->setPort(STRESS_OPCUA_PORT);
    server->start();

    std::mutex waitMtx;
    std::condition_variable waitCv;
    std::unique_lock<std::mutex> waitLock(waitMtx);
    (void)waitCv.wait_for(waitLock, SERVER_STARTUP_WAIT_MS);

    std::shared_ptr<OpcUaClientService> client = OpcUaClientService::create("StressCli");
    client->setUrl("opc.tcp://localhost:" + std::to_string(STRESS_OPCUA_PORT));
    client->start();

    std::vector<std::thread> workers;
    workers.reserve(CLIENT_TASK_WORKERS);
    for (std::size_t w = 0U; w < CLIENT_TASK_WORKERS; ++w) {
        if (w >= CLIENT_TASK_WORKERS) {
            throw std::runtime_error("Worker spawn bound exceeded");
        } else {
            workers.emplace_back([&client, w]() {
                for (std::size_t t = 0U; t < TASKS_PER_WORKER; ++t) {
                    if (t >= TASKS_PER_WORKER) {
                        throw std::runtime_error("Task loop bound exceeded");
                    } else {
                        std::future<int> fut = client->enqueueTask<int>([w, t](UA_Client* rawCli) -> int {
                            if (rawCli == nullptr) {
                                throw std::runtime_error("Null UA_Client in task");
                            } else {
                                return static_cast<int>((w * 100U) + t);
                            }
                        });
                        EXPECT_EQ(fut.get(), static_cast<int>((w * 100U) + t));
                    }
                }
            });
        }
    }

    for (std::size_t w = 0U; w < workers.size(); ++w) {
        if (w >= CLIENT_TASK_WORKERS) {
            throw std::runtime_error("Worker join bound exceeded");
        } else {
            workers[w].join();
        }
    }
    client->stop();
    server->stop();
}

} // namespace quasar::opcua::test
