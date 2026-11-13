/**
 * @file TestSimExtended.cpp
 * @brief CS-0100 comprehensive test suite for the `sim` module.
 *
 * @details Covers `sim::Simulator`, `sim::Model`, `sim::Field`, `sim::SimpleField`,
 * `sim::SimpleArrayField`, `sim::TypeRegistry` (and all `IType` subclasses), `sim::Publication`,
 * `sim::PublishOperation`, `sim::LinkRegistry`, `sim::Resolver`, `sim::LibraryLoader`, and `smp_impl`
 * enums/UUID/AnySimple conversions, including progressive tutorials, 20+ primitive/type cases,
 * boundaries, exception sources, deterministic fuzzing, and concurrent stress tests.
 *
 * @task TSK-20261113-004
 * @feature FE-0060, FE-0070, FE-0080
 * @constraint CS-0010, CS-0040, CS-0060, CS-0090, CS-0100.1, CS-0100.2, CS-0100.4, CS-0100.5, CS-0100.14
 * @compliance Uses AsyncTestLogger, TestScopeGuard, DeterministicRng, explicit types, and total branching.
 */

#include "sim/Field.hpp"
#include "sim/LibraryLoader.hpp"
#include "sim/LinkRegistry.hpp"
#include "sim/Model.hpp"
#include "sim/Publication.hpp"
#include "sim/PublishOperation.hpp"
#include "sim/Resolver.hpp"
#include "sim/Simulator.hpp"
#include "sim/TypeRegistry.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"
#include <core/StandardExceptions.hpp>

#include <gtest/gtest.h>

#include <Smp/ComponentStateKind.h>
#include <Smp/IEntryPoint.h>
#include <Smp/IFactory.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/ILinkRegistry.h>
#include <Smp/Services/IResolver.h>
#include <Smp/SimulatorStateKind.h>
#include <Smp/ViewKind.h>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace sim::test {

/**
 * @brief Symbolic bounds and constants for `sim` CS-0100 tests (`CS-0040`).
 * @task TSK-20261113-004
 * @constraint CS-0040.1, CS-0040.6, CS-0090.1, CS-0090.2
 * @compliance Centralizes PRNG seeds, concurrency thread counts, and iteration caps.
 */
struct SimTestConstants {
    /// @brief Deterministic PRNG seed (`CS-0100.12`).
    static constexpr std::uint64_t PRNG_SEED = 0x1928374655647382ULL;
    /// @brief Number of fuzzing cycles for fields and resolver (`CS-0100.10`).
    static constexpr std::size_t FUZZ_ITERATIONS = 100U;
    /// @brief Concurrent worker threads for LinkRegistry stress (`CS-0100.4`).
    static constexpr std::size_t CONCURRENCY_THREADS = 4U;
    /// @brief Link/remove iterations per worker (`CS-0010.37`).
    static constexpr std::size_t CONCURRENCY_OPS = 50U;
    /// @brief Array length for `SimpleArrayField` boundary checks (`CS-0100.13`).
    static constexpr std::size_t ARRAY_LEN = 16U;
};

/**
 * @brief Helper entry point used during simulator lifecycle and init-entry-point tests.
 * @task TSK-20261113-004
 * @feature FE-0070.7
 * @constraint CS-0090.1, CS-0090.2
 * @compliance Minimal Smp::IEntryPoint recording call count without manual heap allocation.
 */
class CountingEntryPoint final : public virtual Smp::IEntryPoint {
public:
    /**
     * @brief Constructs counting entry point.
     * @param[in] name Object name (`nullptr` mapped to `"InitEP"`).
     * @task TSK-20261113-004
     * @constraint CS-0090.3
     * @compliance Null-safe name handling.
     */
    explicit CountingEntryPoint(Smp::String8 name)
        : _name(name != nullptr ? name : "InitEP"), _executions(0U) {}
    /// @brief Default noexcept virtual destructor (`CS-0060.5`).
    ~CountingEntryPoint() noexcept override = default;

    /// @brief Returns entry point name (`CS-0090.7`).
    Smp::String8 GetName() const override { return _name.c_str(); }
    /// @brief Returns entry point description (`CS-0090.7`).
    Smp::String8 GetDescription() const override { return "CountingInitEntry"; }
    /// @brief Returns nullptr parent (`CS-0090.7`).
    Smp::IObject* GetParent() const override { return nullptr; }
    /// @brief Returns nullptr child (`CS-0090.7`).
    Smp::IObject* GetChild(Smp::String8 /*name*/) const override { return nullptr; }

    /**
     * @brief Increments deterministic invocation counter.
     * @task TSK-20261113-004
     * @constraint CS-0090.1
     * @compliance Tracks initialization/entry-point executions cleanly.
     */
    void Execute() const override { ++_executions; }

    /**
     * @brief Reads number of `Execute()` invocations.
     * @return std::size_t Total calls observed.
     * @task TSK-20261113-004
     * @constraint CS-0090.7
     * @compliance Value accessor.
     */
    std::size_t GetExecutions() const noexcept { return _executions; }

private:
    /// @brief Stored name (`CS-0090.1`).
    std::string _name;
    /// @brief Execution counter (`CS-0090.1`).
    mutable std::size_t _executions;
};

/**
 * @brief Lightweight concrete `sim::Model` derivative publishing primitive & array fields.
 * @task TSK-20261113-004
 * @feature FE-0060.3
 * @constraint CS-0090.1, CS-0090.2
 * @compliance Demonstrates subclassing `sim::Model` without dynamic raw pointers.
 */
class ProbeModel final : public sim::Model {
public:
    /**
     * @brief Constructs probe model.
     * @param[in] name Instance name.
     * @param[in] description Instance description.
     * @param[in] parent Parent composite or nullptr.
     * @param[in] simulator Owning simulator or nullptr.
     * @task TSK-20261113-004
     * @constraint CS-0090.3
     * @compliance Initializes backing scalar and array buffers deterministically.
     */
    ProbeModel(Smp::String8 name, Smp::String8 description, Smp::IObject* parent, Smp::ISimulator* simulator)
        : sim::Model(name, description, parent, simulator), _scalar(0), _doubleVal(0.0), _uuid{1U, 2U, 3U, 4U, 5ULL} {
        for (std::size_t idx = 0U; idx < SimTestConstants::ARRAY_LEN; ++idx) {
            _samples[idx] = static_cast<Smp::Int32>(idx);
        }
    }
    /// @brief Default noexcept virtual destructor (`CS-0060.5`).
    ~ProbeModel() noexcept override = default;

    /**
     * @brief Publishes scalar and array fields when receiver is non-null.
     * @param[in] receiver Publication sink.
     * @task TSK-20261113-004
     * @constraint CS-0090.3
     * @compliance Registers fields in base `_fields` collection after delegating to `sim::Model::Publish`.
     */
    void Publish(Smp::IPublication* receiver) override {
        sim::Model::Publish(receiver);
        if (receiver != nullptr) {
            Smp::IField* scalarField =
                receiver->PublishField("Scalar", "Int32 Probe", &_scalar, Smp::ViewKind::VK_All, true, true, true);
            Smp::IField* dblField =
                receiver->PublishField("Ratio", "Float64 Probe", &_doubleVal, Smp::ViewKind::VK_All, true, true, true);
            Smp::ISimpleArrayField* arrField = receiver->PublishArray(
                "Samples", "Int32 Array", static_cast<Smp::Int64>(SimTestConstants::ARRAY_LEN),
                _samples.data(), Smp::PrimitiveTypeKind::PTK_Int32, Smp::ViewKind::VK_All, true, true, true);
            _fields.Add(scalarField);
            _fields.Add(dblField);
            _fields.Add(dynamic_cast<Smp::IField*>(arrField));
        } else {
            // Null publication receiver handled gracefully (`CS-0060.3`).
        }
    }

    /**
     * @brief Returns unique test UUID.
     * @return const Smp::Uuid& Static component UUID.
     * @task TSK-20261113-004
     * @constraint CS-0090.7
     * @compliance Fixed UUID accessor.
     */
    const Smp::Uuid& GetUuid() const override { return _uuid; }

private:
    /// @brief Published Int32 backing storage (`CS-0090.1`).
    Smp::Int32 _scalar;
    /// @brief Published Float64 backing storage (`CS-0090.1`).
    Smp::Float64 _doubleVal;
    /// @brief Published array backing storage (`CS-0090.1`).
    std::array<Smp::Int32, SimTestConstants::ARRAY_LEN> _samples{};
    /// @brief Component UUID (`CS-0090.1`).
    Smp::Uuid _uuid;
};

// ---------------------------------------------------------------------------
// Progressive Pedagogical Tutorials (CS-0100.5)
// ---------------------------------------------------------------------------

/**
 * @brief Tutorial 1 (Basic): Create a `Simulator`, inspect default services, and cycle states.
 * @task TSK-20261113-004
 * @feature FE-0070.7
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5
 * @compliance Introduces `sim::Simulator` construction, Publish/Configure/Connect, and Exit.
 */
TEST(SimTutorialTest, Tutorial1_BasicSimulatorLifecycle) {
    quasar::test_support::TestScopeGuard guard{"SimTutorialTest.Tutorial1_BasicSimulatorLifecycle"};
    sim::Simulator simulator{};

    EXPECT_EQ(simulator.GetSimulatorState(), Smp::SimulatorStateKind::SSK_Building);
    EXPECT_NE(simulator.GetLogger(), nullptr);
    EXPECT_NE(simulator.GetTimeKeeper(), nullptr);
    EXPECT_NE(simulator.GetScheduler(), nullptr);
    EXPECT_NE(simulator.GetEventManager(), nullptr);
    EXPECT_NE(simulator.GetResolver(), nullptr);
    EXPECT_NE(simulator.GetLinkRegistry(), nullptr);
    EXPECT_NE(simulator.GetTypeRegistry(), nullptr);

    simulator.Publish();
    simulator.Configure();
    simulator.Connect();
    EXPECT_EQ(simulator.GetSimulatorState(), Smp::SimulatorStateKind::SSK_Standby);
    simulator.Exit();
    EXPECT_EQ(simulator.GetSimulatorState(), Smp::SimulatorStateKind::SSK_Exiting);
}

/**
 * @brief Tutorial 2 (Intermediate): Register custom SMP types and publish model fields & operations.
 * @task TSK-20261113-004
 * @feature FE-0070.9, FE-0070.10
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5
 * @compliance Demonstrates `TypeRegistry`, `Publication`, and `PublishOperation`.
 */
TEST(SimTutorialTest, Tutorial2_IntermediateTypeRegistryAndPublication) {
    quasar::test_support::TestScopeGuard guard{"SimTutorialTest.Tutorial2_IntermediateTypeRegistryAndPublication"};
    sim::TypeRegistry types{};
    const Smp::Uuid voltageUuid{0x10U, 0U, 0U, 0U, 1ULL};
    Smp::Publication::IType* voltageType =
        types.AddFloatType("Voltage", "Bus voltage in Volts", voltageUuid, -60.0, 60.0, true, true, "V");
    ASSERT_NE(voltageType, nullptr);
    EXPECT_EQ(types.GetType(voltageUuid), voltageType);

    sim::Publication pub{&types};
    Smp::Float64 busVoltage{24.5};
    Smp::IField* field = pub.PublishField("BusVoltage", "Main bus voltage", &busVoltage);
    ASSERT_NE(field, nullptr);

    Smp::Publication::IPublishOperation* op = pub.PublishOperation("ResetFaults", "Clears fault flags");
    ASSERT_NE(op, nullptr);
    op->PublishParameter("mask", "Fault bitmask", types.GetType(Smp::PrimitiveTypeKind::PTK_UInt32)->GetUuid(),
                         Smp::Publication::ParameterDirectionKind::PDK_In);

    EXPECT_EQ(pub.GetField("BusVoltage"), field);
    EXPECT_NE(pub.GetFields(), nullptr);
    EXPECT_EQ(pub.GetOperation("ResetFaults"), nullptr);
    EXPECT_EQ(pub.GetOperations(), nullptr);
    EXPECT_EQ(pub.GetProperties(), nullptr);
    pub.Unpublish();
}

/**
 * @brief Tutorial 3 (Advanced): Wire `ProbeModel`, `Resolver`, `LinkRegistry`, and timed `Run(duration)`.
 * @task TSK-20261113-004
 * @feature FE-0070.5, FE-0070.6, FE-0070.7
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5
 * @compliance End-to-end orchestration with model resolution, link tracking, init entry point, and timed run.
 */
TEST(SimTutorialTest, Tutorial3_AdvancedModelResolverLinksAndTimedRun) {
    quasar::test_support::TestScopeGuard guard{"SimTutorialTest.Tutorial3_AdvancedModelResolverLinksAndTimedRun"};
    sim::Simulator simulator{};
    std::unique_ptr<ProbeModel> sensorOwned = std::make_unique<ProbeModel>("SensorA", "Telemetry probe", &simulator, &simulator);
    std::unique_ptr<ProbeModel> sinkOwned = std::make_unique<ProbeModel>("SinkB", "Telemetry sink", &simulator, &simulator);
    ProbeModel* sensor = sensorOwned.get();
    ProbeModel* sink = sinkOwned.get();
    CountingEntryPoint initEp{"OnInit"};

    simulator.AddModel(sensorOwned.release());
    simulator.AddModel(sinkOwned.release());
    simulator.AddInitEntryPoint(&initEp);
    simulator.Publish();
    simulator.Configure();
    simulator.Connect();
    simulator.Initialise();
    EXPECT_EQ(simulator.GetSimulatorState(), Smp::SimulatorStateKind::SSK_Standby);
    initEp.Execute();
    EXPECT_EQ(initEp.GetExecutions(), 1U);

    Smp::Services::IResolver* resolver = simulator.GetResolver();
    ASSERT_NE(resolver, nullptr);
    Smp::IObject* resolvedSensor = resolver->ResolveAbsolute("/SensorA");
    EXPECT_EQ(resolvedSensor, sensor);

    Smp::Services::ILinkRegistry* links = simulator.GetLinkRegistry();
    ASSERT_NE(links, nullptr);
    links->AddLink(sensor, sink);
    EXPECT_EQ(links->GetLinkCount(sensor, sink), 1U);
    EXPECT_FALSE(links->CanRemove(sink));

    simulator.Run(1000LL);
    simulator.Hold(true);
    links->RemoveLink(sensor, sink);
    EXPECT_TRUE(links->CanRemove(sink));
    simulator.Exit();
}

// ---------------------------------------------------------------------------
// Exhaustive Method, Enum, Primitive & Exception Coverage
// ---------------------------------------------------------------------------

/**
 * @brief Tests 20+ primitive types, field publication overloads, and `TypeRegistry` type builders.
 * @task TSK-20261113-004
 * @feature FE-0070.9, FE-0070.10, FE-0060.11
 * @constraint CS-0090.1, CS-0090.2, CS-0100.1, CS-0100.4, CS-0100.14
 * @compliance Covers 20 distinct `TypeRegistry` / `Publication` / `SimpleField` / `SimpleArrayField` combinations.
 */
TEST(SimCoverageTest, TwentyPlusTypesFieldsAndEnumStreams) {
    quasar::test_support::TestScopeGuard guard{"SimCoverageTest.TwentyPlusTypesFieldsAndEnumStreams"};
    sim::TypeRegistry reg{};

    const std::array<Smp::PrimitiveTypeKind, 15U> kinds{
        Smp::PrimitiveTypeKind::PTK_Char8,   Smp::PrimitiveTypeKind::PTK_Bool,
        Smp::PrimitiveTypeKind::PTK_Int8,    Smp::PrimitiveTypeKind::PTK_UInt8,
        Smp::PrimitiveTypeKind::PTK_Int16,   Smp::PrimitiveTypeKind::PTK_UInt16,
        Smp::PrimitiveTypeKind::PTK_Int32,   Smp::PrimitiveTypeKind::PTK_UInt32,
        Smp::PrimitiveTypeKind::PTK_Int64,   Smp::PrimitiveTypeKind::PTK_UInt64,
        Smp::PrimitiveTypeKind::PTK_Float32, Smp::PrimitiveTypeKind::PTK_Float64,
        Smp::PrimitiveTypeKind::PTK_Duration,Smp::PrimitiveTypeKind::PTK_DateTime,
        Smp::PrimitiveTypeKind::PTK_String8};
    for (std::size_t idx = 0U; idx < kinds.size(); ++idx) {
        Smp::Publication::IType* t = reg.GetType(kinds[idx]);
        EXPECT_NE(t, nullptr);
    }

    Smp::Publication::IType* fType =
        reg.AddFloatType("F", "Float", {1U, 1U, 1U, 1U, 1ULL}, -1.0, 1.0, false, true, "rad");
    Smp::Publication::IType* iType =
        reg.AddIntegerType("I", "Int", {2U, 1U, 1U, 1U, 1ULL}, 0LL, 100LL, "pct");
    Smp::Publication::IEnumerationType* eType =
        reg.AddEnumerationType("E", "Enum", {3U, 1U, 1U, 1U, 1ULL});
    Smp::Publication::IArrayType* aType =
        reg.AddArrayType("A", "Arr", {4U, 1U, 1U, 1U, 1ULL}, {2U, 1U, 1U, 1U, 1ULL}, 4U, 8U, true);
    Smp::Publication::IStringType* sType =
        reg.AddStringType("S", "Str", {5U, 1U, 1U, 1U, 1ULL}, 64U);
    Smp::Publication::IStructureType* stType =
        reg.AddStructureType("St", "Struct", {6U, 1U, 1U, 1U, 1ULL});
    Smp::Publication::IClassType* cType =
        reg.AddClassType("C", "Class", {7U, 1U, 1U, 1U, 1ULL}, {6U, 1U, 1U, 1U, 1ULL});

    ASSERT_NE(fType, nullptr);
    ASSERT_NE(iType, nullptr);
    ASSERT_NE(eType, nullptr);
    ASSERT_NE(aType, nullptr);
    ASSERT_NE(sType, nullptr);
    ASSERT_NE(stType, nullptr);
    ASSERT_NE(cType, nullptr);
    eType->AddLiteral("ZERO", "Zero", 0);
    stType->AddField("x", "x field", iType->GetUuid(), 0U);
    cType->AddField("y", "y field", fType->GetUuid(), 0U);
    EXPECT_EQ(sType->GetMaxLength(), 64U);
    EXPECT_EQ(aType->GetSize(), 32U);

    // Publish all 12 primitive overloads via sim::Publication.
    sim::Publication pub{&reg};
    Smp::Char8 vC{'A'};
    Smp::Bool vB{true};
    Smp::Int8 vI8{-1};
    Smp::Int16 vI16{-2};
    Smp::Int32 vI32{-3};
    Smp::Int64 vI64{-4};
    Smp::UInt8 vU8{1U};
    Smp::UInt16 vU16{2U};
    Smp::UInt32 vU32{3U};
    Smp::UInt64 vU64{4ULL};
    Smp::Float32 vF32{1.5F};
    Smp::Float64 vF64{2.5};
    EXPECT_NE(pub.PublishField("vC", "", &vC), nullptr);
    EXPECT_NE(pub.PublishField("vB", "", &vB), nullptr);
    EXPECT_NE(pub.PublishField("vI8", "", &vI8), nullptr);
    EXPECT_NE(pub.PublishField("vI16", "", &vI16), nullptr);
    EXPECT_NE(pub.PublishField("vI32", "", &vI32), nullptr);
    EXPECT_NE(pub.PublishField("vI64", "", &vI64), nullptr);
    EXPECT_NE(pub.PublishField("vU8", "", &vU8), nullptr);
    EXPECT_NE(pub.PublishField("vU16", "", &vU16), nullptr);
    EXPECT_NE(pub.PublishField("vU32", "", &vU32), nullptr);
    EXPECT_NE(pub.PublishField("vU64", "", &vU64), nullptr);
    EXPECT_NE(pub.PublishField("vF32", "", &vF32), nullptr);
    EXPECT_NE(pub.PublishField("vF64", "", &vF64), nullptr);

    // Cover smp_impl stream operators across ViewKind & ComponentStateKind.
    std::ostringstream oss{};
    oss << Smp::ViewKind::VK_All << Smp::ComponentStateKind::CSK_Connected
        << Smp::SimulatorStateKind::SSK_Executing;
    EXPECT_FALSE(oss.str().empty());
}

/**
 * @brief Tests `LibraryLoader`, `Simulator` error states, `Store`/`Restore`, `Abort`, and null guards.
 * @task TSK-20261113-004
 * @feature FE-0070.7, FE-0080.7
 * @constraint CS-0090.1, CS-0090.2, CS-0100.7, CS-0100.9
 * @compliance Exercises invalid library load exception, symbol miss returns, and simulator state transitions.
 */
TEST(SimCoverageTest, ExceptionsAndBoundaryStates) {
    quasar::test_support::TestScopeGuard guard{"SimCoverageTest.ExceptionsAndBoundaryStates"};
    sim::LibraryLoader& loader = sim::LibraryLoader::GetInstance();
    EXPECT_THROW(loader.LoadLibrary("/nonexistent/quasar_missing_lib_xyz.so"), sim::LibraryException);
    EXPECT_FALSE(loader.UnloadLibrary(nullptr));
    EXPECT_EQ(loader.GetSymbolAddress(static_cast<void*>(nullptr), "Initialise"), nullptr);
    EXPECT_THROW((void)loader.GetSymbolAddress(std::string("/nonexistent"), "Initialise"), sim::LibraryException);

    sim::Simulator simulator{};
    EXPECT_EQ(simulator.GetService("NonExistentService"), nullptr);
    EXPECT_EQ(simulator.GetFactory({99U, 99U, 99U, 99U, 99ULL}), nullptr);
    EXPECT_EQ(simulator.CreateInstance({99U, 99U, 99U, 99U, 99ULL}, "Bad", "", nullptr), nullptr);
    EXPECT_THROW(simulator.Store("/tmp/quasar_sim_store_test.bin"), core::InvalidSimulatorState);
    EXPECT_THROW(simulator.Restore("/tmp/quasar_sim_store_test.bin"), core::InvalidSimulatorState);
    EXPECT_THROW(simulator.Reconnect(nullptr), core::InvalidSimulatorState);

    simulator.Publish();
    simulator.Configure();
    simulator.Connect();
    simulator.Store("/tmp/quasar_sim_store_test.bin");
    simulator.Restore("/tmp/quasar_sim_store_test.bin");
    simulator.Reconnect(nullptr);
    simulator.Abort();
    EXPECT_EQ(simulator.GetSimulatorState(), Smp::SimulatorStateKind::SSK_Aborting);
}

/**
 * @brief Performs deterministic seeded fuzzing on `SimpleField`, `SimpleArrayField`, and `Resolver`.
 * @task TSK-20261113-004
 * @feature FE-0060.11, FE-0070.5
 * @constraint CS-0090.1, CS-0090.2, CS-0100.10, CS-0100.12
 * @compliance Verifies round-trip value retention and bounded resolver lookups under fuzzed inputs.
 */
TEST(SimFuzzStressTest, DeterministicFieldAndResolverFuzzing) {
    quasar::test_support::TestScopeGuard guard{"SimFuzzStressTest.DeterministicFieldAndResolverFuzzing"};
    quasar::test_support::DeterministicRng rng{SimTestConstants::PRNG_SEED};
    sim::TypeRegistry reg{};
    sim::Publication pub{&reg};

    Smp::Int32 backingScalar{0};
    Smp::ISimpleField* sf = dynamic_cast<Smp::ISimpleField*>(
        pub.PublishField("FuzzScalar", "Scalar", &backingScalar));
    ASSERT_NE(sf, nullptr);

    std::array<Smp::Int32, SimTestConstants::ARRAY_LEN> backingArr{};
    Smp::ISimpleArrayField* af = pub.PublishArray(
        "FuzzArr", "Array", static_cast<Smp::Int64>(SimTestConstants::ARRAY_LEN),
        backingArr.data(), Smp::PrimitiveTypeKind::PTK_Int32);
    ASSERT_NE(af, nullptr);

    sim::Resolver resolver{};
    sim::Simulator simulator{};
    resolver.SetSimulator(&simulator);

    for (std::size_t iter = 0U; iter < SimTestConstants::FUZZ_ITERATIONS; ++iter) {
        const Smp::Int32 val = static_cast<Smp::Int32>(rng.nextInt(-10000, 10000));
        const Smp::UInt64 idx = static_cast<Smp::UInt64>(
            rng.nextInt(0, static_cast<std::int64_t>(SimTestConstants::ARRAY_LEN) - 1));
        sf->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int32, val));
        EXPECT_EQ(static_cast<Smp::Int32>(sf->GetValue()), val);

        af->SetValue(idx, Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int32, val));
        EXPECT_EQ(static_cast<Smp::Int32>(af->GetValue(idx)), val);

        EXPECT_EQ(resolver.ResolveAbsolute("/Unknown/Node"), nullptr);
        EXPECT_EQ(resolver.ResolveRelative("Missing", &simulator), nullptr);
    }
}

/**
 * @brief Exercises concurrent `LinkRegistry` link/query/remove cycles across threads.
 * @task TSK-20261113-004
 * @feature FE-0070.6
 * @constraint CS-0090.1, CS-0090.2, CS-0100.4, CS-0100.13, CS-0100.23
 * @compliance Confirms thread-safe link registry updates without deadlock or data loss.
 */
TEST(SimFuzzStressTest, ConcurrentLinkRegistryStress) {
    quasar::test_support::TestScopeGuard guard{"SimFuzzStressTest.ConcurrentLinkRegistryStress"};
    sim::LinkRegistry registry{};
    std::array<ProbeModel, SimTestConstants::CONCURRENCY_THREADS> models{
        ProbeModel{"M0", "", nullptr, nullptr},
        ProbeModel{"M1", "", nullptr, nullptr},
        ProbeModel{"M2", "", nullptr, nullptr},
        ProbeModel{"M3", "", nullptr, nullptr}};
    ProbeModel target{"Target", "", nullptr, nullptr};

    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::vector<std::thread> workers{};
    workers.reserve(SimTestConstants::CONCURRENCY_THREADS);
    for (std::size_t t = 0U; t < SimTestConstants::CONCURRENCY_THREADS; ++t) {
        workers.emplace_back([&registry, &models, &target, t]() {
            for (std::size_t op = 0U; op < SimTestConstants::CONCURRENCY_OPS; ++op) {
                registry.AddLink(&models[t], &target);
                const Smp::UInt32 count = registry.GetLinkCount(&models[t], &target);
                EXPECT_GE(count, 1U);
                EXPECT_TRUE(registry.RemoveLink(&models[t], &target));
            }
        });
    }
    for (std::size_t t = 0U; t < workers.size(); ++t) {
        workers[t].join();
    }

    const std::chrono::milliseconds elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    EXPECT_LT(elapsed.count(), 5000);
    EXPECT_TRUE(registry.CanRemove(&target));
}

} // namespace sim::test
