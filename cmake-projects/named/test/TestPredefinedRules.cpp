#include <gtest/gtest.h>
#include "quasar/named/traversal/PredefinedRules.hpp"
#include "quasar/named/traversal/Transformer.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedBufferSlice.hpp"
#include "quasar/named/NamedInteger.hpp"

using namespace quasar::named;
using namespace quasar::named::traversal;

class PredefinedRulesTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
};

TEST_F(PredefinedRulesTest, SliceBufferRuleShare) {
    std::vector<uint8_t> data = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
    auto root = NamedBuffer::create("Payload", data);

    Transformer transformer;
    transformer.addRule(PredefinedRules::sliceBuffer("Payload", {
        {"Header", 0, 2},
        {"Body", 2, 4}
    }, CopyPolicy::SHARE));

    auto resultVec = transformer.transform(root);
    ASSERT_EQ(resultVec.size(), 2);

    auto header = resultVec[0]->as<NamedBufferSlice>();
    ASSERT_NE(header, nullptr);
    EXPECT_EQ(header->getName(), "Header");
    EXPECT_EQ(header->size(), 2);
    EXPECT_EQ(header->get(0), 0x11);
    EXPECT_EQ(header->get(1), 0x22);

    auto body = resultVec[1]->as<NamedBufferSlice>();
    ASSERT_NE(body, nullptr);
    EXPECT_EQ(body->getName(), "Body");
    EXPECT_EQ(body->size(), 4);
    EXPECT_EQ(body->get(0), 0x33);

    // Verify sharing: modifying original buffer modifies the slice view
    root->set(0, 0xAA);
    EXPECT_EQ(header->get(0), 0xAA);
}

TEST_F(PredefinedRulesTest, ExtractIntegerRule) {
    std::vector<uint8_t> data = {0x00, 0x00, 0x01, 0x2C}; // 300 in BigEndian 32-bit
    auto root = NamedBuffer::create("Payload", data);

    Transformer transformer;
    transformer.addRule(PredefinedRules::extractIntegerRule<int>("Payload", "ExtractedInt", 0));

    auto resultVec = transformer.transform(root);
    ASSERT_EQ(resultVec.size(), 1);

    auto intObj = resultVec[0]->as<NamedInteger<int>>();
    ASSERT_NE(intObj, nullptr);
    EXPECT_EQ(intObj->getName(), "ExtractedInt");
    EXPECT_EQ(intObj->value(), 300);
}
