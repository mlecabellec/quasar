#include <gtest/gtest.h>
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include <memory>

using namespace quasar::named;

class TestBoundPrimitives : public ::testing::Test {
protected:
    void SetUp() override {
    }
};

TEST_F(TestBoundPrimitives, NamedIntegerBinding) {
    std::shared_ptr<NamedBuffer> buffer = NamedBuffer::create("buffer", 16);
    std::shared_ptr<NamedInteger<int32_t>> val = NamedInteger<int32_t>::create("val", 0);
    
    // Bind at offset 4 with LittleEndian to match how we will write to buffer
    val->bind(buffer, 4, quasar::coretypes::Endianness::LittleEndian);
    ASSERT_TRUE(val->isBound());
    ASSERT_EQ(val->getBoundOffset(), 4);
    ASSERT_EQ(val->getBoundLength(), 4);

    // Write to buffer, check integer
    int32_t testVal = 0x12345678;
    buffer->writeInt(testVal, 4, quasar::coretypes::Endianness::LittleEndian);
    
    ASSERT_EQ(val->value(), testVal);

    // Write to integer, check buffer
    val->setValue(0xABCDEF01);
    ASSERT_EQ(buffer->readInt(4, quasar::coretypes::Endianness::LittleEndian), 0xABCDEF01);
}

TEST_F(TestBoundPrimitives, NamedFloatingPointBinding) {
    std::shared_ptr<NamedBuffer> buffer = NamedBuffer::create("buffer", 16);
    std::shared_ptr<NamedFloatingPoint<double>> val = NamedFloatingPoint<double>::create("val", 0.0);
    
    // Bind with LittleEndian (assuming host is LE for the reinterpret_cast below)
    val->bind(buffer, 8, quasar::coretypes::Endianness::LittleEndian);
    ASSERT_TRUE(val->isBound());

    double testVal = 3.14159;
    // We don't have buffer->writeDouble in this version, but we can use template write
    buffer->write<double>(testVal, 8, quasar::coretypes::Endianness::LittleEndian);

    ASSERT_DOUBLE_EQ(val->value(), testVal);

    val->setValue(2.71828);
    ASSERT_DOUBLE_EQ(buffer->read<double>(8, quasar::coretypes::Endianness::LittleEndian), 2.71828);
}

TEST_F(TestBoundPrimitives, NamedBooleanBinding) {
    std::shared_ptr<NamedBuffer> buffer = NamedBuffer::create("buffer", 16);
    std::shared_ptr<NamedBoolean> val = NamedBoolean::create("val", false);
    
    val->bind(buffer, 0);
    
    buffer->set(0, 1);
    ASSERT_TRUE(val->booleanValue());

    val->setValue(false);
    ASSERT_EQ(buffer->get(0), 0);

    buffer->set(0, 0xFF); // Any non-zero should be true
    ASSERT_TRUE(val->booleanValue());
}

TEST_F(TestBoundPrimitives, BoundaryCheck) {
    std::shared_ptr<NamedBuffer> buffer = NamedBuffer::create("buffer", 4);
    std::shared_ptr<NamedInteger<int32_t>> val = NamedInteger<int32_t>::create("val", 0);
    
    // Exactly fits
    EXPECT_NO_THROW(val->bind(buffer, 0));
    
    // Out of range (starts at 1, ends at 5)
    std::shared_ptr<NamedBuffer> buffer2 = NamedBuffer::create("buffer2", 4);
    EXPECT_THROW(val->bind(buffer2, 1), std::out_of_range);
}

TEST_F(TestBoundPrimitives, CloneSharingPolicy) {
    std::shared_ptr<NamedBuffer> buffer = NamedBuffer::create("buffer", 8);
    std::shared_ptr<NamedInteger<int64_t>> val = NamedInteger<int64_t>::create("val", 0);
    val->bind(buffer, 0);
    val->setValue(123); // Now buffer and val are 123

    // Clone with SHARE policy
    std::shared_ptr<NamedObject> cloned = val->clone(CopyPolicy::SHARE);
    std::shared_ptr<NamedInteger<int64_t>> clonedInt = std::dynamic_pointer_cast<NamedInteger<int64_t>>(cloned);
    
    ASSERT_TRUE(clonedInt->isBound());
    ASSERT_EQ(clonedInt->value(), 123);

    // Modify original, cloned should follow
    val->setValue(456);
    ASSERT_EQ(clonedInt->value(), 456);
}
