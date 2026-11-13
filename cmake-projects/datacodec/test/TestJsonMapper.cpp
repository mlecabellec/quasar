/**
 * @file TestJsonMapper.cpp
 * @brief GoogleTest suite validating JsonMapper schema conversion and error classification.
 * @details Exercises valid object mapping, missing required fields, type mismatches, extra unknown
 * properties, and malformed JSON syntax handling in JsonMapper::toNamedObject().
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100, FE-0020
 * @compliance Satisfies CS-0090.1..CS-0090.9 fixture/test documentation, branch comments, and CS-0010.44 density.
 */
// GoogleTest framework and JsonMapper/codec/named test dependencies.
#include <gtest/gtest.h>
#include "datacodec/JsonMapper.hpp"
#include "datacodec/IntegerCodec.hpp"
#include "datacodec/StringCodec.hpp"
#include "quasar/named/NamedInteger.hpp"
// NamedString wrapper and standard stream output headers.
#include "quasar/named/NamedString.hpp"
#include <iostream>

// Bring datacodec and named symbols into test translation-unit scope.
using namespace datacodec;
using namespace quasar::named;

/**
 * @class JsonMapperTest
 * @brief GoogleTest fixture provisioning a standard two-field ContainerDef test schema.
 * @details Builds a "TestContainer" schema with an integer "status" field and string "message"
 * field prior to each test case.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100, FE-0020
 * @compliance Documents fixture class, setup override, member schema, and containment (CS-0090.1..CS-0090.9).
 * @invariant m_schema holds valid "status" (64-bit IntegerCodec) and "message" (StringCodec) fields after SetUp().
 */
class JsonMapperTest : public ::testing::Test {
protected:
    /**
     * @brief Initializes m_schema before each TEST_F case runs.
     * @details Creates TestContainer schema and attaches status and message FieldDef nodes.
     * @task TSK-20261113-003, TSK-20260303-002
     * @feature FE-0010, FE-0100, FE-0020
     * @compliance Documents fixture setup method and error containment (CS-0090.1, CS-0090.2, CS-0090.8, CS-0090.9).
     * @post m_schema != nullptr and contains 2 FieldDef entries ("status", "message").
     * @throws std::exception if ContainerDef or FieldDef allocation fails.
     * @containment
     *   Approach: Reconstructs fresh shared_ptr schema per test to eliminate cross-test state leakage.
     *   Limitations: Assumes heap allocation succeeds in the test environment.
     */
    void SetUp() override {
        // Construct fresh schema container with status (int64) and message (string) fields.
        m_schema = ContainerDef::create("TestContainer");
        m_schema->addField(FieldDef::create("status", std::make_shared<IntegerCodec<int64_t>>(64)));
        m_schema->addField(FieldDef::create("message", std::make_shared<StringCodec>()));
    }

    /**
     * @brief Shared ContainerDef fixture schema consumed by each JSON mapping test case.
     * @details Re-initialized cleanly inside SetUp() before every test case execution.
     * @task TSK-20261113-003, TSK-20260303-002
     * @feature FE-0010, FE-0100
     * @compliance Fixture member field documented per CS-0090.1 and CS-0090.2.
     */
    std::shared_ptr<ContainerDef> m_schema;
};

/**
 * @brief Verifies valid JSON object mapping produces populated NamedObject root and typed children.
 * @details Maps {"status": 200, "message": "OK"} and validates root name, integer width fallback
 * inspection for "status", and NamedString content for "message".
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100, FE-0020
 * @compliance Documents valid mapping test, branch inspection comments, and density (CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6).
 */
TEST_F(JsonMapperTest, ValidMapping) {
    // Parse schema-compliant JSON string and verify successful std::expected result.
    std::string json = R"({"status": 200, "message": "OK"})";
    std::expected<std::shared_ptr<NamedObject>, JsonMapperErrorCode> result = JsonMapper::toNamedObject(json, *m_schema);
    
    ASSERT_TRUE(result.has_value()) << "Mapping failed with error code: " << static_cast<int>(result.error());
    std::shared_ptr<NamedObject> root = result.value();
    EXPECT_EQ(root->getName(), "TestContainer");
    
    // Locate child "status" node and verify its runtime NamedInteger specialization holds 200.
    std::shared_ptr<NamedObject> statusNode = root->getChild("status");
    ASSERT_NE(statusNode, nullptr) << "Child 'status' not found in tree";
    
    std::cout << "Status node type: " << statusNode->getType() << std::endl;

    std::shared_ptr<NamedInteger<int64_t>> status = std::dynamic_pointer_cast<NamedInteger<int64_t>>(statusNode);
    // Fall back across uint64_t and long int widths in case std::int64_t differs from platform long.
    if (!status) {
        std::shared_ptr<NamedInteger<uint64_t>> statusU = std::dynamic_pointer_cast<NamedInteger<uint64_t>>(statusNode);
        // Check signed long int variant when unsigned 64-bit cast fails.
        if (!statusU) {
             std::shared_ptr<NamedInteger<long int>> statusL = std::dynamic_pointer_cast<NamedInteger<long int>>(statusNode);
             ASSERT_NE(statusL, nullptr) << "Child 'status' is not a NamedInteger of any expected width. Type: " << statusNode->getType();
             EXPECT_EQ(statusL->value(), 200);
        // Assert expected value 200 on unsigned 64-bit NamedInteger fallback.
        } else {
             EXPECT_EQ(statusU->value(), 200);
        }
    // Assert expected value 200 on exact int64_t NamedInteger match.
    } else {
        EXPECT_EQ(status->value(), 200);
    }
    
    // Locate child "message" node and confirm NamedString equals "OK".
    std::shared_ptr<NamedString> message = std::dynamic_pointer_cast<NamedString>(root->getChild("message"));
    ASSERT_NE(message, nullptr) << "Child 'message' is not a NamedString";
    EXPECT_EQ(message->toString(), "OK");
}

/**
 * @brief Verifies omission of a required schema field yields JsonMapperErrorCode::MissingField.
 * @details Maps {"status": 200} without "message" and confirms std::expected returns MissingField.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @compliance Documents MissingField negative test case (CS-0090.1, CS-0090.2, CS-0090.5).
 */
TEST_F(JsonMapperTest, MissingField) {
    // Submit JSON object missing the required "message" property and verify MissingField error.
    std::string json = R"({"status": 200})";
    std::expected<std::shared_ptr<NamedObject>, JsonMapperErrorCode> result = JsonMapper::toNamedObject(json, *m_schema);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), JsonMapperErrorCode::MissingField);
}

/**
 * @brief Verifies type mismatch between JSON token and codec yields JsonMapperErrorCode::TypeMismatch.
 * @details Passes string "wrong" for integer field "status" and checks returned TypeMismatch code.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @compliance Documents TypeMismatch negative test case (CS-0090.1, CS-0090.2, CS-0090.5).
 */
TEST_F(JsonMapperTest, TypeMismatch) {
    // Submit string token for integer "status" field and verify TypeMismatch error.
    std::string json = R"({"status": "wrong", "message": "OK"})";
    std::expected<std::shared_ptr<NamedObject>, JsonMapperErrorCode> result = JsonMapper::toNamedObject(json, *m_schema);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), JsonMapperErrorCode::TypeMismatch);
}

/**
 * @brief Verifies unexpected JSON keys yield JsonMapperErrorCode::ExtraField.
 * @details Supplies undeclared property "extra" alongside valid fields and asserts ExtraField error.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @compliance Documents ExtraField negative test case (CS-0090.1, CS-0090.2, CS-0090.5).
 */
TEST_F(JsonMapperTest, ExtraField) {
    // Submit JSON object with undeclared "extra" property and verify ExtraField error.
    std::string json = R"({"status": 200, "message": "OK", "extra": 123})";
    std::expected<std::shared_ptr<NamedObject>, JsonMapperErrorCode> result = JsonMapper::toNamedObject(json, *m_schema);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), JsonMapperErrorCode::ExtraField);
}

/**
 * @brief Verifies syntactically malformed JSON yields JsonMapperErrorCode::InvalidJson.
 * @details Supplies unclosed JSON object string and asserts InvalidJson error containment.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @compliance Documents InvalidJson syntax-error containment test case (CS-0090.1, CS-0090.2, CS-0090.5).
 */
TEST_F(JsonMapperTest, InvalidJson) {
    // Submit syntactically unclosed JSON object and verify InvalidJson error.
    std::string json = R"({"status": 200, "message": "OK")"; 
    std::expected<std::shared_ptr<NamedObject>, JsonMapperErrorCode> result = JsonMapper::toNamedObject(json, *m_schema);
    
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), JsonMapperErrorCode::InvalidJson);
}
