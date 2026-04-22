#include <gtest/gtest.h>
#include "datacodec/JsonMapper.hpp"
#include "datacodec/IntegerCodec.hpp"
#include "datacodec/StringCodec.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedString.hpp"
#include <iostream>

using namespace datacodec;
using namespace quasar::named;

class JsonMapperTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_schema = ContainerDef::create("TestContainer");
        m_schema->addField(FieldDef::create("status", std::make_shared<IntegerCodec<int64_t>>(64)));
        m_schema->addField(FieldDef::create("message", std::make_shared<StringCodec>()));
    }

    std::shared_ptr<ContainerDef> m_schema;
};

TEST_F(JsonMapperTest, ValidMapping) {
    std::string json = R"({"status": 200, "message": "OK"})";
    std::expected<std::shared_ptr<NamedObject>, JsonMapperErrorCode> result = JsonMapper::toNamedObject(json, *m_schema);
    
    ASSERT_TRUE(result.has_value()) << "Mapping failed with error code: " << static_cast<int>(result.error());
    std::shared_ptr<NamedObject> root = result.value();
    EXPECT_EQ(root->getName(), "TestContainer");
    
    std::shared_ptr<NamedObject> statusNode = root->getChild("status");
    ASSERT_NE(statusNode, nullptr) << "Child 'status' not found in tree";
    
    std::cout << "Status node type: " << statusNode->getType() << std::endl;

    std::shared_ptr<NamedInteger<int64_t>> status = std::dynamic_pointer_cast<NamedInteger<int64_t>>(statusNode);
    if (!status) {
        std::shared_ptr<NamedInteger<uint64_t>> statusU = std::dynamic_pointer_cast<NamedInteger<uint64_t>>(statusNode);
        if (!statusU) {
             std::shared_ptr<NamedInteger<long int>> statusL = std::dynamic_pointer_cast<NamedInteger<long int>>(statusNode);
             ASSERT_NE(statusL, nullptr) << "Child 'status' is not a NamedInteger of any expected width. Type: " << statusNode->getType();
             EXPECT_EQ(statusL->value(), 200);
        } else {
             EXPECT_EQ(statusU->value(), 200);
        }
    } else {
        EXPECT_EQ(status->value(), 200);
    }
    
    std::shared_ptr<NamedString> message = std::dynamic_pointer_cast<NamedString>(root->getChild("message"));
    ASSERT_NE(message, nullptr) << "Child 'message' is not a NamedString";
    EXPECT_EQ(message->toString(), "OK");
}

TEST_F(JsonMapperTest, MissingField) {
    std::string json = R"({"status": 200})";
    std::expected<std::shared_ptr<NamedObject>, JsonMapperErrorCode> result = JsonMapper::toNamedObject(json, *m_schema);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), JsonMapperErrorCode::MissingField);
}

TEST_F(JsonMapperTest, TypeMismatch) {
    std::string json = R"({"status": "wrong", "message": "OK"})";
    std::expected<std::shared_ptr<NamedObject>, JsonMapperErrorCode> result = JsonMapper::toNamedObject(json, *m_schema);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), JsonMapperErrorCode::TypeMismatch);
}

TEST_F(JsonMapperTest, ExtraField) {
    std::string json = R"({"status": 200, "message": "OK", "extra": 123})";
    std::expected<std::shared_ptr<NamedObject>, JsonMapperErrorCode> result = JsonMapper::toNamedObject(json, *m_schema);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), JsonMapperErrorCode::ExtraField);
}

TEST_F(JsonMapperTest, InvalidJson) {
    std::string json = R"({"status": 200, "message": "OK")"; 
    std::expected<std::shared_ptr<NamedObject>, JsonMapperErrorCode> result = JsonMapper::toNamedObject(json, *m_schema);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), JsonMapperErrorCode::InvalidJson);
}
