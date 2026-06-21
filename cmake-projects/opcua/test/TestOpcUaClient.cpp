#include <gtest/gtest.h>
#include "quasar/opcua/OpcUaClientService.hpp"
#include "quasar/opcua/OpcUaServerService.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedMethod.hpp"
#include "quasar/named/Serialization.hpp"
#include <thread>
#include <chrono>

using namespace quasar::named;
using namespace quasar::opcua;

class OpcUaIntegrationTest : public ::testing::Test {
protected:
    /// @brief Set up integration test suite.
    void SetUp() override {
        // Setup Server
        serverRoot = NamedObject::create("ServerRoot");
        serverData = NamedObject::create("Data", serverRoot);
        serverInt = NamedInteger<int32_t>::create("MyInt", 42, serverData);
        
        serverMethod = NamedMethod::create("Multiply", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
            (void)owner;
            if (!args) return nullptr;
            
            int64_t val = 0;
            if (std::shared_ptr<NamedInteger<int32_t>> i32 = std::dynamic_pointer_cast<NamedInteger<int32_t>>(args)) {
                val = i32->value();
            } else if (std::shared_ptr<NamedInteger<int64_t>> i64 = std::dynamic_pointer_cast<NamedInteger<int64_t>>(args)) {
                val = i64->value();
            } else {
                return nullptr;
            }
            
            return NamedInteger<int64_t>::create("result", val * 2);
        }, serverData);

        serverVoidMethod = NamedMethod::create("VoidMethod", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
            (void)owner; (void)args;
            return nullptr;
        }, serverData);

        server = OpcUaServerService::create("OpcUaServer");
        server->setPort(4842); // Port for integration test
        server->setRootObject(serverRoot);
        server->start();
        
        // Give server time to bind
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Setup Client
        client = OpcUaClientService::create("OpcUaClient");
        client->setUrl("opc.tcp://localhost:4842");
    }

    /// @brief Tear down integration test suite.
    void TearDown() override {
        if (client && client->isRunning()) {
            client->stop();
        }
        if (server && server->isRunning()) {
            server->stop();
        }
    }

    std::shared_ptr<NamedObject> serverRoot;
    std::shared_ptr<NamedObject> serverData;
    std::shared_ptr<NamedInteger<int32_t>> serverInt;
    std::shared_ptr<NamedMethod> serverMethod;
    
    /**
     * @brief Mirrored void method on the OPC UA server for testing.
     */
    std::shared_ptr<NamedMethod> serverVoidMethod;
    
    std::shared_ptr<OpcUaServerService> server;
    std::shared_ptr<OpcUaClientService> client;
};

TEST_F(OpcUaIntegrationTest, MirroringAndValueSync) {
    EXPECT_NO_THROW(client->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Allow mirroring to complete
    
    // Verify Mirroring
    std::shared_ptr<NamedObject> mirroredRoot = client->getChild("ServerRoot");
    ASSERT_NE(mirroredRoot, nullptr) << "Failed to mirror ServerRoot";
    
    std::shared_ptr<NamedObject> mirroredData = mirroredRoot->getChild("Data");
    ASSERT_NE(mirroredData, nullptr) << "Failed to mirror Data";
    
    std::shared_ptr<NamedObject> mirroredIntObj = mirroredData->getChild("MyInt");
    ASSERT_NE(mirroredIntObj, nullptr) << "Failed to mirror MyInt";
    
    std::shared_ptr<NamedInteger<int32_t>> mirroredInt = std::dynamic_pointer_cast<NamedInteger<int32_t>>(mirroredIntObj);
    ASSERT_NE(mirroredInt, nullptr) << "MyInt is not a NamedInteger<int32_t>";
    
    // Check initial value
    EXPECT_EQ(mirroredInt->value(), 42);
    
    // Test server to client sync
    serverInt->setValue(100);
    
    // Wait for the OPC UA subscription to trigger the client
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    EXPECT_EQ(mirroredInt->value(), 100) << "Value did not synchronize from server to client";
}

TEST_F(OpcUaIntegrationTest, MethodExecution) {
    EXPECT_NO_THROW(client->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Allow mirroring
    
    std::shared_ptr<NamedObject> mirroredRoot = client->getChild("ServerRoot");
    ASSERT_NE(mirroredRoot, nullptr);
    
    std::shared_ptr<NamedObject> mirroredData = mirroredRoot->getChild("Data");
    ASSERT_NE(mirroredData, nullptr);
    
    std::shared_ptr<NamedObject> mirroredMethodObj = mirroredData->getChild("Multiply");
    ASSERT_NE(mirroredMethodObj, nullptr) << "Failed to mirror Multiply method";
    
    std::shared_ptr<NamedMethod> mirroredMethod = std::dynamic_pointer_cast<NamedMethod>(mirroredMethodObj);
    ASSERT_NE(mirroredMethod, nullptr) << "Multiply is not a NamedMethod";
    
    // Execute method remotely
    std::shared_ptr<NamedInteger<int64_t>> args = NamedInteger<int64_t>::create("input", 21);
    std::shared_ptr<NamedObject> resultObj = mirroredMethod->execute(args);
    
    ASSERT_NE(resultObj, nullptr) << "Remote method execution returned nullptr (timed out or failed)";
    
    std::shared_ptr<quasar::coretypes::Number> resultNum = std::dynamic_pointer_cast<quasar::coretypes::Number>(resultObj);
    ASSERT_NE(resultNum, nullptr) << "Result is not a coretypes::Number";
    
    EXPECT_EQ(resultNum->toInt64(), 42) << "Mathematical operation failed remotely";
}

/**
 * @brief Integration test verifying successful remote execution of void methods over OPC UA.
 */
TEST_F(OpcUaIntegrationTest, VoidMethodExecution) {
    EXPECT_NO_THROW(client->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Allow mirroring
    
    std::shared_ptr<NamedObject> mirroredRoot = client->getChild("ServerRoot");
    ASSERT_NE(mirroredRoot, nullptr);
    
    std::shared_ptr<NamedObject> mirroredData = mirroredRoot->getChild("Data");
    ASSERT_NE(mirroredData, nullptr);
    
    std::shared_ptr<NamedObject> mirroredMethodObj = mirroredData->getChild("VoidMethod");
    ASSERT_NE(mirroredMethodObj, nullptr) << "Failed to mirror VoidMethod method";
    
    std::shared_ptr<NamedMethod> mirroredMethod = std::dynamic_pointer_cast<NamedMethod>(mirroredMethodObj);
    ASSERT_NE(mirroredMethod, nullptr) << "VoidMethod is not a NamedMethod";
    
    std::shared_ptr<NamedObject> resultObj = mirroredMethod->execute(nullptr);
    ASSERT_NE(resultObj, nullptr);
    EXPECT_EQ(resultObj->getName(), "void");
}
