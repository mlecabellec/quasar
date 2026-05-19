#include <gtest/gtest.h>
#include "quasar/zmq/Context.hpp"
#include "quasar/zmq/Socket.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include <thread>
#include <chrono>
#include <optional>

using namespace quasar::zmq;
using namespace quasar::named;

TEST(ZmqPluginTest, ContextAndSocketCreation) {
    EXPECT_NO_THROW({
        Context ctx;
        Socket pub(ctx, ZMQ_PUB);
        Socket sub(ctx, ZMQ_SUB);
    });
}

TEST(ZmqPluginTest, RawSendReceive) {
    Context ctx;
    Socket pub(ctx, ZMQ_PUB);
    Socket sub(ctx, ZMQ_SUB);
    
    pub.bind("inproc://test_raw");
    sub.connect("inproc://test_raw");
    sub.subscribe(""); 
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    EXPECT_NO_THROW(pub.send("Hello ZMQ", 0));
    
    std::optional<std::string> msg = sub.receive(0);
    ASSERT_TRUE(msg.has_value());
    EXPECT_EQ(msg.value(), "Hello ZMQ");
}

TEST(ZmqPluginTest, PublishReceiveTree) {
    Context ctx;
    Socket pub(ctx, ZMQ_PUB);
    Socket sub(ctx, ZMQ_SUB);
    
    pub.bind("inproc://test_tree");
    sub.connect("inproc://test_tree");
    sub.subscribe("mytopic");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0xFF};
    std::shared_ptr<NamedBuffer> buf = NamedBuffer::create("mybuffer", data);
    
    EXPECT_NO_THROW(pub.publishTree("mytopic", buf));
    
    std::shared_ptr<NamedObject> received;
    EXPECT_NO_THROW({
        received = sub.receiveTree();
    });
    
    ASSERT_NE(received, nullptr);
    EXPECT_EQ(received->getName(), "mybuffer");
    EXPECT_EQ(received->getType(), "NamedBuffer");
    
    std::shared_ptr<NamedBuffer> rxBuf = std::dynamic_pointer_cast<NamedBuffer>(received);
    ASSERT_NE(rxBuf, nullptr);
    EXPECT_EQ(rxBuf->toVector(), data);
}

TEST(ZmqPluginTest, TopicFiltering) {
    Context ctx;
    Socket pub(ctx, ZMQ_PUB);
    Socket sub(ctx, ZMQ_SUB);
    
    pub.bind("inproc://test_topic");
    sub.connect("inproc://test_topic");
    sub.subscribe("topicA");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    std::vector<uint8_t> data1 = {0x0A};
    std::vector<uint8_t> data2 = {0x0B};
    std::shared_ptr<NamedBuffer> buf1 = NamedBuffer::create("A", data1);
    std::shared_ptr<NamedBuffer> buf2 = NamedBuffer::create("B", data2);
    
    // Publish to topicB (should be filtered)
    pub.publishTree("topicB", buf2);
    // Publish to topicA (should be received)
    pub.publishTree("topicA", buf1);
    
    std::shared_ptr<NamedObject> received = sub.receiveTree();
    ASSERT_NE(received, nullptr);
    EXPECT_EQ(received->getName(), "A");
}
