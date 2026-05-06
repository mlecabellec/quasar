#include "quasar/named/NamedBitBuffer.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/Serialization.hpp"
#include "quasar/named/NamedTimestamp.hpp"
#include "quasar/named/NamedDuration.hpp"
#include "quasar/named/NamedDate.hpp"
#include "quasar/named/NamedQuantity.hpp"
#include "quasar/named/NamedArray.hpp"
#include "quasar/named/NamedMap.hpp"
#include "quasar/named/NamedVariant.hpp"

#include <gtest/gtest.h>
#include <iostream>

using namespace quasar::named;

class SerializationTest : public ::testing::Test {
protected:
  std::shared_ptr<NamedObject> createTestHierarchy() {
    // Step: Create test hierarchy for serialization
    std::cout << "Step: Create test hierarchy for serialization" << std::endl;
    std::shared_ptr<NamedObject> root = NamedObject::create("root");

    // Step: Create various named types as children
    std::cout << "Step: Create various named types as children" << std::endl;
    NamedInteger<int32_t>::create("intVal", 42, root);
    NamedBoolean::create("boolVal", true, root);
    NamedString::create("stringVal", "test", root);
    NamedFloatingPoint<double>::create("floatVal", 3.14, root);

    std::vector<uint8_t> data = {0xAA, 0xBB};
    NamedBuffer::create("bufferVal", data, root);

    std::shared_ptr<NamedBitBuffer> bb =
        NamedBitBuffer::create("bitBufferVal", 16, root);
    bb->setBit(0, true);

    NamedDate::create("dateVal", 19000, root);
    NamedTimestamp::create("tsVal", 1680000000000000, root);
    NamedDuration::create("durVal", 42000000, root);
    NamedQuantity::create("qtyVal", 220.5, quasar::coretypes::Units::Volt, root);

    std::shared_ptr<NamedArray<NamedObject>> array = NamedArray<NamedObject>::create("arrayVal", root);
    array->push_back(NamedInteger<int32_t>::create("item0", 123));
    
    std::shared_ptr<NamedMap<NamedObject>> map = NamedMap<NamedObject>::create("mapVal", root);
    map->put("key1", NamedString::create("item1", "mapped"));
    
    std::shared_ptr<NamedVariant> variant = NamedVariant::create("varVal", quasar::coretypes::Variant(false), root);

    return root;
  }

  void verifyHierarchy(const std::shared_ptr<NamedObject> &obj) {
    // Assertion: Check if object is not null
    std::cout << "Assertion: Check if restored object is not nullptr"
              << std::endl;
    ASSERT_TRUE(obj != nullptr);

    // Assertion: Check root name
    std::cout << "Assertion: Check if name is \"root\"" << std::endl;
    EXPECT_EQ(obj->getName(), "root");

    // Assertion: Check children size
    std::cout << "Assertion: Check if children size is 13" << std::endl;
    std::list<std::shared_ptr<NamedObject>> children = obj->getChildren();
    EXPECT_EQ(children.size(), 13);

    // Step: Verify each child by name and value
    std::cout << "Step: Verify each child by name and value" << std::endl;
    bool foundInt = false;
    bool foundBool = false;
    bool foundString = false;
    bool foundFloat = false;
    bool foundBuffer = false;
    bool foundBitBuffer = false;
    bool foundDate = false;
    bool foundTs = false;
    bool foundDur = false;
    bool foundQty = false;

    for (std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
      const std::shared_ptr<NamedObject> &child = *it;
      if (child->getName() == "intVal") {
        foundInt = true;
        std::shared_ptr<NamedInteger<int32_t>> i =
            std::dynamic_pointer_cast<NamedInteger<int32_t>>(child);
        if (!i) {
          // Maybe deserialized as int64?
          std::shared_ptr<NamedInteger<int64_t>> i64 =
              std::dynamic_pointer_cast<NamedInteger<int64_t>>(child);
          std::cout << "Assertion: Check intVal (as int64)" << std::endl;
          ASSERT_TRUE(i64 != nullptr);
          EXPECT_EQ(i64->toLong(), 42);
        } else {
          std::cout << "Assertion: Check intVal (as int32)" << std::endl;
          EXPECT_EQ(i->toInt(), 42);
        }
      } else if (child->getName() == "boolVal") {
        foundBool = true;
        std::shared_ptr<NamedBoolean> b =
            std::dynamic_pointer_cast<NamedBoolean>(child);
        std::cout << "Assertion: Check boolVal" << std::endl;
        ASSERT_TRUE(b != nullptr);
        EXPECT_TRUE(b->booleanValue());
      } else if (child->getName() == "stringVal") {
        foundString = true;
        std::shared_ptr<NamedString> s =
            std::dynamic_pointer_cast<NamedString>(child);
        std::cout << "Assertion: Check stringVal" << std::endl;
        ASSERT_TRUE(s != nullptr);
        EXPECT_EQ(s->toString(), "test");
      } else if (child->getName() == "floatVal") {
        foundFloat = true;
        std::shared_ptr<NamedFloatingPoint<double>> f =
            std::dynamic_pointer_cast<NamedFloatingPoint<double>>(child);
        std::cout << "Assertion: Check floatVal" << std::endl;
        ASSERT_TRUE(f != nullptr);
        EXPECT_DOUBLE_EQ(f->toDouble(), 3.14);
      } else if (child->getName() == "bufferVal") {
        foundBuffer = true;
        std::shared_ptr<NamedBuffer> b =
            std::dynamic_pointer_cast<NamedBuffer>(child);
        std::cout << "Assertion: Check bufferVal" << std::endl;
        ASSERT_TRUE(b != nullptr);
        EXPECT_EQ(b->size(), 2);
        EXPECT_EQ(b->get(0), 0xAA);
      } else if (child->getName() == "bitBufferVal") {
        foundBitBuffer = true;
        std::shared_ptr<NamedBitBuffer> bb =
            std::dynamic_pointer_cast<NamedBitBuffer>(child);
        std::cout << "Assertion: Check bitBufferVal" << std::endl;
        ASSERT_TRUE(bb != nullptr);
        EXPECT_EQ(bb->bitSize(), 16);
        EXPECT_TRUE(bb->getBit(0));
      } else if (child->getName() == "dateVal") {
        foundDate = true;
        std::shared_ptr<NamedDate> d = std::dynamic_pointer_cast<NamedDate>(child);
        std::cout << "Assertion: Check dateVal" << std::endl;
        ASSERT_TRUE(d != nullptr);
        EXPECT_EQ(d->value(), 19000);
      } else if (child->getName() == "tsVal") {
        foundTs = true;
        std::shared_ptr<NamedTimestamp> ts = std::dynamic_pointer_cast<NamedTimestamp>(child);
        std::cout << "Assertion: Check tsVal" << std::endl;
        ASSERT_TRUE(ts != nullptr);
        EXPECT_EQ(ts->value(), 1680000000000000);
      } else if (child->getName() == "durVal") {
        foundDur = true;
        std::shared_ptr<NamedDuration> dur = std::dynamic_pointer_cast<NamedDuration>(child);
        std::cout << "Assertion: Check durVal" << std::endl;
        ASSERT_TRUE(dur != nullptr);
        EXPECT_EQ(dur->value(), 42000000);
      } else if (child->getName() == "qtyVal") {
        foundQty = true;
        std::shared_ptr<NamedQuantity> qty = std::dynamic_pointer_cast<NamedQuantity>(child);
        std::cout << "Assertion: Check qtyVal" << std::endl;
        ASSERT_TRUE(qty != nullptr);
        EXPECT_DOUBLE_EQ(qty->value(), 220.5);
        EXPECT_DOUBLE_EQ(qty->value(), 220.5);
        EXPECT_EQ(qty->getUnitSymbol(), "V");
      } else if (child->getName() == "arrayVal") {
        std::shared_ptr<NamedArray<NamedObject>> arr = std::dynamic_pointer_cast<NamedArray<NamedObject>>(child);
        ASSERT_TRUE(arr != nullptr);
        EXPECT_EQ(arr->size(), 1);
      } else if (child->getName() == "mapVal") {
        std::shared_ptr<NamedMap<NamedObject>> m = std::dynamic_pointer_cast<NamedMap<NamedObject>>(child);
        ASSERT_TRUE(m != nullptr);
        EXPECT_EQ(m->size(), 1);
        EXPECT_TRUE(m->contains("key1"));
      } else if (child->getName() == "varVal") {
        std::shared_ptr<NamedVariant> v = std::dynamic_pointer_cast<NamedVariant>(child);
        ASSERT_TRUE(v != nullptr);
        EXPECT_EQ(v->getVariant().getAs<bool>(), false);
      }
    }

    // Assertion: Ensure all expected children were found
    std::cout << "Assertion: Ensure all 13 expected children were found"
              << std::endl;
    EXPECT_TRUE(foundInt);
    EXPECT_TRUE(foundBool);
    EXPECT_TRUE(foundString);
    EXPECT_TRUE(foundFloat);
    EXPECT_TRUE(foundBuffer);
    EXPECT_TRUE(foundBitBuffer);
    EXPECT_TRUE(foundDate);
    EXPECT_TRUE(foundTs);
    EXPECT_TRUE(foundDur);
    EXPECT_TRUE(foundQty);
  }
};

TEST_F(SerializationTest, XMLRoundTrip) {
  // Proof of compliance: [FE-0020.9.4] XML conversion.
  // Step: XML Round Trip
  std::cout << "Step: XML Round Trip" << std::endl;
  std::shared_ptr<NamedObject> root = createTestHierarchy();
  std::string xml = serialization::toXml(root);
  std::shared_ptr<NamedObject> restored = serialization::fromXml(xml);
  verifyHierarchy(restored);
}

TEST_F(SerializationTest, YAMLRoundTrip) {
  // Proof of compliance: [FE-0020.9.3] YAML conversion.
  // Step: YAML Round Trip
  std::cout << "Step: YAML Round Trip" << std::endl;
  std::shared_ptr<NamedObject> root = createTestHierarchy();
  std::string yaml = serialization::toYaml(root);
  std::shared_ptr<NamedObject> restored = serialization::fromYaml(yaml);
  verifyHierarchy(restored);
}

TEST_F(SerializationTest, JSONRoundTrip) {
  // Proof of compliance: [FE-0020.9.2] JSON conversion.
  // Step: JSON Round Trip
  std::cout << "Step: JSON Round Trip" << std::endl;
  std::shared_ptr<NamedObject> root = createTestHierarchy();
  std::string json = serialization::toJson(root);
  std::shared_ptr<NamedObject> restored = serialization::fromJson(json);
  verifyHierarchy(restored);
}

TEST_F(SerializationTest, BinaryRoundTrip) {
  // Proof of compliance: [FE-0020.9.2] BSON conversion.
  // Proof of compliance: [TSK-20260311-004.2.1] efficient binary serialization.
  // Step: Binary Round Trip (BSON)
  std::cout << "Step: Binary Round Trip (BSON)" << std::endl;
  std::shared_ptr<NamedObject> root = createTestHierarchy();
  std::vector<uint8_t> binary = serialization::toBinary(root);
  std::shared_ptr<NamedObject> restored = serialization::fromBinary(binary);
  verifyHierarchy(restored);
}

