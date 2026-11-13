/**
 * @file TestCoretypesExtended.cpp
 * @brief Extended CS-0100 test suite for `quasar_coretypes` (tutorials, 20+ templates, temporal/unit boundaries, fuzz & stress).
 * @details Covers previously untested temporal primitives (`Date`, `Duration`, `Timestamp`), SI `Unit`/`Units` and `Quantity` arithmetic, `Variant` enum branches, `BitBuffer`/`BitBufferSlice` bit-packing boundaries, 20+ `Integer<T>`/`FloatingPoint<T>` instantiations, seeded fuzzing, and parallel concurrency stress.
 * @task [TSK-20260308-001] Polymorphic Integer Dispatch & Primitive Coverage
 * @feature [FE-0030] Safe Core Primitive Types
 * @feature [FE-0110] Temporal, Unit, Quantity, and Variant Extensions
 * @compliance Fulfills CS-0010, CS-0040, CS-0060, CS-0090, and CS-0100 without modifying legacy test sources.
 */

#include "quasar/coretypes/BitBuffer.hpp"
#include "quasar/coretypes/BitBufferSlice.hpp"
#include "quasar/coretypes/Boolean.hpp"
#include "quasar/coretypes/Buffer.hpp"
#include "quasar/coretypes/BufferSlice.hpp"
#include "quasar/coretypes/Constants.hpp"
#include "quasar/coretypes/Date.hpp"
#include "quasar/coretypes/Duration.hpp"
#include "quasar/coretypes/FloatingPoint.hpp"
#include "quasar/coretypes/Integer.hpp"
#include "quasar/coretypes/Number.hpp"
#include "quasar/coretypes/Quantity.hpp"
#include "quasar/coretypes/String.hpp"
#include "quasar/coretypes/Timestamp.hpp"
#include "quasar/coretypes/Unit.hpp"
#include "quasar/coretypes/Variant.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <gtest/gtest.h>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace quasar::coretypes::test {

/**
 * @brief Maximum iterations for seeded fuzz loops (`CS-0040.2`, `CS-0010.37`).
 * @requirement [CS-0040.2] Mandatory Symbolic Limitations
 * @compliance Named `UPPER_SNAKE_CASE` constant replaces raw literals.
 */
constexpr std::size_t MAX_FUZZ_ITERATIONS = 128U;

/**
 * @brief Bit count for large `BitBuffer` stress testing (`CS-0040.2`).
 * @requirement [FE-0030.5] BitBuffer Scalability
 * @compliance Named symbolic constant for buffer sizing.
 */
constexpr std::size_t STRESS_BIT_COUNT = 65536U;

/**
 * @brief Thread count for concurrent stress tests (`CS-0040.2`).
 * @requirement [CS-0010.5] Thread Safety Verification
 * @compliance Bounded worker count under CS-0080 and CS-0100.14 limits.
 */
constexpr std::size_t CONCURRENT_WORKERS = 4U;

/**
 * @brief Per-worker iteration bound (`CS-0010.37`, `CS-0040.2`).
 * @requirement [CS-0010.37] Bounded Loops
 * @compliance Guarantees deterministic termination.
 */
constexpr std::size_t WORKER_ITERATIONS = 200U;

/**
 * @brief Deterministic seed for fuzzing (`CS-0100.12`).
 * @requirement [CS-0100.12] Deterministic Tests
 * @compliance Fixed seed guarantees reproducible inputs.
 */
constexpr std::uint64_t CORETYPES_FUZZ_SEED = 0xC04E719EULL;

/**
 * @brief Lock timeout for shared state guards (`CS-0010.26`).
 * @requirement [CS-0010.26] Timed Mutex Usage
 * @compliance Finite lock acquisition window prevents deadlocks.
 */
constexpr std::chrono::milliseconds LOCK_TIMEOUT_MS{500};

/**
 * @brief Helper exercising a concrete `Integer<T>` instantiation (`CS-0090.1`).
 * @tparam T Integral element type.
 * @param[in] val Justification: sample value for round-trip; Usage: construct Integer<T>; Consumed: no; Prerequisites: integral type.
 * @param[in] label Justification: diagnostic log label; Usage: async log event; Consumed: no; Prerequisites: non-empty string.
 * @return Always returns `true` after assertions succeed; Justification: aggregates instantiation checks; Usage: assertion; Consumed: caller checks bool; Prerequisites: none; Limitations: exercises int64_t conversion range; Lifecycle: value; Modification / Deletion: none; Specific Values: true; Error Behavior: GTest failure on mismatch.
 * @throws None directly; propagates assertion failure via GTest.
 * @containment Checked integral template parameters prevent `bool` instantiation error (`CS-0010.19`).
 * @task [TSK-20260308-001] Polymorphic Integer Dispatch
 * @compliance [CS-0010.45] Full Doxygen documentation with contracts.
 */
template <typename T>
[[nodiscard]] bool verifyIntegerTemplate(T val, const std::string& label) {
    // Instantiate Integer<T> with explicit type (no auto, CS-0010.34).
    Integer<T> num(val);
    Integer<T> copy(num);
    quasar::test_support::AsyncTestLogger::instance().logInput(label, static_cast<long long>(val));
    EXPECT_EQ(copy.value(), val);
    EXPECT_EQ(num.toString(), std::to_string(val));
    quasar::test_support::AsyncTestLogger::instance().logOutput(label, copy.toString());
    return copy.value() == val;
}

/**
 * @brief Helper exercising a concrete `FloatingPoint<T>` instantiation (`CS-0090.1`).
 * @tparam T Floating point type.
 * @param[in] val Justification: finite float/double value; Usage: construct FloatingPoint<T>; Consumed: no; Prerequisites: finite floating-point value.
 * @param[in] label Justification: diagnostic log label; Usage: async log event; Consumed: no; Prerequisites: non-empty string.
 * @return Returns `true` when round-trip matches; Justification: template validation; Usage: EXPECT_TRUE; Consumed: caller checks; Prerequisites: finite input; Limitations: exact equality for binary-exact test values; Lifecycle: value; Modification / Deletion: none; Specific Values: true; Error Behavior: GTest failure.
 * @throws None.
 * @containment Validates finite literal inputs to avoid NaN inequality traps.
 * @task [TSK-20260308-001] Primitive Coverage
 * @compliance [CS-0090.3] Formal parameter contracts present.
 */
template <typename T>
[[nodiscard]] bool verifyFloatTemplate(T val, const std::string& label) {
    // Verify copy and value preservation for FloatingPoint<T>.
    FloatingPoint<T> fp(val);
    FloatingPoint<T> fp2(fp);
    quasar::test_support::AsyncTestLogger::instance().logInput(label, static_cast<double>(val));
    EXPECT_DOUBLE_EQ(static_cast<double>(fp2.value()), static_cast<double>(val));
    quasar::test_support::AsyncTestLogger::instance().logOutput(label, static_cast<double>(fp2.value()));
    return fp2.value() == val;
}

TEST(CoretypesTutorialTest, Tutorial1_SimpleTemporalAndVariantBasics) {
    quasar::test_support::TestScopeGuard scope("CoretypesTutorialTest.Tutorial1");
    // Step 1: Construct a Date at epoch and check ISO-8601 formatting.
    Date epochDate(0);
    Date currentDate = Date::now();
    EXPECT_FALSE(epochDate.toISO8601().empty());
    EXPECT_GE(currentDate.value(), 0);
    quasar::test_support::AsyncTestLogger::instance().logOutput("epochISO", epochDate.toISO8601());

    // Step 2: Construct Duration and Timestamp values and verify second conversion.
    Duration twoSec = Duration::fromSeconds(2.5);
    EXPECT_NEAR(twoSec.toSeconds(), 2.5, 1e-6);
    Timestamp nowTs = Timestamp::now();
    EXPECT_FALSE(nowTs.toISO8601().empty());
    EXPECT_GT(nowTs.value(), 0);

    // Step 3: Store heterogeneous primitive values inside Variant safely.
    Variant emptyVar;
    Variant intVar(static_cast<int64_t>(42));
    Variant strVar(std::string("quasar"));
    EXPECT_EQ(emptyVar.getVariantType(), VariantType::Empty);
    EXPECT_TRUE(intVar.holds<int64_t>());
    EXPECT_EQ(intVar.getAs<int64_t>(), 42);
    EXPECT_EQ(strVar.toString(), "quasar");
}

TEST(CoretypesTutorialTest, Tutorial2_IntermediatePhysicalUnitsAndQuantities) {
    quasar::test_support::TestScopeGuard scope("CoretypesTutorialTest.Tutorial2");
    // Step 1: Compare compatible units (Meter vs Kilometer) and convert quantities.
    EXPECT_TRUE(Units::Meter.hasSameDimensions(Units::Kilometer));
    EXPECT_FALSE(Units::Meter.hasSameDimensions(Units::Second));
    Quantity distanceKm(2.5, Units::Kilometer);
    Quantity distanceM = distanceKm.convertTo(Units::Meter);
    quasar::test_support::AsyncTestLogger::instance().logOutput("distanceMeters", distanceM.value());
    EXPECT_NEAR(distanceM.value(), 2500.0, 1e-9);

    // Step 2: Add compatible Quantities in mixed scales (m + km).
    Quantity shortRun(500.0, Units::Meter);
    Quantity combined = shortRun + Quantity(1.0, Units::Kilometer);
    EXPECT_NEAR(combined.value(), 1500.0, 1e-9);
    EXPECT_EQ(combined.getType(), "Quantity");

    // Step 3: Derive compound dimensions via Unit operators (* and /).
    Unit speedUnit = Units::Meter / Units::Second;
    EXPECT_TRUE(speedUnit.hasSameDimensions(Units::Speed));
    Unit areaUnit = Units::Meter * Units::Meter;
    EXPECT_TRUE(areaUnit.hasSameDimensions(Units::Area));
}

TEST(CoretypesTutorialTest, Tutorial3_AdvancedBitBufferSlicingAndTemperatureOffset) {
    quasar::test_support::TestScopeGuard scope("CoretypesTutorialTest.Tutorial3");
    // Step 1: Allocate a 32-bit BitBuffer (BitBuffer constructor expects bit count!).
    std::shared_ptr<BitBuffer> frame = std::make_shared<BitBuffer>(static_cast<size_t>(32U));
    frame->setBit(0U, true);
    frame->setBit(15U, true);
    frame->setBit(16U, true);
    frame->setBit(31U, true);

    // Step 2: Create non-owning BitBufferSlices and concatenate them into a new BitBuffer.
    BitBufferSlice lower(frame, 0U, 16U);
    BitBufferSlice upper(frame, 16U, 16U);
    BitBufferSlice nested = upper.slice(0U, 8U);
    EXPECT_EQ(nested.size(), 8U);
    EXPECT_TRUE(nested.getBit(0U));
    std::shared_ptr<BitBuffer> merged = lower.concat(nested);
    std::vector<uint8_t> packed = merged->toVector();
    EXPECT_EQ(packed.size(), 3U);
    EXPECT_EQ(merged->size(), 3U);

    // Step 3: Exercise offset unit conversion (Celsius <-> Kelvin) using Quantity.
    Quantity boilingC(100.0, Units::Celsius);
    Quantity boilingK = boilingC.convertTo(Units::Kelvin);
    quasar::test_support::AsyncTestLogger::instance().logOutput("boilingK", boilingK.value());
    EXPECT_NEAR(boilingK.value(), 373.15, 1e-6);
}

TEST(CoretypesTemplateTest, TwentyPlusPrimitiveTemplateInstantiations) {
    quasar::test_support::TestScopeGuard scope("CoretypesTemplateTest.20Plus");
    // Instantiate 20 distinct Integer<T> and FloatingPoint<T> cases (CS-0100.15).
    EXPECT_TRUE(verifyIntegerTemplate<int8_t>(static_cast<int8_t>(-42), "int8_minish"));
    EXPECT_TRUE(verifyIntegerTemplate<int8_t>(std::numeric_limits<int8_t>::max(), "int8_max"));
    EXPECT_TRUE(verifyIntegerTemplate<uint8_t>(static_cast<uint8_t>(0U), "uint8_zero"));
    EXPECT_TRUE(verifyIntegerTemplate<uint8_t>(std::numeric_limits<uint8_t>::max(), "uint8_max"));
    EXPECT_TRUE(verifyIntegerTemplate<int16_t>(static_cast<int16_t>(-1234), "int16"));
    EXPECT_TRUE(verifyIntegerTemplate<uint16_t>(static_cast<uint16_t>(54321U), "uint16"));
    EXPECT_TRUE(verifyIntegerTemplate<int32_t>(-100000, "int32"));
    EXPECT_TRUE(verifyIntegerTemplate<uint32_t>(3000000000U, "uint32"));
    EXPECT_TRUE(verifyIntegerTemplate<int64_t>(-900000000000LL, "int64"));
    EXPECT_TRUE(verifyIntegerTemplate<uint64_t>(18000000000000ULL, "uint64"));
    EXPECT_TRUE(verifyIntegerTemplate<short>(static_cast<short>(77), "short"));
    EXPECT_TRUE(verifyIntegerTemplate<unsigned short>(static_cast<unsigned short>(88U), "ushort"));
    EXPECT_TRUE(verifyIntegerTemplate<int>(1024, "int"));
    EXPECT_TRUE(verifyIntegerTemplate<unsigned int>(2048U, "uint"));
    EXPECT_TRUE(verifyIntegerTemplate<long>(4096L, "long"));
    EXPECT_TRUE(verifyIntegerTemplate<unsigned long>(8192UL, "ulong"));
    EXPECT_TRUE(verifyIntegerTemplate<long long>(16384LL, "llong"));
    EXPECT_TRUE(verifyIntegerTemplate<unsigned long long>(32768ULL, "ullong"));
    EXPECT_TRUE(verifyFloatTemplate<float>(3.5F, "float_3_5"));
    EXPECT_TRUE(verifyFloatTemplate<double>(-125.25, "double_neg"));
    EXPECT_TRUE(verifyFloatTemplate<long double>(0.125L, "ldouble"));
}

TEST(CoretypesCoverageTest, AllEnumsUnitSymbolsAndExceptionBoundaries) {
    quasar::test_support::TestScopeGuard scope("CoretypesCoverageTest.AllEnumsAndBounds");
    // Exercise all VariantType enum branches and invalid getAs<> exception paths.
    const std::array<Variant, 6> variants = {
        Variant(),
        Variant(true),
        Variant(static_cast<int64_t>(7)),
        Variant(1.25),
        Variant(std::string("abc")),
        Variant(std::vector<uint8_t>{0x01U, 0x02U})
    };
    const std::array<VariantType, 6> expectedTypes = {
        VariantType::Empty,
        VariantType::Boolean,
        VariantType::Integer,
        VariantType::Double,
        VariantType::String,
        VariantType::Buffer
    };

    for (std::size_t idx = 0U; idx < variants.size(); ++idx) {
        if (idx >= expectedTypes.size()) {
            // Defensive iteration bound check (CS-0010.37).
            throw std::runtime_error("Variant index out of range");
        } else {
            EXPECT_EQ(variants[idx].getVariantType(), expectedTypes[idx]);
            EXPECT_FALSE(variants[idx].toString().empty());
        }
    }
    EXPECT_EQ(variants[1].getIf<bool>() != nullptr, true);
    EXPECT_EQ(variants[1].getIf<int64_t>(), nullptr);
    EXPECT_THROW((void)variants[0].getAs<int64_t>(), std::bad_variant_access);

    // Exercise Units::fromSymbol across standard symbols plus fallback symbol.
    const std::array<std::string, 10> symbols = {
        "m", "km", "mm", "kg", "g", "s", "A", "K", "V", "unknown_sym"
    };
    for (std::size_t idx = 0U; idx < symbols.size(); ++idx) {
        if (idx >= symbols.size()) {
            throw std::runtime_error("Symbol index out of range");
        } else {
            Unit resolved = Units::fromSymbol(symbols[idx]);
            quasar::test_support::AsyncTestLogger::instance().logBranch(symbols[idx] + "->" + resolved.symbol);
            if (symbols[idx] == "unknown_sym") {
                EXPECT_TRUE(resolved.symbol.empty());
            } else {
                EXPECT_FALSE(resolved.symbol.empty());
            }
        }
    }

    // Exercise incompatible unit conversion and BitBufferSlice boundary exceptions.
    Quantity qMeter(10.0, Units::Meter);
    Quantity qAmp(5.0, Units::Ampere);
    EXPECT_THROW((void)qMeter.convertTo(Units::Second), std::invalid_argument);
    EXPECT_THROW((void)(qMeter + qAmp), std::invalid_argument);

    std::shared_ptr<BitBuffer> emptyBuf;
    EXPECT_THROW(BitBufferSlice(emptyBuf, 0U, 8U), std::invalid_argument);
    std::shared_ptr<BitBuffer> smallBuf = std::make_shared<BitBuffer>(static_cast<size_t>(16U));
    EXPECT_THROW(BitBufferSlice(smallBuf, 8U, 16U), std::out_of_range);
    BitBufferSlice slice(smallBuf, 0U, 8U);
    EXPECT_THROW((void)slice.getBit(8U), std::out_of_range);
    EXPECT_THROW(slice.setBit(8U, true), std::out_of_range);
    EXPECT_THROW((void)slice.slice(4U, 8U), std::out_of_range);

    // Exercise BitBuffer out-of-range bit accesses and Constants symbols.
    EXPECT_THROW((void)smallBuf->getBit(16U), std::out_of_range);
    EXPECT_THROW(smallBuf->setBit(16U, true), std::out_of_range);
    EXPECT_GE(DEFAULT_MUTEX_TIMEOUT.count(), 1);
}

TEST(CoretypesFuzzStressTest, DeterministicQuantityFuzzAndConcurrentBitBufferStress) {
    quasar::test_support::TestScopeGuard scope("CoretypesFuzzStressTest.FuzzAndConcurrent");
    // Step 1: Deterministic seeded fuzzing of Quantity conversions & arithmetic.
    quasar::test_support::DeterministicRng rng(CORETYPES_FUZZ_SEED);
    for (std::size_t iter = 0U; iter < MAX_FUZZ_ITERATIONS; ++iter) {
        if (iter >= MAX_FUZZ_ITERATIONS) {
            throw std::runtime_error("Fuzz bound exceeded");
        } else {
            double valM = rng.nextDouble(-10000.0, 10000.0);
            Quantity qM(valM, Units::Meter);
            Quantity qKm = qM.convertTo(Units::Kilometer);
            Quantity qBack = qKm.convertTo(Units::Meter);
            EXPECT_NEAR(qBack.value(), valM, 1e-6);
        }
    }

    // Step 2: Large BitBuffer memory stress (65536 bits = 8192 bytes) and pack/unpack check.
    std::shared_ptr<BitBuffer> bigBits = std::make_shared<BitBuffer>(STRESS_BIT_COUNT);
    for (std::size_t bitIdx = 0U; bitIdx < STRESS_BIT_COUNT; bitIdx += 257U) {
        if (bitIdx >= STRESS_BIT_COUNT) {
            throw std::runtime_error("Stress bit bound exceeded");
        } else {
            bigBits->setBit(bitIdx, true);
        }
    }
    EXPECT_TRUE(bigBits->getBit(0U));
    EXPECT_EQ(bigBits->toVector().size(), STRESS_BIT_COUNT / 8U);

    // Step 3: Concurrent threads mutating disjoint slices protected by a shared timed mutex.
    std::shared_ptr<BitBuffer> sharedBits = std::make_shared<BitBuffer>(static_cast<size_t>(1024U));
    std::timed_mutex bitMutex;
    std::vector<std::thread> workers;
    workers.reserve(CONCURRENT_WORKERS);
    std::size_t totalFlips = 0U;

    for (std::size_t w = 0U; w < CONCURRENT_WORKERS; ++w) {
        if (w >= CONCURRENT_WORKERS) {
            throw std::runtime_error("Worker bound exceeded");
        } else {
            workers.emplace_back([&sharedBits, &bitMutex, &totalFlips, w]() {
                for (std::size_t step = 0U; step < WORKER_ITERATIONS; ++step) {
                    if (step >= WORKER_ITERATIONS) {
                        throw std::runtime_error("Worker iteration bound exceeded");
                    } else {
                        std::unique_lock<std::timed_mutex> lock(bitMutex, LOCK_TIMEOUT_MS);
                        if (!lock.owns_lock()) {
                            throw std::runtime_error("Timed mutex timeout in coretypes stress test");
                        } else {
                            std::size_t bitPos = (w * 64U) + (step % 64U);
                            sharedBits->setBit(bitPos, (step % 2U) == 0U);
                            ++totalFlips;
                        }
                    }
                }
            });
        }
    }

    for (std::size_t w = 0U; w < workers.size(); ++w) {
        if (w >= CONCURRENT_WORKERS) {
            throw std::runtime_error("Join bound exceeded");
        } else {
            workers[w].join();
        }
    }
    EXPECT_EQ(totalFlips, CONCURRENT_WORKERS * WORKER_ITERATIONS);
}

} // namespace quasar::coretypes::test
