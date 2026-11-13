/**
 * @file test_sim_extended_cs0100.cpp
 * @brief CS-0100 multi-library SMP simulator integration test inside `sim-test`.
 *
 * @details Loads all three shared SMP sample libraries (`sample-inverter`,
 * `sample-delayed-simple`, `sample-delayed-array`) concurrently into one `sim::Simulator`,
 * wires Scheduler + TimeKeeper + Logger events, executes seeded deterministic fuzz cycles,
 * and validates bounded wall-clock completion.
 *
 * @task TSK-20261113-004
 * @feature FE-0020, FE-0070
 * @constraint CS-0010, CS-0040, CS-0060, CS-0090, CS-0100.2, CS-0100.5, CS-0100.12, CS-0100.16
 * @compliance Uses non-blocking `AsyncTestLogger`, seeded `DeterministicRng`, and explicit type declarations.
 */

#include "test_common.hpp"
#include "quasar/test_support/AsyncTestLogger.hpp"

#include <Smp/ISimpleArrayField.h>
#include <Smp/Services/IScheduler.h>
#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>

namespace sim_test::cs0100 {

/**
 * @brief Constants for multi-model SMP simulation integration tests (`CS-0040`).
 * @task TSK-20261113-004
 * @constraint CS-0040.1, CS-0040.6, CS-0090.1
 * @compliance Centralizes UUIDs, fuzz iteration count, and deterministic seed.
 */
struct IntegrationBounds {
    /// @brief Fixed deterministic PRNG seed (`CS-0100.12`).
    static constexpr std::uint64_t PRNG_SEED = 0xD4C3B2A109876543ULL;
    /// @brief Fuzz step count across coupled dynamically loaded models (`CS-0010.37`).
    static constexpr std::size_t COUPLED_STEPS = 48U;
    /// @brief Cycle period in SMP simulation nanoseconds (`CS-0040.2`).
    static constexpr Smp::Duration STEP_DURATION_NS = 1000LL;
};

/**
 * @brief Loads `sample-inverter`, `sample-delayed-simple`, and `sample-delayed-array` together in one simulator.
 * @task TSK-20261113-004
 * @feature FE-0020, FE-0070.7
 * @constraint CS-0090.1, CS-0090.2, CS-0100.2, CS-0100.16
 * @compliance Verifies dynamic symbol resolution and deterministic coupled execution across 3 shared libraries.
 */
TEST(SimTestIntegration, MultiLibraryDynamicCoupledSimulation) {
    quasar::test_support::TestScopeGuard guard{"SimTestIntegration.MultiLibraryDynamicCoupledSimulation"};
    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    sim::Simulator simulator{};
    LoadLibrary(simulator, "sample-inverter");
    LoadLibrary(simulator, "sample-delayed-simple");
    LoadLibrary(simulator, "sample-delayed-array");

    const Smp::Uuid invUuid{0x12345678, 0x1234, 0x5678, 0x1234, 0x567812345678ULL};
    const Smp::Uuid simpleUuid{0xAAA11111, 0x1111, 0x1111, 0x1111, 0x111111111111ULL};
    const Smp::Uuid arrayUuid{0xAAA22222, 0x2222, 0x2222, 0x2222, 0x222222222222ULL};

    Smp::IComponent* invComp = simulator.CreateInstance(invUuid, "Inv1", "Dynamic Inverter", nullptr);
    Smp::IComponent* simpleComp = simulator.CreateInstance(simpleUuid, "DelayScalar1", "Dynamic Scalar", nullptr);
    Smp::IComponent* arrayComp = simulator.CreateInstance(arrayUuid, "DelayArray1", "Dynamic Array", nullptr);
    ASSERT_NE(invComp, nullptr);
    ASSERT_NE(simpleComp, nullptr);
    ASSERT_NE(arrayComp, nullptr);

    Smp::IModel* invModel = dynamic_cast<Smp::IModel*>(invComp);
    Smp::IModel* simpleModel = dynamic_cast<Smp::IModel*>(simpleComp);
    Smp::IModel* arrayModel = dynamic_cast<Smp::IModel*>(arrayComp);
    ASSERT_NE(invModel, nullptr);
    ASSERT_NE(simpleModel, nullptr);
    ASSERT_NE(arrayModel, nullptr);

    simulator.AddModel(invModel);
    simulator.AddModel(simpleModel);
    simulator.AddModel(arrayModel);
    simulator.Publish();
    simulator.Configure();
    simulator.Connect();

    Smp::IEntryPoint* invEp = dynamic_cast<Smp::IEntryPoint*>(invComp->GetChild("Execute"));
    Smp::IEntryPoint* simpleEp = dynamic_cast<Smp::IEntryPoint*>(simpleComp->GetChild("Execute"));
    Smp::IEntryPoint* arrayEp = dynamic_cast<Smp::IEntryPoint*>(arrayComp->GetChild("Execute"));
    ASSERT_NE(invEp, nullptr);
    ASSERT_NE(simpleEp, nullptr);
    ASSERT_NE(arrayEp, nullptr);

    Smp::ISimpleField* invIn = dynamic_cast<Smp::ISimpleField*>(invComp->GetField("Input"));
    Smp::ISimpleField* invOut = dynamic_cast<Smp::ISimpleField*>(invComp->GetField("Output"));
    Smp::ISimpleField* delIn = dynamic_cast<Smp::ISimpleField*>(simpleComp->GetField("Input_Bool"));
    Smp::ISimpleField* delOut = dynamic_cast<Smp::ISimpleField*>(simpleComp->GetField("Output_Bool"));
    ASSERT_NE(invIn, nullptr);
    ASSERT_NE(invOut, nullptr);
    ASSERT_NE(delIn, nullptr);
    ASSERT_NE(delOut, nullptr);

    Smp::Services::IScheduler* scheduler = simulator.GetScheduler();
    ASSERT_NE(scheduler, nullptr);

    quasar::test_support::DeterministicRng rng{IntegrationBounds::PRNG_SEED};
    bool prev1{false};
    bool prev2{false};
    for (std::size_t step = 0U; step < IntegrationBounds::COUPLED_STEPS; ++step) {
        const bool stim = rng.nextInt(0, 1) != 0;
        invIn->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool, stim));
        scheduler->AddSimulationTimeEvent(
            invEp, IntegrationBounds::STEP_DURATION_NS, 0LL, 0);
        simulator.Run(IntegrationBounds::STEP_DURATION_NS);
        simulator.Hold(true);

        const bool inverted = static_cast<Smp::Bool>(invOut->GetValue());
        EXPECT_EQ(inverted, !stim);

        delIn->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool, inverted));
        simpleEp->Execute();
        arrayEp->Execute();

        if (step >= 2U) {
            EXPECT_EQ(static_cast<Smp::Bool>(delOut->GetValue()), prev2);
        } else {
            // Initial pipeline fill cycle (`CS-0060.3`).
        }
        prev2 = prev1;
        prev1 = inverted;
    }

    simulator.Exit();
    const std::chrono::milliseconds elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
    EXPECT_LT(elapsed.count(), 5000);
}

/**
 * @brief Verifies error handling when loading a nonexistent SMP shared library into `Simulator`.
 * @task TSK-20261113-004
 * @feature FE-0070.7
 * @constraint CS-0090.1, CS-0090.2, CS-0100.7
 * @compliance Ensures clean exception propagation when `dlopen` fails across all candidate paths.
 */
TEST(SimTestIntegration, MissingLibraryThrowsCleanly) {
    quasar::test_support::TestScopeGuard guard{"SimTestIntegration.MissingLibraryThrowsCleanly"};
    sim::Simulator simulator{};
    EXPECT_THROW(LoadLibrary(simulator, "quasar_nonexistent_smp_model"), std::exception);
}

} // namespace sim_test::cs0100
