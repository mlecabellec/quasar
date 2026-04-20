#include <gtest/gtest.h>
#include "quasar/named/traversal/Transformer.hpp"
#include "quasar/named/traversal/PredefinedRules.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include <memory>
#include <vector>

using namespace quasar::named;
using namespace quasar::named::traversal;

class ProtocolMappingTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(ProtocolMappingTest, CastToStructureZeroCopy) {
    // Simulate an EtherCAT-like packet (8 bytes)
    // Offset 0: Status (Int32)
    // Offset 4: Temperature (Double/Float64)
    // Offset 12: Alarm (Bool) -> total 13 bytes
    std::shared_ptr<NamedBuffer> rawData = NamedBuffer::create("Packet", 16);
    
    // Initialize data
    rawData->writeInt(0x1234, 0, quasar::coretypes::Endianness::LittleEndian);
    double temp = 36.6;
    uint8_t* tptr = reinterpret_cast<uint8_t*>(&temp);
    for(size_t i=0; i<8; ++i) rawData->set(4+i, tptr[i]);
    rawData->set(12, 1);

    Transformer transformer;
    
    std::vector<FieldMapping> mappings = {
        {"Status", "int32", 0},
        {"Temperature", "float64", 4},
        {"Alarm", "bool", 12}
    };

    transformer.addRule(PredefinedRules::castToStructure("Packet", mappings));

    std::vector<std::shared_ptr<NamedObject>> result = transformer.transform(rawData);
    ASSERT_EQ(result.size(), 1);
    
    std::shared_ptr<NamedObject> packetView = result[0];
    ASSERT_EQ(packetView->getName(), "Packet");
    ASSERT_EQ(packetView->getType(), "NamedBuffer"); // It's a clone of the buffer

    // Check children (the pseudo-primitives)
    std::shared_ptr<NamedInteger<int32_t>> status = std::dynamic_pointer_cast<NamedInteger<int32_t>>(packetView->getChild("Status"));
    std::shared_ptr<NamedFloatingPoint<double>> temperature = std::dynamic_pointer_cast<NamedFloatingPoint<double>>(packetView->getChild("Temperature"));
    std::shared_ptr<NamedBoolean> alarm = std::dynamic_pointer_cast<NamedBoolean>(packetView->getChild("Alarm"));

    ASSERT_NE(status, nullptr);
    ASSERT_NE(temperature, nullptr);
    ASSERT_NE(alarm, nullptr);

    ASSERT_EQ(status->value(), 0x1234);
    ASSERT_DOUBLE_EQ(temperature->value(), 36.6);
    ASSERT_TRUE(alarm->booleanValue());

    // Verify ZERO-COPY: modify rawData, check packetView children
    rawData->writeInt(0x5678, 0, quasar::coretypes::Endianness::LittleEndian);
    ASSERT_EQ(status->value(), 0x5678);

    temperature->setValue(40.0);
    double val40 = temperature->value();
    ASSERT_EQ(rawData->readInt(4, quasar::coretypes::Endianness::LittleEndian), *reinterpret_cast<int32_t*>(&val40)); // Assuming same layout
}

TEST_F(ProtocolMappingTest, ExtractIntegerZeroCopy) {
    std::shared_ptr<NamedBuffer> buf = NamedBuffer::create("Data", 8);
    buf->writeInt(42, 0);

    Transformer transformer;
    transformer.addRule(PredefinedRules::extractIntegerRule<int32_t>("Data", "Value", 0));

    std::vector<std::shared_ptr<NamedObject>> result = transformer.transform(buf);
    ASSERT_EQ(result.size(), 1);
    std::shared_ptr<NamedInteger<int32_t>> val = std::dynamic_pointer_cast<NamedInteger<int32_t>>(result[0]);
    
    ASSERT_NE(val, nullptr);
    ASSERT_EQ(val->value(), 42);
    ASSERT_TRUE(val->isBound());

    // Change buffer, check bound integer
    buf->writeInt(100, 0);
    ASSERT_EQ(val->value(), 100);
}
