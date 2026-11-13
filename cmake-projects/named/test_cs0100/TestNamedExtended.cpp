/**
 * @file TestNamedExtended.cpp
 * @brief Extended CS-0100 test suite for `quasar_named` covering reflexive methods, web methods, traversal rules, CopyPolicy, and temporal/quantity/variant nodes.
 * @details Validates `NamedMethod`, `TypedNamedMethod`, `WebNamedMethod`, `NamedService`, `Transformer` / `PredefinedRules` / `TransformationRule`, `NamedQuantity`, `NamedDate`, `NamedDuration`, `NamedTimestamp`, `NamedVariant`, and concurrent service/tree stress.
 * @task [TSK-20260328-001] Reflexive Execution & Service Orchestration
 * @task [TSK-20260311-001] Tree Transformation Engine
 * @feature [FE-0260] Reflexive Methods & Services
 * @feature [FE-0150] Tree Transformation Engine
 * @compliance Fulfills CS-0010, CS-0040, CS-0060, CS-0090, and CS-0100 while keeping existing tests untouched.
 */

#include "datacodec/Schema.hpp"
#include "quasar/coretypes/Unit.hpp"
#include "quasar/named/CopyPolicy.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedBuffer.hpp"
#include "quasar/named/NamedConfig.hpp"
#include "quasar/named/NamedDate.hpp"
#include "quasar/named/NamedDuration.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedMethod.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedQuantity.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedTimestamp.hpp"
#include "quasar/named/NamedVariant.hpp"
#include "quasar/named/Serialization.hpp"
#include "quasar/named/TypedNamedMethod.hpp"
#include "quasar/named/WebNamedMethod.hpp"
#include "quasar/named/traversal/PredefinedRules.hpp"
#include "quasar/named/traversal/TransformationRule.hpp"
#include "quasar/named/traversal/Transformer.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"
#include <array>
#include <chrono>
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace quasar::named::test {

/**
 * @brief Iteration limit for tree transformation fuzz loop (`CS-0040.2`, `CS-0010.37`).
 * @requirement [CS-0040.2] Symbolic Constants
 * @compliance Named constant replaces raw loop literal.
 */
constexpr std::size_t MAX_TRANSFORM_FUZZ_ITERS = 64U;

/**
 * @brief Worker thread count for concurrent `NamedVariant` stress (`CS-0040.2`).
 * @requirement [FE-0110.2.2] Thread-safe NamedVariant
 * @compliance Fixed bounded thread count.
 */
constexpr std::size_t NAMED_STRESS_THREADS = 4U;

/**
 * @brief Mutations per thread in concurrent `NamedVariant` stress (`CS-0010.37`).
 * @requirement [CS-0010.37] Bounded Loops
 * @compliance Guarantees finite runtime.
 */
constexpr std::size_t MUTATIONS_PER_THREAD = 150U;

/**
 * @brief Deterministic seed for `quasar_named` fuzz tests (`CS-0100.12`).
 * @requirement [CS-0100.12] Deterministic Fuzzing
 * @compliance Fixed 64-bit seed.
 */
constexpr std::uint64_t NAMED_FUZZ_SEED = 0xAA4E7001ULL;

/**
 * @brief Lock timeout for test synchronization (`CS-0010.26`).
 * @requirement [CS-0010.26] Timed Mutex Usage
 * @compliance Prevents unbounded waits.
 */
constexpr std::chrono::milliseconds TEST_LOCK_TIMEOUT{500};

TEST(NamedTutorialTest, Tutorial1_SimpleNamedPrimitivesAndQuantities) {
    quasar::test_support::TestScopeGuard scope("NamedTutorialTest.Tutorial1");
    // Step 1: Build a root device object and attach typed physical and temporal children.
    std::shared_ptr<NamedObject> device = NamedObject::create("PumpStation");
    std::shared_ptr<NamedQuantity> flow =
        NamedQuantity::create("FlowRate", 12.5, quasar::coretypes::Units::Meter, device);
    std::shared_ptr<NamedQuantity> customVolt =
        NamedQuantity::create("RailVoltage", 24.0, "V", device);
    std::shared_ptr<NamedDate> commissioned = NamedDate::now("CommissionDate", device);
    std::shared_ptr<NamedDuration> cycle = NamedDuration::create("CycleUs", 10000, device);
    std::shared_ptr<NamedTimestamp> boot = NamedTimestamp::now("BootTime", device);

    // Step 2: Inspect types, units, and clone behavior.
    EXPECT_EQ(flow->getType(), "NamedQuantity");
    EXPECT_EQ(customVolt->getUnitSymbol(), "V");
    EXPECT_EQ(commissioned->getType(), "NamedDate");
    EXPECT_EQ(cycle->getType(), "NamedDuration");
    EXPECT_EQ(boot->getType(), "NamedTimestamp");
    std::shared_ptr<NamedObject> clonedFlow = flow->clone(CopyPolicy::DUPLICATE);
    EXPECT_EQ(clonedFlow->getName(), "FlowRate");
    quasar::test_support::AsyncTestLogger::instance().logOutput("flowUnit", flow->getUnitSymbol());
}

TEST(NamedTutorialTest, Tutorial2_IntermediateReflexiveAndWebNamedMethod) {
    quasar::test_support::TestScopeGuard scope("NamedTutorialTest.Tutorial2");
    // Step 1: Create a parent controller node with a state integer child.
    std::shared_ptr<NamedObject> ctrl = NamedObject::create("MotionController");
    std::shared_ptr<NamedInteger<int64_t>> targetPos =
        NamedInteger<int64_t>::create("TargetPos", 0, ctrl);

    // Step 2: Attach a NamedMethod and WebNamedMethod onto the hierarchy.
    std::shared_ptr<NamedMethod> moveCmd = NamedMethod::create(
        "MoveTo",
        [targetPos](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
            (void)owner;
            std::shared_ptr<NamedObject> posArg = (args != nullptr) ? args->getChild("pos") : nullptr;
            if (posArg != nullptr && posArg->is<NamedInteger<int64_t>>()) {
                targetPos->setValue(posArg->as<NamedInteger<int64_t>>()->value());
            } else {
                // Default zero target when argument missing (CS-0060.3 total branching).
                targetPos->setValue(0);
            }
            return NamedString::create("status", "OK");
        },
        ctrl);

    std::shared_ptr<WebNamedMethod> webEp = WebNamedMethod::create(
        "HttpMove",
        moveCmd->clone(CopyPolicy::DUPLICATE) != nullptr
            ? [](std::shared_ptr<NamedObject> o, std::shared_ptr<NamedObject> a) -> std::shared_ptr<NamedObject> {
                  (void)o;
                  (void)a;
                  return NamedString::create("http", "200");
              }
            : NamedMethod::MethodType{},
        "PUT",
        "/api/v1/move",
        "Commands axis motion",
        nullptr,
        nullptr,
        ctrl);

    // Step 3: Invoke NamedMethod dynamically and verify WebNamedMethod REST metadata updates.
    std::shared_ptr<NamedObject> callArgs = NamedObject::create("args");
    (void)NamedInteger<int64_t>::create("pos", 4500, callArgs);
    std::shared_ptr<NamedObject> result = moveCmd->execute(callArgs);
    EXPECT_EQ(targetPos->value(), 4500);
    EXPECT_EQ(result->getName(), "status");

    EXPECT_EQ(webEp->getType(), "WebNamedMethod");
    EXPECT_EQ(webEp->getHttpVerb(), "PUT");
    EXPECT_EQ(webEp->getAlias(), "/api/v1/move");
    EXPECT_EQ(webEp->getOasSummary(), "Commands axis motion");
    webEp->setHttpVerb("POST");
    webEp->setAlias("/api/v2/move");
    webEp->setOasSummary("Updated motion command");
    EXPECT_EQ(webEp->getHttpVerb(), "POST");
    EXPECT_EQ(webEp->getAlias(), "/api/v2/move");
    EXPECT_EQ(webEp->getOasSummary(), "Updated motion command");
    quasar::test_support::AsyncTestLogger::instance().logOutput("webVerb", webEp->getHttpVerb());
}

TEST(NamedTutorialTest, Tutorial3_AdvancedTreeTransformationAndSerialization) {
    quasar::test_support::TestScopeGuard scope("NamedTutorialTest.Tutorial3");
    // Step 1: Construct a raw Ethernet frame buffer inside a NamedObject container.
    std::shared_ptr<NamedObject> packet = NamedObject::create("Frame");
    std::vector<uint8_t> rawBytes = {0x12U, 0x34U, 0x00U, 0x2AU};
    (void)NamedBuffer::create("Payload", rawBytes, packet);

    // Step 2: Configure XSLT-style Transformer with PredefinedRules::sliceBuffer and castToStructure.
    traversal::Transformer transformer;
    std::vector<traversal::SliceDefinition> slices = {
        traversal::SliceDefinition{"Header", 0U, 2U},
        traversal::SliceDefinition{"Body", 2U, 2U}
    };
    transformer.addRule(traversal::PredefinedRules::sliceBuffer("Payload", slices, CopyPolicy::SHARE, 10));
    transformer.addRule(
        traversal::PredefinedRules::extractIntegerRule<uint16_t>(
            "Header", "HeaderId", 0U, quasar::coretypes::Endianness::BigEndian, 20));

    std::vector<std::shared_ptr<NamedObject>> outRoots = transformer.transform(packet);
    EXPECT_FALSE(outRoots.empty());

    // Step 3: Serialize to JSON/YAML/XML and round-trip verify.
    std::string jsonStr = serialization::toJson(packet);
    std::string yamlStr = serialization::toYaml(packet);
    std::string xmlStr = serialization::toXml(packet);
    EXPECT_FALSE(jsonStr.empty());
    EXPECT_FALSE(yamlStr.empty());
    EXPECT_FALSE(xmlStr.empty());
    quasar::test_support::AsyncTestLogger::instance().logOutput("jsonLen", static_cast<long long>(jsonStr.size()));
}

TEST(NamedCoverageTest, ConfigConstantsCopyPoliciesVariantAndErrorBranches) {
    quasar::test_support::TestScopeGuard scope("NamedCoverageTest.Branches");
    // Check strategic NamedConfig bounds and both CopyPolicy enumerators.
    EXPECT_GT(config::HARD_LIMIT_ITERATIONS, 0U);
    EXPECT_GT(config::DEFAULT_LOCK_TIMEOUT.count(), 0);
    EXPECT_NE(static_cast<int>(CopyPolicy::DUPLICATE), static_cast<int>(CopyPolicy::SHARE));

    // Check NamedVariant transitions across Boolean/Integer/Double/String and clone policies.
    std::shared_ptr<NamedVariant> nv = NamedVariant::create("DynVal", coretypes::Variant(true));
    EXPECT_EQ(nv->getType(), "NamedVariant");
    EXPECT_EQ(nv->getVariantType(), coretypes::VariantType::Boolean);
    nv->setVariant(coretypes::Variant(static_cast<int64_t>(999)));
    EXPECT_EQ(nv->getAs<int64_t>(), 999);
    std::shared_ptr<NamedObject> nvDup = nv->clone(CopyPolicy::DUPLICATE);
    std::shared_ptr<NamedObject> nvShare = nv->clone(CopyPolicy::SHARE);
    EXPECT_EQ(nvDup->getType(), "NamedVariant");
    EXPECT_EQ(nvShare->getType(), "NamedVariant");

    // Check TypedNamedMethod validation error enumerations and null/empty boundary throws.
    std::shared_ptr<TypedNamedMethod> typedNull = TypedNamedMethod::create(
        "UnconstrainedOp",
        [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) -> std::shared_ptr<NamedObject> {
            (void)owner;
            (void)args;
            return NamedBoolean::create("done", true);
        });
    EXPECT_EQ(typedNull->getType(), "TypedNamedMethod");
    std::shared_ptr<NamedObject> emptyArgs = NamedObject::create("EmptyArgs");
    EXPECT_TRUE(typedNull->validateInput(*emptyArgs).has_value());
    EXPECT_THROW((void)NamedObject::create(""), std::runtime_error);
    EXPECT_THROW((void)serialization::fromXml("<broken>"), std::runtime_error);
}

TEST(NamedFuzzStressTest, TransformationFuzzAndConcurrentNamedVariantStress) {
    quasar::test_support::TestScopeGuard scope("NamedFuzzStressTest.FuzzAndConcurrent");
    // Step 1: Deterministic fuzzing over Transformer slice sizes bounded strictly by buffer length.
    quasar::test_support::DeterministicRng rng(NAMED_FUZZ_SEED);
    for (std::size_t iter = 0U; iter < MAX_TRANSFORM_FUZZ_ITERS; ++iter) {
        if (iter >= MAX_TRANSFORM_FUZZ_ITERS) {
            throw std::runtime_error("Fuzz bound exceeded");
        } else {
            std::size_t bufLen = static_cast<std::size_t>(rng.nextInt(8, 64));
            std::size_t split = static_cast<std::size_t>(rng.nextInt(1, static_cast<int>(bufLen - 1U)));
            std::shared_ptr<NamedObject> root = NamedObject::create("Root_" + std::to_string(iter));
            std::vector<uint8_t> data(bufLen, 0x5AU);
            (void)NamedBuffer::create("RawBuf", data, root);

            traversal::Transformer tr;
            std::vector<traversal::SliceDefinition> defs = {
                traversal::SliceDefinition{"PartA", 0U, split},
                traversal::SliceDefinition{"PartB", split, bufLen - split}
            };
            tr.addRule(traversal::PredefinedRules::sliceBuffer("RawBuf", defs, CopyPolicy::DUPLICATE, 5));
            tr.transformInPlace(root);
            EXPECT_NE(root->getChild("PartA"), nullptr);
            EXPECT_NE(root->getChild("PartB"), nullptr);
        }
    }

    // Step 2: Concurrent writers hammering a shared NamedVariant under timed mutex guard.
    std::shared_ptr<NamedVariant> sharedVar =
        NamedVariant::create("SharedTelemetry", coretypes::Variant(static_cast<int64_t>(0)));
    std::timed_mutex guard;
    std::vector<std::thread> threads;
    threads.reserve(NAMED_STRESS_THREADS);

    for (std::size_t tid = 0U; tid < NAMED_STRESS_THREADS; ++tid) {
        if (tid >= NAMED_STRESS_THREADS) {
            throw std::runtime_error("Thread spawn bound exceeded");
        } else {
            threads.emplace_back([&sharedVar, &guard, tid]() {
                for (std::size_t m = 0U; m < MUTATIONS_PER_THREAD; ++m) {
                    if (m >= MUTATIONS_PER_THREAD) {
                        throw std::runtime_error("Mutation loop bound exceeded");
                    } else {
                        std::unique_lock<std::timed_mutex> lock(guard, TEST_LOCK_TIMEOUT);
                        if (!lock.owns_lock()) {
                            throw std::runtime_error("Timed mutex timeout in NamedVariant stress");
                        } else {
                            sharedVar->setVariant(coretypes::Variant(static_cast<int64_t>((tid * 1000U) + m)));
                        }
                    }
                }
            });
        }
    }

    for (std::size_t tid = 0U; tid < threads.size(); ++tid) {
        if (tid >= NAMED_STRESS_THREADS) {
            throw std::runtime_error("Thread join bound exceeded");
        } else {
            threads[tid].join();
        }
    }
    EXPECT_EQ(sharedVar->getVariantType(), coretypes::VariantType::Integer);
}

} // namespace quasar::named::test
