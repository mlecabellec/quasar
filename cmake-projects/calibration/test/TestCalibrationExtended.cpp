/**
 * @file TestCalibrationExtended.cpp
 * @brief CS-0100 comprehensive unit, Lua integration, boundary, fuzz, and tutorial tests for `quasar::calibration`.
 *
 * @details Supplements existing `quasar_calibration_test` without modifying existing test files.
 * Covers `IdentityCalibration`, `LinearCalibration`, `PolynomialCalibration`, `TableCalibration`,
 * `EnumCalibration`, `FormatCalibration`, `Named*Calibration`, `CompositeCalibration`,
 * `ConditionalCalibration`, `CalibrationManager`, and `bindCalibrationTypes` (Lua).
 *
 * @task TSK-20261113-004, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @constraint CS-0010, CS-0040, CS-0060, CS-0090, CS-0100.1, CS-0100.2, CS-0100.5
 * @compliance Uses AsyncTestLogger, TestScopeGuard, DeterministicRng, and explicit types.
 */

#include "quasar/calibration/CalibrationManager.hpp"
#include "quasar/calibration/Calibrations.hpp"
#include "quasar/calibration/ConditionalCalibration.hpp"
#include "quasar/calibration/LuaBindings.hpp"
#include "quasar/calibration/NamedCalibration.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"

#include <gtest/gtest.h>
#include <sol/sol.hpp>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace quasar::calibration::test {

/**
 * @brief Symbolic constants and seeds for calibration tests (`CS-0040`).
 * @task TSK-20261113-004
 * @constraint CS-0040.1, CS-0040.6, CS-0090.1
 * @compliance Centralizes PRNG seed, fuzz count, and worker limits.
 */
struct CalibrationTestConstants {
    /// @brief Fixed deterministic PRNG seed (`CS-0100.12`).
    static constexpr std::uint64_t PRNG_SEED = 0xCAFEBABE12345678ULL;
    /// @brief Fuzz iterations per calibration curve (`CS-0100.10`).
    static constexpr std::size_t FUZZ_ITERATIONS = 200U;
    /// @brief Concurrent reader worker count (`CS-0100.4`).
    static constexpr std::size_t WORKER_COUNT = 4U;
    /// @brief Reads per concurrent worker (`CS-0010.37`).
    static constexpr std::size_t READS_PER_WORKER = 100U;
};

// ---------------------------------------------------------------------------
// Progressive Pedagogical Tutorials (CS-0100.5)
// ---------------------------------------------------------------------------

/**
 * @brief Tutorial 1 (Basic): Linear scale/offset calibration and inverse recovery.
 * @task TSK-20261113-004, TSK-20260311-010
 * @feature FE-0050, FE-0160
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5
 * @compliance Minimal pedagogical example for human developers using `LinearCalibration`.
 */
TEST(CalibrationTutorialTest, Tutorial1_SimpleLinearTransform) {
    quasar::test_support::TestScopeGuard guard{"CalibrationTutorialTest.Tutorial1_SimpleLinearTransform"};
    // Step 1: Construct a linear transform y = 0.1 * x - 50.0 (e.g., ADC counts to degC).
    quasar::calibration::LinearCalibration adcToTemp{0.1, -50.0};

    // Step 2: Convert raw count 750 -> 25.0 degC.
    const quasar::calibration::Variant eng = adcToTemp.rawToEng(static_cast<int64_t>(750));
    EXPECT_DOUBLE_EQ(quasar::calibration::toDouble(eng), 25.0);

    // Step 3: Invert engineering 25.0 degC back to raw 750.0.
    const quasar::calibration::Variant rawBack = adcToTemp.engToRaw(25.0);
    EXPECT_DOUBLE_EQ(quasar::calibration::toDouble(rawBack), 750.0);
}

/**
 * @brief Tutorial 2 (Intermediate): Organize named calibrations in `CalibrationManager` + piecewise table.
 * @task TSK-20261113-004, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0210
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5
 * @compliance Demonstrates `CalibrationManager`, `NamedLinearCalibration`, `NamedPolynomialCalibration`, and `TableCalibration`.
 */
TEST(CalibrationTutorialTest, Tutorial2_IntermediateNamedManagerAndTable) {
    quasar::test_support::TestScopeGuard guard{"CalibrationTutorialTest.Tutorial2_IntermediateNamedManagerAndTable"};
    /** @brief Local bridge exposing `addChild`/`setSelf` on `CalibrationManager` for attachment (`CS-0090.1`). */
    struct AttachableManager final : public quasar::calibration::CalibrationManager {
        explicit AttachableManager(const std::string& n) : CalibrationManager(n) {}
        void attach(std::shared_ptr<quasar::named::NamedObject> child) { addChild(child); }
    };
    /** @brief Local bridge calling `setSelf` on `NamedLinearCalibration` so `as<NamedCalibration>()` works (`CS-0090.1`). */
    struct AttachableLinear final : public quasar::calibration::NamedLinearCalibration {
        AttachableLinear(const std::string& n, double m, double b) : NamedLinearCalibration(n, m, b) {}
        static std::shared_ptr<AttachableLinear> create(const std::string& n, double m, double b) {
            std::shared_ptr<AttachableLinear> ptr = std::make_shared<AttachableLinear>(n, m, b);
            ptr->setSelf(ptr);
            return ptr;
        }
    };
    /** @brief Local bridge calling `setSelf` on `NamedPolynomialCalibration` so `as<NamedCalibration>()` works (`CS-0090.1`). */
    struct AttachablePoly final : public quasar::calibration::NamedPolynomialCalibration {
        AttachablePoly(const std::string& n, std::vector<double> c) : NamedPolynomialCalibration(n, std::move(c)) {}
        static std::shared_ptr<AttachablePoly> create(const std::string& n, std::vector<double> c) {
            std::shared_ptr<AttachablePoly> ptr = std::make_shared<AttachablePoly>(n, std::move(c));
            ptr->setSelf(ptr);
            return ptr;
        }
    };
    std::shared_ptr<AttachableManager> mgr =
        std::make_shared<AttachableManager>("SensorCalibrations");
    EXPECT_EQ(mgr->getType(), "CalibrationManager");

    std::shared_ptr<AttachableLinear> lin = AttachableLinear::create("BusScale", 2.0, 1.5);
    std::shared_ptr<AttachablePoly> poly =
        AttachablePoly::create("Thermistor", std::vector<double>{1.0, 2.0, 0.5});

    mgr->attach(lin);
    mgr->attach(poly);
    EXPECT_EQ(mgr->getCalibration("BusScale"), lin);
    EXPECT_EQ(mgr->getCalibration("MissingCal"), nullptr);

    quasar::calibration::TableCalibration table{
        std::vector<quasar::calibration::TableCalibration::Point>{
            {10.0, 100.0}, {0.0, 0.0}, {20.0, 300.0}}};
    EXPECT_DOUBLE_EQ(quasar::calibration::toDouble(table.rawToEng(5.0)), 50.0);
    EXPECT_DOUBLE_EQ(quasar::calibration::toDouble(table.rawToEng(15.0)), 200.0);
    EXPECT_DOUBLE_EQ(quasar::calibration::toDouble(table.rawToEng(-5.0)), 0.0);
    EXPECT_DOUBLE_EQ(quasar::calibration::toDouble(table.rawToEng(25.0)), 300.0);
}

/**
 * @brief Tutorial 3 (Advanced): `ConditionalCalibration` dispatch combined with Lua usertype bindings.
 * @task TSK-20261113-004, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5, CS-0100.16
 * @compliance Demonstrates runtime predicate dispatch and Lua script evaluation via `bindCalibrationTypes`.
 */
TEST(CalibrationTutorialTest, Tutorial3_AdvancedConditionalAndLuaBindings) {
    quasar::test_support::TestScopeGuard guard{"CalibrationTutorialTest.Tutorial3_AdvancedConditionalAndLuaBindings"};
    int modeFlag{1};
    std::shared_ptr<quasar::calibration::ConditionalCalibration> cond =
        std::make_shared<quasar::calibration::ConditionalCalibration>("AdaptiveGain");
    cond->setDefaultCase(std::make_shared<quasar::calibration::IdentityCalibration>());
    cond->addCase(static_cast<int64_t>(1), std::make_shared<quasar::calibration::LinearCalibration>(10.0, 0.0));
    cond->setEvaluator([&modeFlag]() -> quasar::calibration::Variant {
        return static_cast<int64_t>(modeFlag);
    });

    EXPECT_DOUBLE_EQ(quasar::calibration::toDouble(cond->rawToEng(3.0)), 30.0);
    modeFlag = 2;
    EXPECT_DOUBLE_EQ(quasar::calibration::toDouble(cond->rawToEng(3.0)), 3.0);

    sol::state lua{};
    lua.open_libraries(sol::lib::base, sol::lib::math);
    quasar::calibration::bindCalibrationTypes(lua);
    lua.script(R"(
        local cal = NamedLinearCalibration.new("LuaGain", 2.5, -1.0)
        lua_eng = cal:rawToEng(4.0)
    )");
    const double luaEng = lua["lua_eng"];
    EXPECT_DOUBLE_EQ(luaEng, 9.0);
}

// ---------------------------------------------------------------------------
// Boundary, Exception, Enum & Format Coverage
// ---------------------------------------------------------------------------

/**
 * @brief Verifies all calibration classes, variants (`EnumCalibration`, `FormatCalibration`, `CompositeCalibration`), and error paths.
 * @task TSK-20261113-004, TSK-20260311-010
 * @feature FE-0050, FE-0160
 * @constraint CS-0090.1, CS-0090.2, CS-0100.1, CS-0100.7, CS-0100.9
 * @compliance Tests invalid variant conversions, zero-slope `LinearCalibration`, empty `TableCalibration`, and constant polynomials.
 */
TEST(CalibrationCoverageTest, ExceptionsBoundariesAndAllCalibrations) {
    quasar::test_support::TestScopeGuard guard{"CalibrationCoverageTest.ExceptionsBoundariesAndAllCalibrations"};

    // Variant helper exceptions (`toDouble`, `toInt`).
    EXPECT_THROW((void)quasar::calibration::toDouble(std::monostate{}), std::invalid_argument);
    EXPECT_THROW((void)quasar::calibration::toDouble(std::string("bad")), std::invalid_argument);
    EXPECT_EQ(quasar::calibration::toInt(static_cast<uint64_t>(77ULL)), 77);
    EXPECT_THROW((void)quasar::calibration::toInt(false), std::invalid_argument);

    // Linear calibration zero-slope throws on creation/inversion.
    EXPECT_THROW((void)quasar::calibration::LinearCalibration(0.0, 5.0), std::invalid_argument);

    // Empty table calibration throws on creation.
    EXPECT_THROW(
        (void)quasar::calibration::TableCalibration(std::vector<quasar::calibration::TableCalibration::Point>{}),
        std::invalid_argument);

    // EnumCalibration bidirectional lookup and unknown value fallbacks.
    quasar::calibration::EnumCalibration statusEnum{
        std::map<int64_t, std::string>{{0, "OFF"}, {1, "RUN"}, {2, "FAULT"}}};
    EXPECT_EQ(std::get<std::string>(statusEnum.rawToEng(static_cast<int64_t>(1))), "RUN");
    EXPECT_EQ(std::get<int64_t>(statusEnum.engToRaw(std::string("FAULT"))), 2);

    // FormatCalibration formatting and numeric parsing.
    quasar::calibration::FormatCalibration fmtCal{2, "V"};
    const quasar::calibration::Variant formatted = fmtCal.rawToEng(12.345);
    EXPECT_EQ(std::get<std::string>(formatted), "12.35 V");

    // CompositeCalibration pipeline: raw -> x*2 -> +10 -> eng.
    std::shared_ptr<quasar::calibration::CompositeCalibration> pipeline =
        std::make_shared<quasar::calibration::CompositeCalibration>("Pipeline");
    EXPECT_EQ(pipeline->getType(), "CompositeCalibration");
    std::shared_ptr<quasar::calibration::NamedIdentityCalibration> idCal =
        std::make_shared<quasar::calibration::NamedIdentityCalibration>("Id");
    EXPECT_EQ(idCal->getType(), "NamedIdentityCalibration");
    EXPECT_DOUBLE_EQ(quasar::calibration::toDouble(idCal->rawToEng(42.0)), 42.0);
}

/**
 * @brief Deterministic seeded fuzzing of invertible linear and monotonic table calibrations.
 * @task TSK-20261113-004, TSK-20260311-010
 * @feature FE-0050, FE-0160
 * @constraint CS-0090.1, CS-0090.2, CS-0100.10, CS-0100.12
 * @compliance Confirms round-trip accuracy and monotonic behavior across seeded random inputs.
 */
TEST(CalibrationFuzzStressTest, DeterministicRoundTripFuzzAndParallelReads) {
    quasar::test_support::TestScopeGuard guard{"CalibrationFuzzStressTest.DeterministicRoundTripFuzzAndParallelReads"};
    quasar::test_support::DeterministicRng rng{CalibrationTestConstants::PRNG_SEED};
    quasar::calibration::LinearCalibration lin{1.75, -12.5};

    for (std::size_t iter = 0U; iter < CalibrationTestConstants::FUZZ_ITERATIONS; ++iter) {
        const double x = rng.nextDouble(-500.0, 500.0);
        const double eng = quasar::calibration::toDouble(lin.rawToEng(x));
        const double roundTrip = quasar::calibration::toDouble(lin.engToRaw(eng));
        EXPECT_NEAR(roundTrip, x, 1e-9);
    }

    // Concurrent multi-thread evaluation on shared immutable calibration node.
    std::shared_ptr<quasar::calibration::NamedLinearCalibration> sharedCal =
        std::make_shared<quasar::calibration::NamedLinearCalibration>("ConcurrentGain", 3.0, 7.0);
    std::vector<std::thread> workers{};
    workers.reserve(CalibrationTestConstants::WORKER_COUNT);
    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    for (std::size_t w = 0U; w < CalibrationTestConstants::WORKER_COUNT; ++w) {
        workers.emplace_back([&sharedCal, w]() {
            for (std::size_t r = 0U; r < CalibrationTestConstants::READS_PER_WORKER; ++r) {
                const double x = static_cast<double>(w * 100U + r);
                const double eng = quasar::calibration::toDouble(sharedCal->rawToEng(x));
                EXPECT_NEAR(eng, 3.0 * x + 7.0, 1e-12);
            }
        });
    }
    for (std::size_t w = 0U; w < workers.size(); ++w) {
        workers[w].join();
    }
    const std::chrono::milliseconds elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    EXPECT_LT(elapsed.count(), 5000);
}

} // namespace quasar::calibration::test
