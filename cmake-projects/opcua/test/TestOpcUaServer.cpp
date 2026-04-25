#include <gtest/gtest.h>
#include "quasar/opcua/OpcUaServerService.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedMethod.hpp"
#include "quasar/named/Serialization.hpp"
#include <thread>
#include <chrono>

using namespace quasar::named;
using namespace quasar::opcua;

class OpcUaServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        root = NamedObject::create("Root");
        data = NamedObject::create("Data", root);
        myInt = NamedInteger<int32_t>::create("MyInt", 42, data);
        
        server = OpcUaServerService::create("OpcUaServer");
        server->setPort(4841); // Use a different port to avoid conflicts
        server->setRootObject(root);
    }

    void TearDown() override {
        if (server && server->isRunning()) {
            server->stop();
        }
    }

    std::shared_ptr<NamedObject> root;
    std::shared_ptr<NamedObject> data;
    std::shared_ptr<NamedInteger<int32_t>> myInt;
    std::shared_ptr<OpcUaServerService> server;
};

TEST_F(OpcUaServerTest, BasicStartupAndMapping) {
    EXPECT_NO_THROW(server->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Let it spin up
    EXPECT_TRUE(server->isRunning());
    
    // Test dynamic value update
    EXPECT_NO_THROW(myInt->setValue(100));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    EXPECT_NO_THROW(server->stop());
    EXPECT_FALSE(server->isRunning());
}

TEST_F(OpcUaServerTest, MethodMappingAndExecution) {
    std::shared_ptr<NamedMethod> multiplyMethod = NamedMethod::create("Multiply", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
        (void)owner;
        if (!args || args->getType() != "NamedInteger") return nullptr;
        std::shared_ptr<NamedInteger<int64_t>> input = std::dynamic_pointer_cast<NamedInteger<int64_t>>(args);
        if (!input) return nullptr;
        return NamedInteger<int64_t>::create("result", input->value() * 2);
    }, data);

    EXPECT_NO_THROW(server->start());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(server->isRunning());
    
    EXPECT_NO_THROW(server->stop());
}
