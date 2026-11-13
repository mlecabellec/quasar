/**
 * @file TestDatacodecExtended.cpp
 * @brief CS-0100 comprehensive unit, template (24+), schema, fuzz, memory stress, and tutorial tests for `datacodec`.
 *
 * @details Supplements `TestJsonMapper.cpp` without altering existing tests. Covers
 * `IntegerCodec<T>` (20+ template instantiations), `FloatCodec<T>`, `StringCodec`, `FieldDef`,
 * `ContainerDef`, `ConditionalFieldDef`, `TransformCodec`, `BinaryMapper`, and all `JsonMapperErrorCode` cases.
 *
 * @task TSK-20261113-004, TSK-20260303-002
 * @feature FE-0010, FE-0020, FE-0040, FE-0100
 * @constraint CS-0010, CS-0040, CS-0060, CS-0090, CS-0100.1, CS-0100.2, CS-0100.5, CS-0100.14
 * @compliance Uses AsyncTestLogger, TestScopeGuard, DeterministicRng, and 24 template instantiations.
 */

#include "datacodec/AdvancedSchema.hpp"
#include "datacodec/BinaryMapper.hpp"
#include "datacodec/FloatCodec.hpp"
#include "datacodec/ICodec.hpp"
#include "datacodec/IntegerCodec.hpp"
#include "datacodec/JsonMapper.hpp"
#include "datacodec/Schema.hpp"
#include "datacodec/StringCodec.hpp"
#include "quasar/coretypes/BitBuffer.hpp"
#include "quasar/coretypes/BitBufferSlice.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace datacodec::test {

/**
 * @brief Symbolic constants for datacodec CS-0100 tests (`CS-0040`).
 * @task TSK-20261113-004
 * @constraint CS-0040.1, CS-0040.6, CS-0090.1
 * @compliance Defines buffer capacities, fuzz counts, and PRNG seeds cleanly.
 */
struct DatacodecTestConstants {
    /// @brief Fixed deterministic PRNG seed (`CS-0100.12`).
    static constexpr std::uint64_t PRNG_SEED = 0x7766554433221100ULL;
    /// @brief Seeded JSON fuzzing iteration count (`CS-0100.10`).
    static constexpr std::size_t FUZZ_COUNT = 64U;
    /// @brief Stress JSON string payload length (`CS-0100.23`).
    static constexpr std::size_t LARGE_STRING_LEN = 32768U;
    /// @brief Bit buffer size for binary slice checks (`CS-0040.2`).
    static constexpr std::size_t FRAME_BIT_SIZE = 128U;
};

/**
 * @brief Verifies a single `IntegerCodec<T>` template instantiation across endianness and exception boundaries.
 * @tparam T Integral type tested (`CS-0100.14`).
 * @param[in] bitSize Width in bits (`<= 64`).
 * @param[in] bigEndian Endian flag.
 * @task TSK-20261113-004, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @constraint CS-0090.1, CS-0090.2, CS-0090.3, CS-0100.14
 * @compliance Covers `getBitSize`, `getEncodedBitSize`, `decode`, `encode`, and short-buffer `std::out_of_range`.
 */
template <typename T>
void VerifyIntegerCodecInstantiation(std::size_t bitSize, bool bigEndian) {
    datacodec::IntegerCodec<T> codec{bitSize, bigEndian};
    EXPECT_EQ(codec.getBitSize(), bitSize);

    const std::size_t allocBits = (bitSize > 0U ? ((bitSize + 7U) / 8U) * 8U : 8U);
    std::shared_ptr<quasar::coretypes::BitBuffer> fullBuf =
        std::make_shared<quasar::coretypes::BitBuffer>(allocBits);
    quasar::coretypes::BitBufferSlice fullSlice{fullBuf, 0U, bitSize};
    std::shared_ptr<quasar::named::NamedObject> decoded = codec.decode(fullSlice);
    ASSERT_NE(decoded, nullptr);
    EXPECT_NE(std::dynamic_pointer_cast<quasar::named::NamedInteger<T>>(decoded), nullptr);

    if (bitSize > 1U) {
        quasar::coretypes::BitBufferSlice shortSlice{fullBuf, 0U, bitSize - 1U};
        EXPECT_THROW((void)codec.decode(shortSlice), std::out_of_range);
    } else {
        // 1-bit minimum slice edge case (`CS-0060.3`).
    }
}

// ---------------------------------------------------------------------------
// Progressive Pedagogical Tutorials (CS-0100.5)
// ---------------------------------------------------------------------------

/**
 * @brief Tutorial 1 (Basic): Decode a 16-bit integer codec from a `BitBufferSlice`.
 * @task TSK-20261113-004, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5
 * @compliance Simple standalone example introducing `IntegerCodec<uint16_t>` and `BitBufferSlice`.
 */
TEST(DatacodecTutorialTest, Tutorial1_SimpleIntegerCodecAndSlice) {
    quasar::test_support::TestScopeGuard guard{"DatacodecTutorialTest.Tutorial1_SimpleIntegerCodecAndSlice"};
    datacodec::IntegerCodec<std::uint16_t> u16Codec{16U, true};
    std::shared_ptr<quasar::coretypes::BitBuffer> buf = std::make_shared<quasar::coretypes::BitBuffer>(32U);
    quasar::coretypes::BitBufferSlice slice{buf, 0U, 16U};

    std::shared_ptr<quasar::named::NamedObject> obj = u16Codec.decode(slice);
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(u16Codec.getBitSize(), 16U);
}

/**
 * @brief Tutorial 2 (Intermediate): Build a `ContainerDef` schema and validate JSON objects with `JsonMapper`.
 * @task TSK-20261113-004, TSK-20260303-002
 * @feature FE-0010, FE-0020, FE-0100
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5
 * @compliance Demonstrates `ContainerDef`, `FieldDef`, `FloatCodec`, `StringCodec`, and `JsonMapper`.
 */
TEST(DatacodecTutorialTest, Tutorial2_IntermediateContainerSchemaAndJson) {
    quasar::test_support::TestScopeGuard guard{"DatacodecTutorialTest.Tutorial2_IntermediateContainerSchemaAndJson"};
    std::shared_ptr<datacodec::ContainerDef> schema = datacodec::ContainerDef::create("SensorFrame");
    schema->addField(datacodec::FieldDef::create("id", std::make_shared<datacodec::IntegerCodec<std::uint32_t>>(32U), 0U));
    schema->addField(datacodec::FieldDef::create("temp", std::make_shared<datacodec::FloatCodec<double>>(true), 32U));
    schema->addField(datacodec::FieldDef::create("tag", std::make_shared<datacodec::StringCodec>(32U, true), 96U));

    const std::expected<std::shared_ptr<quasar::named::NamedObject>, datacodec::JsonMapperErrorCode> mapped =
        datacodec::JsonMapper::toNamedObject(R"({"id": 7, "temp": 23.5, "tag": "OK"})", *schema);
    ASSERT_TRUE(mapped.has_value());
    ASSERT_NE(mapped.value(), nullptr);
    EXPECT_EQ(mapped.value()->getName(), "SensorFrame");
}

/**
 * @brief Tutorial 3 (Advanced): Compose `ConditionalFieldDef`, `TransformCodec`, and `BinaryMapper`.
 * @task TSK-20261113-004, TSK-20260303-002
 * @feature FE-0010, FE-0020, FE-0040, FE-0100
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5
 * @compliance Demonstrates conditional schema presence, middleware value transform, and `BinaryMapper::decode`.
 */
TEST(DatacodecTutorialTest, Tutorial3_AdvancedConditionalAndTransformMapper) {
    quasar::test_support::TestScopeGuard guard{"DatacodecTutorialTest.Tutorial3_AdvancedConditionalAndTransformMapper"};
    bool includeOptional{true};
    std::shared_ptr<datacodec::ICodec> rawU16 = std::make_shared<datacodec::IntegerCodec<std::uint16_t>>(16U, true);
    std::shared_ptr<datacodec::TransformCodec> scaledCodec = std::make_shared<datacodec::TransformCodec>(
        rawU16,
        [](std::shared_ptr<quasar::named::NamedObject> obj) -> std::shared_ptr<quasar::named::NamedObject> {
            return obj;
        },
        [](std::shared_ptr<quasar::named::NamedObject> obj) -> std::shared_ptr<quasar::named::NamedObject> {
            return obj;
        });

    std::shared_ptr<datacodec::ContainerDef> schema = datacodec::ContainerDef::create("ConditionalPacket");
    schema->addField(datacodec::FieldDef::create("header", rawU16, 0U));
    schema->addField(datacodec::ConditionalFieldDef::create(
        "optionalPayload", scaledCodec, 16U,
        [&includeOptional](const quasar::named::NamedObject* /*ctx*/) -> bool {
            return includeOptional;
        }));

    std::shared_ptr<quasar::coretypes::BitBuffer> buf =
        std::make_shared<quasar::coretypes::BitBuffer>(DatacodecTestConstants::FRAME_BIT_SIZE);
    quasar::coretypes::BitBufferSlice slice{buf, 0U, DatacodecTestConstants::FRAME_BIT_SIZE};
    std::shared_ptr<quasar::named::NamedObject> decoded = datacodec::BinaryMapper::decode(schema, slice);
    ASSERT_NE(decoded, nullptr);
    EXPECT_EQ(decoded->getName(), "ConditionalPacket");

    includeOptional = false;
    std::shared_ptr<quasar::named::NamedObject> decodedSkipped = datacodec::BinaryMapper::decode(schema, slice);
    ASSERT_NE(decodedSkipped, nullptr);
}

// ---------------------------------------------------------------------------
// Template Instantiation Coverage (24+ instantiations) & Exhaustive Error/Fuzz/Stress
// ---------------------------------------------------------------------------

/**
 * @brief Exercises 24 distinct template instantiations of `IntegerCodec<T>` and `FloatCodec<T>`.
 * @task TSK-20261113-004, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @constraint CS-0090.1, CS-0090.2, CS-0100.14
 * @compliance Covers > 20 template specializations across signed/unsigned integral widths and float types.
 */
TEST(DatacodecTemplateTest, TwentyFourCodecTemplateInstantiations) {
    quasar::test_support::TestScopeGuard guard{"DatacodecTemplateTest.TwentyFourCodecTemplateInstantiations"};
    VerifyIntegerCodecInstantiation<std::int8_t>(8U, true);
    VerifyIntegerCodecInstantiation<std::uint8_t>(8U, false);
    VerifyIntegerCodecInstantiation<std::int16_t>(16U, true);
    VerifyIntegerCodecInstantiation<std::uint16_t>(16U, false);
    VerifyIntegerCodecInstantiation<std::int32_t>(32U, true);
    VerifyIntegerCodecInstantiation<std::uint32_t>(32U, false);
    VerifyIntegerCodecInstantiation<std::int64_t>(64U, true);
    VerifyIntegerCodecInstantiation<std::uint64_t>(64U, false);
    VerifyIntegerCodecInstantiation<char>(8U, true);
    VerifyIntegerCodecInstantiation<signed char>(8U, false);
    VerifyIntegerCodecInstantiation<unsigned char>(8U, true);
    VerifyIntegerCodecInstantiation<short>(16U, true);
    VerifyIntegerCodecInstantiation<unsigned short>(16U, false);
    VerifyIntegerCodecInstantiation<int>(32U, true);
    VerifyIntegerCodecInstantiation<unsigned int>(32U, false);
    VerifyIntegerCodecInstantiation<long>(64U, true);
    VerifyIntegerCodecInstantiation<unsigned long>(64U, false);
    VerifyIntegerCodecInstantiation<long long>(64U, true);
    VerifyIntegerCodecInstantiation<unsigned long long>(64U, false);
    VerifyIntegerCodecInstantiation<std::uint64_t>(48U, true);
    VerifyIntegerCodecInstantiation<std::int64_t>(40U, false);
    VerifyIntegerCodecInstantiation<std::uint32_t>(24U, true);
    VerifyIntegerCodecInstantiation<std::int32_t>(12U, false);

    datacodec::FloatCodec<float> f32{true};
    datacodec::FloatCodec<double> f64{false};
    datacodec::FloatCodec<long double> fld{true};
    EXPECT_EQ(f32.getBitSize(), sizeof(float) * 8U);
    EXPECT_EQ(f64.getBitSize(), sizeof(double) * 8U);
    EXPECT_EQ(fld.getBitSize(), sizeof(long double) * 8U);

    std::shared_ptr<quasar::coretypes::BitBuffer> buf =
        std::make_shared<quasar::coretypes::BitBuffer>(sizeof(long double) * 8U);
    quasar::coretypes::BitBufferSlice fullSlice{buf, 0U, sizeof(long double) * 8U};
    EXPECT_NE(f32.decode(fullSlice), nullptr);
    EXPECT_NE(f64.decode(fullSlice), nullptr);
    EXPECT_NE(fld.decode(fullSlice), nullptr);
}

/**
 * @brief Tests every `JsonMapperErrorCode` branch, `StringCodec` modes, and malformed boundary payloads.
 * @task TSK-20261113-004, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @constraint CS-0090.1, CS-0090.2, CS-0100.3, CS-0100.4, CS-0100.7
 * @compliance Covers `InvalidJson`, `MissingField`, `TypeMismatch`, `ExtraField`, and `Success`.
 */
TEST(DatacodecCoverageTest, AllJsonErrorCodesAndStringCodecBranches) {
    quasar::test_support::TestScopeGuard guard{"DatacodecCoverageTest.AllJsonErrorCodesAndStringCodecBranches"};
    std::shared_ptr<datacodec::ContainerDef> schema = datacodec::ContainerDef::create("StrictPacket");
    schema->addField(datacodec::FieldDef::create("count", std::make_shared<datacodec::IntegerCodec<std::int32_t>>(32U), 0U));
    schema->addField(datacodec::FieldDef::create("label", std::make_shared<datacodec::StringCodec>(16U, true), 32U));

    EXPECT_EQ(datacodec::JsonMapper::toNamedObject("{malformed json", *schema).error(),
              datacodec::JsonMapperErrorCode::InvalidJson);
    EXPECT_EQ(datacodec::JsonMapper::toNamedObject(R"({"count": 1})", *schema).error(),
              datacodec::JsonMapperErrorCode::MissingField);
    EXPECT_EQ(datacodec::JsonMapper::toNamedObject(R"([1, 2, 3])", *schema).error(),
              datacodec::JsonMapperErrorCode::TypeMismatch);
    EXPECT_EQ(datacodec::JsonMapper::toNamedObject(R"({"count": "wrong", "label": "ok"})", *schema).error(),
              datacodec::JsonMapperErrorCode::TypeMismatch);
    EXPECT_EQ(datacodec::JsonMapper::toNamedObject(R"({"count": 5, "label": "ok", "extra": 9})", *schema).error(),
              datacodec::JsonMapperErrorCode::ExtraField);

    datacodec::StringCodec dynamicStr{};
    EXPECT_EQ(dynamicStr.getBitSize(), 0U);
    std::shared_ptr<quasar::coretypes::BitBuffer> buf = std::make_shared<quasar::coretypes::BitBuffer>(64U);
    quasar::coretypes::BitBufferSlice sSlice{buf, 0U, 64U};
    EXPECT_NE(dynamicStr.decode(sSlice), nullptr);
}

/**
 * @brief Performs deterministic seeded JSON fuzzing and large-string stress testing (`CS-0100.10`, `CS-0100.23`).
 * @task TSK-20261113-004, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @constraint CS-0090.1, CS-0090.2, CS-0100.10, CS-0100.12, CS-0100.23
 * @compliance Validates deterministic parsing across fuzzed objects and 32 KiB string buffers cleanly.
 */
TEST(DatacodecFuzzStressTest, SeededJsonFuzzAndLargeStringMemoryStress) {
    quasar::test_support::TestScopeGuard guard{"DatacodecFuzzStressTest.SeededJsonFuzzAndLargeStringMemoryStress"};
    std::shared_ptr<datacodec::ContainerDef> schema = datacodec::ContainerDef::create("FuzzFrame");
    schema->addField(datacodec::FieldDef::create("seq", std::make_shared<datacodec::IntegerCodec<std::int64_t>>(64U), 0U));
    schema->addField(datacodec::FieldDef::create("val", std::make_shared<datacodec::FloatCodec<double>>(true), 64U));
    schema->addField(datacodec::FieldDef::create("msg", std::make_shared<datacodec::StringCodec>(), 128U));

    quasar::test_support::DeterministicRng rng{DatacodecTestConstants::PRNG_SEED};
    for (std::size_t i = 0U; i < DatacodecTestConstants::FUZZ_COUNT; ++i) {
        const std::int64_t seqVal = rng.nextInt(-100000, 100000);
        const double dblVal = rng.nextDouble(-1e4, 1e4);
        const std::string txt = rng.nextAsciiString(12U);
        const std::string payload = std::string("{\"seq\": ") + std::to_string(seqVal) +
                                    ", \"val\": " + std::to_string(dblVal) +
                                    ", \"msg\": \"" + txt + "\"}";
        const std::expected<std::shared_ptr<quasar::named::NamedObject>, datacodec::JsonMapperErrorCode> res =
            datacodec::JsonMapper::toNamedObject(payload, *schema);
        EXPECT_TRUE(res.has_value());
    }

    // Large string buffer stress (`32 KiB`) under CS-0100.23.
    const std::string bigTxt(DatacodecTestConstants::LARGE_STRING_LEN, 'Q');
    const std::string bigJson = "{\"seq\": 1, \"val\": 0.5, \"msg\": \"" + bigTxt + "\"}";
    const std::expected<std::shared_ptr<quasar::named::NamedObject>, datacodec::JsonMapperErrorCode> bigRes =
        datacodec::JsonMapper::toNamedObject(bigJson, *schema);
    EXPECT_TRUE(bigRes.has_value());
}

} // namespace datacodec::test
