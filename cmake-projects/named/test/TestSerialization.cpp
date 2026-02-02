#include <gtest/gtest.h>
#include "quasar/named/Serialization.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedBitBuffer.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"

using namespace quasar::named;

class SerializationTest : public ::testing::Test {
protected:
    std::shared_ptr<NamedObject> createTestHierarchy() {
        auto root = NamedObject::create("root");
        
        NamedInteger<int32_t>::create("intVal", 42, root);
        NamedBoolean::create("boolVal", true, root);
        NamedString::create("stringVal", "test", root);
        NamedFloatingPoint<double>::create("floatVal", 3.14, root);
        
        std::vector<uint8_t> data = {0xAA, 0xBB};
        NamedBuffer::create("bufferVal", data, root);
        
        auto bb = NamedBitBuffer::create("bitBufferVal", 16, root);
        bb->setBit(0, true);
        
        return root;
    }
    
    void verifyHierarchy(const std::shared_ptr<NamedObject>& obj) {
        ASSERT_TRUE(obj != nullptr);
        EXPECT_EQ(obj->getName(), "root");
        
        auto children = obj->getChildren();
        EXPECT_EQ(children.size(), 6);
        
        // Find children by name to verify
        bool foundInt = false;
        bool foundBool = false;
        bool foundString = false;
        bool foundFloat = false;
        bool foundBuffer = false;
        bool foundBitBuffer = false;
        
        for(const auto& child : children) {
            if (child->getName() == "intVal") {
                foundInt = true;
                auto i = std::dynamic_pointer_cast<NamedInteger<int32_t>>(child);
                if (!i) {
                     // Maybe deserialized as int64?
                     auto i64 = std::dynamic_pointer_cast<NamedInteger<int64_t>>(child);
                     ASSERT_TRUE(i64 != nullptr);
                     EXPECT_EQ(i64->toLong(), 42);
                } else {
                    EXPECT_EQ(i->toInt(), 42);
                }
            } else if (child->getName() == "boolVal") {
                foundBool = true;
                auto b = std::dynamic_pointer_cast<NamedBoolean>(child);
                ASSERT_TRUE(b != nullptr);
                EXPECT_TRUE(b->booleanValue());
            } else if (child->getName() == "stringVal") {
                foundString = true;
                auto s = std::dynamic_pointer_cast<NamedString>(child);
                ASSERT_TRUE(s != nullptr);
                EXPECT_EQ(s->toString(), "test");
            } else if (child->getName() == "floatVal") {
                foundFloat = true;
                auto f = std::dynamic_pointer_cast<NamedFloatingPoint<double>>(child);
                ASSERT_TRUE(f != nullptr);
                EXPECT_DOUBLE_EQ(f->toDouble(), 3.14);
            } else if (child->getName() == "bufferVal") {
                foundBuffer = true;
                auto b = std::dynamic_pointer_cast<NamedBuffer>(child);
                ASSERT_TRUE(b != nullptr);
                EXPECT_EQ(b->size(), 2);
                EXPECT_EQ(b->get(0), 0xAA);
            } else if (child->getName() == "bitBufferVal") {
                foundBitBuffer = true;
                auto bb = std::dynamic_pointer_cast<NamedBitBuffer>(child);
                ASSERT_TRUE(bb != nullptr);
                EXPECT_EQ(bb->bitSize(), 16);
                EXPECT_TRUE(bb->getBit(0));
            }
        }
        
        EXPECT_TRUE(foundInt);
        EXPECT_TRUE(foundBool);
        EXPECT_TRUE(foundString);
        EXPECT_TRUE(foundFloat);
        EXPECT_TRUE(foundBuffer);
        EXPECT_TRUE(foundBitBuffer);
    }
};

TEST_F(SerializationTest, XMLRoundTrip) {
    auto root = createTestHierarchy();
    std::string xml = serialization::toXml(root);
    // std::cout << xml << std::endl;
    auto restored = serialization::fromXml(xml);
    verifyHierarchy(restored);
}

TEST_F(SerializationTest, YAMLRoundTrip) {
    auto root = createTestHierarchy();
    std::string yaml = serialization::toYaml(root);
    // std::cout << yaml << std::endl;
    auto restored = serialization::fromYaml(yaml);
    verifyHierarchy(restored);
}

TEST_F(SerializationTest, JSONRoundTrip) {
    auto root = createTestHierarchy();
    std::string json = serialization::toJson(root);
    // std::cout << json << std::endl;
    auto restored = serialization::fromJson(json);
    verifyHierarchy(restored);
}
