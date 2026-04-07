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
    void SetUp() override {
        // Setup Server
        serverRoot = NamedObject::create("ServerRoot");
        serverData = NamedObject::create("Data", serverRoot);
        serverInt = NamedInteger<int32_t>::create("MyInt", 42, serverData);
        
        serverMethod = NamedMethod::create("Multiply", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
            (void)owner;
            if (!args || args->getType() != "NamedInteger") return nullptr;
            auto input = std::dynamic_pointer_cast<NamedInteger<int64_t>>(args);
            if (!input) return nullptr;
            return NamedInteger<int64_t>::create("result", input->value() * 2);
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
    
    std::shared_ptr<OpcUaServerService> server;
    std::shared_ptr<OpcUaClientService> client;
};

TEST_F(OpcUaIntegrationTest, MirroringAndValueSync) {
    EXPECT_NO_THROW(client->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Allow mirroring to complete
    
    // Verify Mirroring
    auto mirroredRoot = client->getChild("ServerRoot");
    ASSERT_NE(mirroredRoot, nullptr) << "Failed to mirror ServerRoot";
    
    auto mirroredData = mirroredRoot->getChild("Data");
    ASSERT_NE(mirroredData, nullptr) << "Failed to mirror Data";
    
    auto mirroredIntObj = mirroredData->getChild("MyInt");
    ASSERT_NE(mirroredIntObj, nullptr) << "Failed to mirror MyInt";
    
    auto mirroredInt = std::dynamic_pointer_cast<NamedInteger<int32_t>>(mirroredIntObj);
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
    
    auto mirroredRoot = client->getChild("ServerRoot");
    ASSERT_NE(mirroredRoot, nullptr);
    
    auto mirroredData = mirroredRoot->getChild("Data");
    ASSERT_NE(mirroredData, nullptr);
    
    auto mirroredMethodObj = mirroredData->getChild("Multiply");
    ASSERT_NE(mirroredMethodObj, nullptr) << "Failed to mirror Multiply method";
    
    auto mirroredMethod = std::dynamic_pointer_cast<NamedMethod>(mirroredMethodObj);
    ASSERT_NE(mirroredMethod, nullptr) << "Multiply is not a NamedMethod";
    
    // Execute method remotely
    auto args = NamedInteger<int64_t>::create("input", 21);
    auto resultObj = mirroredMethod->execute(args);
    
    ASSERT_NE(resultObj, nullptr) << "Remote method execution returned nullptr (timed out or failed)";
    
    auto resultInt = std::dynamic_pointer_cast<NamedInteger<int64_t>>(resultObj);
    ASSERT_NE(resultInt, nullptr) << "Result is not a NamedInteger<int64_t>";
    
    EXPECT_EQ(resultInt->value(), 42) << "Mathematical operation failed remotely";
}
