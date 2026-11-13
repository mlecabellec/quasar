/**
 * @file TestSampleModels.cpp
 * @brief CS-0100 compliant unit, template, fuzz, and tutorial tests for `sample-models`.
 *
 * @details Covers `sample::SimpleCollection<T>` (24 template instantiations), `sample::SmpModel`,
 * `sample::InverterModel`, `sample::DelayedSimpleModel`, and `sample::DelayedArrayModel`.
 *
 * @task TSK-20261113-004
 * @feature FE-0020, FE-0060
 * @constraint CS-0010, CS-0040, CS-0060, CS-0090, CS-0100.1, CS-0100.2, CS-0100.5, CS-0100.14
 * @compliance Uses asynchronous non-blocking test logging, seeded PRNG, total branching, and 24 template instantiations.
 */

#include "DelayedModels.hpp"
#include "InverterModel.hpp"
#include "common/SimpleCollection.hpp"
#include "sim/Publication.hpp"
#include "sim/TypeRegistry.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"

#include <gtest/gtest.h>

#include <Smp/IComponent.h>
#include <Smp/IContainer.h>
#include <Smp/IEntryPoint.h>
#include <Smp/IFactory.h>
#include <Smp/IField.h>
#include <Smp/IModel.h>
#include <Smp/IOperation.h>
#include <Smp/IProperty.h>
#include <Smp/IService.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimpleField.h>
#include <Smp/PrimitiveTypes.h>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

namespace sample::test {

/**
 * @brief Symbolic constants for sample-models test bounds and seeds.
 * @task TSK-20261113-004
 * @constraint CS-0040.1, CS-0040.6, CS-0090.1, CS-0090.2
 * @compliance Centralizes all iteration caps, pipeline delays, and deterministic PRNG seeds.
 */
struct SampleModelTestConstants {
    /// @brief Pipeline delay steps required for delayed models to surface inputs (`CS-0040.2`).
    static constexpr std::size_t DELAY_PIPELINE_STEPS = 3U;
    /// @brief Fuzz iterations per randomized sweep (`CS-0040.2`, `CS-0100.10`).
    static constexpr std::size_t FUZZ_ITERATIONS = 128U;
    /// @brief Fixed deterministic PRNG seed (`CS-0100.12`).
    static constexpr std::uint64_t PRNG_SEED = 0x5A391E07ULL;
    /// @brief Concurrent worker thread count for stress testing (`CS-0100.4`, `CS-0100.23`).
    static constexpr std::size_t WORKER_THREADS = 4U;
    /// @brief Operations executed per concurrency worker (`CS-0010.37`).
    static constexpr std::size_t OPS_PER_THREAD = 64U;
};

/**
 * @brief Minimal named object used to instantiate `sample::SimpleCollection<T>` across 24 types.
 * @tparam TagId Compile-time tag distinguishing template instantiations (`CS-0100.14`).
 * @task TSK-20261113-004
 * @feature FE-0060
 * @constraint CS-0090.1, CS-0090.2, CS-0100.14
 * @compliance Provides `GetName()` required by `sample::SimpleCollection<T>::at(Smp::String8)`.
 */
template <int TagId>
class TaggedNamedItem {
public:
    /**
     * @brief Constructs a tagged item with a given name.
     * @param[in] name
     *   - Justification: Identifies item during `SimpleCollection::at(String8)` lookup.
     *   - Usage: Copied into `_name`.
     *   - Consumed: Read-only C-string view.
     *   - Prerequisites: Valid non-null pointer or empty string.
     * @task TSK-20261113-004
     * @constraint CS-0090.3
     * @compliance Handles null pointer safely via conditional initialization.
     */
    explicit TaggedNamedItem(Smp::String8 name) : _name(name != nullptr ? name : "") {}

    /**
     * @brief Returns the stored item name.
     * @return Smp::String8
     *   - Justification: Satisfies `SimpleCollection<T>::at(Smp::String8)` concept requirement.
     *   - Usage: Read-only access to object identifier.
     *   - Consumed: Non-owning pointer; caller must not free.
     *   - Prerequisites: Object must outlive pointer usage.
     *   - Limitations: Null-terminated ASCII/UTF-8 C string.
     *   - Lifecycle: Tied to `TaggedNamedItem` lifetime.
     *   - Modification / Deletion: Immutable.
     *   - Specific Values: Name passed at construction.
     *   - Error Behavior: No-throw.
     * @task TSK-20261113-004
     * @constraint CS-0090.7
     * @compliance Provides noexcept non-owning view.
     */
    Smp::String8 GetName() const noexcept { return _name.c_str(); }

    /**
     * @brief Returns compile-time tag id (`CS-0100.14`).
     * @return int
     *   - Justification: Confirms distinct template specialization instantiation.
     *   - Usage: Asserted in parametrized template test loops.
     *   - Consumed: Scalar value copied by caller.
     *   - Prerequisites: None.
     *   - Limitations: Constant per specialization.
     *   - Lifecycle: Prvalue.
     *   - Modification / Deletion: Immutable.
     *   - Specific Values: Equal to `TagId`.
     *   - Error Behavior: No-throw.
     * @task TSK-20261113-004
     * @constraint CS-0090.7
     * @compliance Compile-time constant tag accessor.
     */
    static constexpr int GetTag() noexcept { return TagId; }

private:
    /// @brief Stored item name (`CS-0090.1`).
    std::string _name;
};

/**
 * @brief Minimal mock `IObject` parent for SMP sample model hierarchy verification.
 * @task TSK-20261113-004
 * @feature FE-0060
 * @constraint CS-0090.1, CS-0090.2
 * @compliance Implements `Smp::IObject` without dynamic allocations.
 */
class MockParentObject final : public virtual Smp::IObject {
public:
    /**
     * @brief Constructs parent mock with given name.
     * @param[in] name Object name (`nullptr` treated as `"Parent"`).
     * @task TSK-20261113-004
     * @constraint CS-0090.3
     * @compliance Null-safe initialization.
     */
    explicit MockParentObject(Smp::String8 name) : _name(name != nullptr ? name : "Parent") {}
    /// @brief Default virtual destructor (`CS-0060.5`).
    ~MockParentObject() noexcept override = default;

    /// @brief Returns parent name (`CS-0090.7`).
    Smp::String8 GetName() const override { return _name.c_str(); }
    /// @brief Returns static description (`CS-0090.7`).
    Smp::String8 GetDescription() const override { return "MockParent"; }
    /// @brief Root parent has no parent (`nullptr`) (`CS-0090.7`).
    Smp::IObject* GetParent() const override { return nullptr; }
    /// @brief Resolves no children (`CS-0090.7`).
    Smp::IObject* GetChild(Smp::String8 /*name*/) const override { return nullptr; }

private:
    /// @brief Parent name buffer (`CS-0090.1`).
    std::string _name;
};

/**
 * @brief Executes verification of a single `sample::SimpleCollection<TaggedNamedItem<TagId>>` instance.
 * @tparam TagId Distinct integer tag producing an independent template instantiation (`CS-0100.14`).
 * @task TSK-20261113-004
 * @feature FE-0060
 * @constraint CS-0090.1, CS-0090.2, CS-0100.14
 * @compliance Covers `Add`, `empty`, `size`, `at(name)`, `at(index)`, `begin`, `end`, and null/OOB guards.
 */
template <int TagId>
void VerifyCollectionSpecialization() {
    sample::SimpleCollection<TaggedNamedItem<TagId>> collection{};
    EXPECT_TRUE(collection.empty());
    EXPECT_EQ(collection.size(), 0U);
    EXPECT_EQ(collection.at(nullptr), nullptr);
    EXPECT_EQ(collection.at("missing"), nullptr);
    EXPECT_EQ(collection.at(std::size_t{0U}), nullptr);

    TaggedNamedItem<TagId> itemA{"alpha"};
    TaggedNamedItem<TagId> itemB{"beta"};
    collection.Add(nullptr);
    EXPECT_EQ(collection.size(), 0U);

    collection.Add(&itemA);
    collection.Add(&itemB);
    EXPECT_FALSE(collection.empty());
    EXPECT_EQ(collection.size(), 2U);
    EXPECT_EQ(collection.at("alpha"), &itemA);
    EXPECT_EQ(collection.at("beta"), &itemB);
    EXPECT_EQ(collection.at(std::size_t{1U}), &itemB);
    EXPECT_EQ(TaggedNamedItem<TagId>::GetTag(), TagId);

    std::size_t iteratedCount{0U};
    for (typename sample::SimpleCollection<TaggedNamedItem<TagId>>::const_iterator it = collection.begin();
         it != collection.end(); ++it) {
        if (iteratedCount >= 8U) {
            throw std::runtime_error("Bounded iterator limit exceeded");
        } else {
            // Valid bounded iteration step (`CS-0010.37`).
        }
        EXPECT_NE(*it, nullptr);
        ++iteratedCount;
    }
    EXPECT_EQ(iteratedCount, 2U);
}

// ---------------------------------------------------------------------------
// Progressive Pedagogical Tutorials (CS-0100.5)
// ---------------------------------------------------------------------------

/**
 * @brief Tutorial 1 (Basic): Publish and execute `InverterModel` boolean negation.
 * @task TSK-20261113-004
 * @feature FE-0020
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5
 * @compliance Demonstrates basic construction, input toggle, entry-point invocation, and output check.
 */
TEST(SampleModelsTutorialTest, Tutorial1_SimpleInverterUsage) {
    quasar::test_support::TestScopeGuard guard{"SampleModelsTutorialTest.Tutorial1_SimpleInverterUsage"};
    sim::TypeRegistry typeRegistry{};
    sim::Publication publication{&typeRegistry};
    sample::InverterModel inverter{"TutorialInverter", "Pedagogical basic inverter", nullptr};

    // Step 1: Publish SMP fields to bind `_input` and `_output`.
    inverter.Publish(&publication);
    EXPECT_EQ(static_cast<int>(inverter.GetState()), static_cast<int>(Smp::ComponentStateKind::CSK_Publishing));

    // Step 2: Set input to true and run the Execute entry point.
    inverter.SetSimpleValue("Input", Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool, true));
    Smp::IObject* execObj = inverter.GetChild("Execute");
    ASSERT_NE(execObj, nullptr);
    Smp::IEntryPoint* entry = dynamic_cast<Smp::IEntryPoint*>(execObj);
    ASSERT_NE(entry, nullptr);
    entry->Execute();

    // Step 3: Read inverted output (`!true == false`).
    Smp::AnySimple outputVal = inverter.GetSimpleValue("Output");
    EXPECT_FALSE(static_cast<Smp::Bool>(outputVal));
}

/**
 * @brief Tutorial 2 (Intermediate): Drive `DelayedSimpleModel` through its 3-step pipeline.
 * @task TSK-20261113-004
 * @feature FE-0020
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5
 * @compliance Demonstrates multi-type SMP field updates and 3-cycle shift-register observation.
 */
TEST(SampleModelsTutorialTest, Tutorial2_IntermediateDelayedPipeline) {
    quasar::test_support::TestScopeGuard guard{"SampleModelsTutorialTest.Tutorial2_IntermediateDelayedPipeline"};
    sim::TypeRegistry typeRegistry{};
    sim::Publication publication{&typeRegistry};
    sample::DelayedSimpleModel model{"TutorialDelayed", "2-step delay register", nullptr};
    model.Publish(&publication);
    model.Configure(nullptr, nullptr);
    model.Connect(nullptr);

    Smp::ISimpleField* inInt = dynamic_cast<Smp::ISimpleField*>(model.GetField("Input_Int32"));
    Smp::ISimpleField* outInt = dynamic_cast<Smp::ISimpleField*>(model.GetField("Output_Int32"));
    ASSERT_NE(inInt, nullptr);
    ASSERT_NE(outInt, nullptr);

    inInt->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int32, static_cast<Smp::Int32>(42)));
    for (std::size_t step = 0U; step < SampleModelTestConstants::DELAY_PIPELINE_STEPS; ++step) {
        model.Execute();
        quasar::test_support::AsyncTestLogger::instance().logEvent(
            "STEP", "Tutorial2_Pipeline", std::to_string(static_cast<Smp::Int32>(outInt->GetValue())),
            static_cast<std::uint64_t>(step));
    }
    EXPECT_EQ(static_cast<Smp::Int32>(outInt->GetValue()), 42);
}

/**
 * @brief Tutorial 3 (Advanced): Compose parent hierarchy, `DelayedArrayModel`, and lifecycle transitions.
 * @task TSK-20261113-004
 * @feature FE-0020, FE-0060
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.5
 * @compliance Demonstrates parent wiring, array field indexing, and full `Created->Disconnected` lifecycle.
 */
TEST(SampleModelsTutorialTest, Tutorial3_AdvancedCompositeArrayAndLifecycle) {
    quasar::test_support::TestScopeGuard guard{"SampleModelsTutorialTest.Tutorial3_AdvancedCompositeArrayAndLifecycle"};
    MockParentObject root{"MissionPayload"};
    sim::TypeRegistry typeRegistry{};
    sim::Publication publication{&typeRegistry};
    sample::DelayedArrayModel arrayModel{"TelemetryDelay", "Array transport", &root};

    EXPECT_EQ(arrayModel.GetParent(), &root);
    EXPECT_EQ(static_cast<int>(arrayModel.GetState()), static_cast<int>(Smp::ComponentStateKind::CSK_Created));

    arrayModel.Publish(&publication);
    arrayModel.Configure(nullptr, nullptr);
    arrayModel.Connect(nullptr);

    Smp::ISimpleArrayField* inArr = dynamic_cast<Smp::ISimpleArrayField*>(arrayModel.GetField("InputArray"));
    Smp::ISimpleArrayField* outArr = dynamic_cast<Smp::ISimpleArrayField*>(arrayModel.GetField("OutputArray"));
    ASSERT_NE(inArr, nullptr);
    ASSERT_NE(outArr, nullptr);

    const Smp::UInt64 arrSize = inArr->GetSize();
    for (Smp::UInt64 idx = 0U; idx < arrSize; ++idx) {
        inArr->SetValue(idx, Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int8, static_cast<Smp::Int8>(idx + 10U)));
    }
    for (std::size_t step = 0U; step < SampleModelTestConstants::DELAY_PIPELINE_STEPS; ++step) {
        arrayModel.Execute();
    }
    EXPECT_EQ(static_cast<Smp::Int8>(outArr->GetValue(0U)), 10);
    EXPECT_EQ(static_cast<Smp::Int8>(outArr->GetValue(9U)), 19);

    arrayModel.Disconnect();
    EXPECT_EQ(static_cast<int>(arrayModel.GetState()), static_cast<int>(Smp::ComponentStateKind::CSK_Disconnected));
}

// ---------------------------------------------------------------------------
// Template Coverage (24 cases) & Branch / Boundary / Fuzz Tests
// ---------------------------------------------------------------------------

/**
 * @brief Covers 24 distinct template instantiations of `sample::SimpleCollection<T>`.
 * @task TSK-20261113-004
 * @feature FE-0060
 * @constraint CS-0090.1, CS-0090.2, CS-0100.14
 * @compliance Exceeds the CS-0100.14 requirement of at least 20 template cases using tagged types and SMP interfaces.
 */
TEST(SampleModelsTemplateTest, TwentyFourCollectionInstantiations) {
    quasar::test_support::TestScopeGuard guard{"SampleModelsTemplateTest.TwentyFourCollectionInstantiations"};
    VerifyCollectionSpecialization<1>();
    VerifyCollectionSpecialization<2>();
    VerifyCollectionSpecialization<3>();
    VerifyCollectionSpecialization<4>();
    VerifyCollectionSpecialization<5>();
    VerifyCollectionSpecialization<6>();
    VerifyCollectionSpecialization<7>();
    VerifyCollectionSpecialization<8>();
    VerifyCollectionSpecialization<9>();
    VerifyCollectionSpecialization<10>();
    VerifyCollectionSpecialization<11>();
    VerifyCollectionSpecialization<12>();
    VerifyCollectionSpecialization<13>();
    VerifyCollectionSpecialization<14>();
    VerifyCollectionSpecialization<15>();
    VerifyCollectionSpecialization<16>();
    VerifyCollectionSpecialization<17>();
    VerifyCollectionSpecialization<18>();
    VerifyCollectionSpecialization<19>();
    VerifyCollectionSpecialization<20>();
    VerifyCollectionSpecialization<21>();

    sample::SimpleCollection<Smp::IField> fields{};
    sample::SimpleCollection<Smp::IComponent> components{};
    sample::SimpleCollection<Smp::IModel> models{};
    EXPECT_TRUE(fields.empty());
    EXPECT_TRUE(components.empty());
    EXPECT_TRUE(models.empty());
}

/**
 * @brief Verifies all null guards, UUIDs, fallbacks, and untouched base stubs on sample models.
 * @task TSK-20261113-004
 * @feature FE-0020
 * @constraint CS-0090.1, CS-0090.2, CS-0100.1, CS-0100.3, CS-0100.9
 * @compliance Covers `Publish(nullptr)`, unknown field/child queries, UUID uniqueness, and `SmpModel` stubs.
 */
TEST(SampleModelsCoverageTest, NullGuardsUuidsAndAllModelBranches) {
    quasar::test_support::TestScopeGuard guard{"SampleModelsCoverageTest.NullGuardsUuidsAndAllModelBranches"};

    sample::InverterModel inv{nullptr, nullptr, nullptr};
    EXPECT_STREQ(inv.GetName(), "");
    EXPECT_STREQ(inv.GetDescription(), "");
    inv.Publish(nullptr);
    EXPECT_THROW(inv.GetChild(nullptr), std::logic_error);
    EXPECT_EQ(inv.GetChild("UnknownChild"), nullptr);
    EXPECT_EQ(inv.GetContainer("None"), nullptr);
    EXPECT_NE(inv.GetContainers(), nullptr);
    EXPECT_NE(inv.GetFields(), nullptr);
    EXPECT_EQ(static_cast<int>(inv.GetSimpleValue("Missing").type), static_cast<int>(Smp::PrimitiveTypeKind::PTK_None));
    inv.SetSimpleValue("Output", Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool, true));
    inv.GetSimpleArrayValue("None", 0U, nullptr, 0U);
    inv.SetSimpleArrayValue("None", 0U, nullptr, 0U);
    EXPECT_FALSE(inv.AddChild(nullptr, nullptr));
    EXPECT_FALSE(inv.RemoveChild(nullptr, nullptr));
    EXPECT_EQ(inv.IsChildInCollection(nullptr, nullptr), nullptr);
    EXPECT_EQ(inv.GetUuid().Data1, 0x12345678U);

    sample::DelayedSimpleModel dsm{"DSM", "Delayed scalar", nullptr};
    dsm.Publish(nullptr);
    EXPECT_EQ(dsm.GetUuid().Data1, 0xAAA11111U);
    EXPECT_NE(dsm.GetChild("Execute"), nullptr);
    EXPECT_EQ(dsm.GetChild("Other"), nullptr);

    sample::DelayedArrayModel dam{"DAM", "Delayed array", nullptr};
    dam.Publish(nullptr);
    EXPECT_EQ(dam.GetUuid().Data1, 0xAAA22222U);
    EXPECT_NE(dam.GetChild("Execute"), nullptr);
    EXPECT_EQ(dam.GetChild("Other"), nullptr);
}

/**
 * @brief Performs deterministic seeded fuzzing across `DelayedSimpleModel`, `DelayedArrayModel`, and `InverterModel`.
 * @task TSK-20261113-004
 * @feature FE-0020
 * @constraint CS-0090.1, CS-0090.2, CS-0100.10, CS-0100.12
 * @compliance Uses fixed-seed `DeterministicRng` so pipeline delay verification is 100% reproducible.
 */
TEST(SampleModelsFuzzTest, DeterministicSeededPipelineFuzzing) {
    quasar::test_support::TestScopeGuard guard{"SampleModelsFuzzTest.DeterministicSeededPipelineFuzzing"};
    quasar::test_support::DeterministicRng rng{SampleModelTestConstants::PRNG_SEED};
    sim::TypeRegistry typeRegistry{};
    sim::Publication publication{&typeRegistry};

    sample::InverterModel inverter{"FuzzInv", "Fuzz inverter", nullptr};
    sample::DelayedSimpleModel scalar{"FuzzScalar", "Fuzz delayed scalar", nullptr};
    sample::DelayedArrayModel arr{"FuzzArr", "Fuzz delayed array", nullptr};
    inverter.Publish(&publication);
    scalar.Publish(&publication);
    arr.Publish(&publication);

    Smp::ISimpleField* inF64 = dynamic_cast<Smp::ISimpleField*>(scalar.GetField("Input_Float64"));
    Smp::ISimpleField* outF64 = dynamic_cast<Smp::ISimpleField*>(scalar.GetField("Output_Float64"));
    Smp::ISimpleArrayField* inArr = dynamic_cast<Smp::ISimpleArrayField*>(arr.GetField("InputArray"));
    Smp::ISimpleArrayField* outArr = dynamic_cast<Smp::ISimpleArrayField*>(arr.GetField("OutputArray"));
    ASSERT_NE(inF64, nullptr);
    ASSERT_NE(outF64, nullptr);
    ASSERT_NE(inArr, nullptr);
    ASSERT_NE(outArr, nullptr);

    Smp::Float64 hist1{0.0};
    Smp::Float64 hist2{0.0};
    Smp::Int8 arrHist1{0};
    Smp::Int8 arrHist2{0};

    for (std::size_t iter = 0U; iter < SampleModelTestConstants::FUZZ_ITERATIONS; ++iter) {
        const bool bit = rng.nextInt(0, 1) != 0;
        inverter.SetSimpleValue("Input", Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool, bit));
        inverter.Execute();
        EXPECT_EQ(static_cast<Smp::Bool>(inverter.GetSimpleValue("Output")), !bit);

        const Smp::Float64 nextF64 = rng.nextDouble(-1000.0, 1000.0);
        const Smp::Int8 nextByte = static_cast<Smp::Int8>(rng.nextInt(-100, 100));
        inF64->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Float64, nextF64));
        inArr->SetValue(0U, Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int8, nextByte));

        scalar.Execute();
        arr.Execute();

        if (iter >= 2U) {
            EXPECT_DOUBLE_EQ(static_cast<Smp::Float64>(outF64->GetValue()), hist2);
            EXPECT_EQ(static_cast<Smp::Int8>(outArr->GetValue(0U)), arrHist2);
        } else {
            // Initial pipeline fill steps before 2-step delay flushes out (`CS-0060.3`).
        }

        hist2 = hist1;
        hist1 = nextF64;
        arrHist2 = arrHist1;
        arrHist1 = nextByte;
    }
}

/**
 * @brief Exercises independent `InverterModel` instances in parallel threads for bounded duration.
 * @task TSK-20261113-004
 * @feature FE-0020
 * @constraint CS-0090.1, CS-0090.2, CS-0100.4, CS-0100.13, CS-0100.23
 * @compliance Verifies multi-threaded execution stability within explicit timing limits.
 */
TEST(SampleModelsStressTest, ParallelModelExecutionStress) {
    quasar::test_support::TestScopeGuard guard{"SampleModelsStressTest.ParallelModelExecutionStress"};
    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    std::vector<std::thread> workers{};
    workers.reserve(SampleModelTestConstants::WORKER_THREADS);
    for (std::size_t t = 0U; t < SampleModelTestConstants::WORKER_THREADS; ++t) {
        workers.emplace_back([t]() {
            sim::TypeRegistry tr{};
            sim::Publication pub{&tr};
            sample::InverterModel inv{"ConcurrentInv", "Thread-local model", nullptr};
            inv.Publish(&pub);
            for (std::size_t op = 0U; op < SampleModelTestConstants::OPS_PER_THREAD; ++op) {
                const bool input = ((op + t) & 1U) == 0U;
                inv.SetSimpleValue("Input", Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool, input));
                inv.Execute();
                EXPECT_EQ(static_cast<Smp::Bool>(inv.GetSimpleValue("Output")), !input);
            }
        });
    }
    for (std::size_t t = 0U; t < workers.size(); ++t) {
        workers[t].join();
    }

    const std::chrono::milliseconds elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    EXPECT_LT(elapsed.count(), 5000);
}

} // namespace sample::test
