/**
 * @file test_inverter.cpp
 * @brief Integration test verifying `sample-inverter` dynamic loading and boolean inversion across 5 steps.
 *
 * @details Uses `test_common.hpp` helpers to load `sample-inverter`, initialize the component
 * to `Connected`, alternate boolean `Input` values over 5 iterations, verify inverted `Output`,
 * and call `simulator.Exit()`.
 *
 * @task TSK-20261113-003
 * @feature FE-0020
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8
 * @compliance Documents test entry point, bounded iteration, null validation, error handling,
 * and return contracts without changing executable statements.
 */

// Shared SMP test harness helpers and standard console I/O.
#include "test_common.hpp"
#include <iostream>

/**
 * @brief Entry point validating boolean inversion on `sample-inverter`.
 *
 * @details Instantiates the `Inverter` component via factory UUID, checks `Input`/`Output`
 * fields and `Execute` entry point, runs 5 alternating boolean test steps, and exits cleanly.
 *
 * @task TSK-20261113-003
 * @feature FE-0020
 * @constraint CS-0010.15, CS-0010.20, CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
 * @compliance Enforces bounded 5-step verification loop, explicit null checks, `try`/`catch`
 * error containment, and full CS-0090 return documentation.
 *
 * @post Simulator exits cleanly or unwinds via RAII on error.
 * @return int
 *   Justification: Reports test pass/fail status to CTest runner.
 *   Usage: Returned directly from `main` to OS exit handler.
 *   Consumed: Scalar integer copied to C runtime environment.
 *   Prerequisites: `sample-inverter` library must be reachable via `LoadLibrary`.
 *   Limitations: Exercises 5 alternating boolean states without stress concurrency.
 *   Lifecycle: Ephemeral scalar exit status at process termination.
 *   Modification / Deletion: Immutable prvalue returned to OS; no caller cleanup needed.
 *   Specific Values: 0 (all 5 inversion checks passed); 1 (missing interface, inversion failure, or caught exception).
 *   Error Behavior: Catches `std::exception`, prints diagnostic to `std::cerr`, and returns 1.
 * @retval 0 Inverter test passed all 5 iterations and exited cleanly.
 * @retval 1 Field/entry-point lookup failed, output matched input, or exception caught.
 * @throws std::exception [Resource / Fatal] Caught internally; non-`std::exception` throws terminate process.
 * @error
 *   - Hardware / Bus / I/O (`LoadLibrary` failure): Caught by `catch (const std::exception&)`; logs error and returns 1.
 *   - Validation / Contract (null fields/entry point or non-inverted output): Logs error and returns 1.
 * @containment
 *   Approach: Outer `try`/`catch (const std::exception&)` block captures all standard exceptions
 *   and ensures RAII cleanup of `sim::Simulator`.
 *   Limitations: Non-`std::exception` throws are unhandled; `simulator.Exit()` is skipped on early failure returns.
 */
int main() {
    // Isolate simulator setup, model execution, and teardown exceptions [CS-0010.20].
    try {
        // Instantiate simulator and load the sample-inverter model library.
        sim::Simulator simulator;
        LoadLibrary(simulator, "sample-inverter");

        // Instantiate and drive Inverter component to Connected state.
        Smp::Uuid inverterUuid = {0x12345678, 0x1234, 0x5678, 0x1234, 0x567812345678};
        Smp::IComponent* component = CreateAndInitModel(simulator, inverterUuid, "Inverter");

        // Resolve boolean Input/Output simple fields and Execute entry point.
        Smp::ISimpleField* input = dynamic_cast<Smp::ISimpleField*>(component->GetField("Input"));
        Smp::ISimpleField* output = dynamic_cast<Smp::ISimpleField*>(component->GetField("Output"));
        Smp::IEntryPoint* ep = dynamic_cast<Smp::IEntryPoint*>(component->GetChild("Execute"));

        // Abort before dereferencing if any field or entry-point pointer is null [CS-0010.15].
        if (!input || !output || !ep) {
            std::cerr << "Fields or entry point not found!" << std::endl;
            return 1;
        }

        // Execute bounded 5-step alternating boolean inversion test [CS-0010.37].
        for (int i = 0; i < 5; ++i) {
            // Alternate testVal (true on even steps, false on odd) and execute model tick.
            bool testVal = (i % 2 == 0);
            input->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Bool, testVal));
            ep->Execute();
            // Retrieve output and fail if it did not invert the input boolean.
            bool result = (bool)output->GetValue();
            std::cout << "Iteration " << i << ": Input=" << testVal << ", Output=" << result << std::endl;
            // Verify logical negation invariant: output must differ from input.
            if (result == testVal) {
                std::cerr << "Test failed at iteration " << i << std::endl;
                return 1;
            }
        }

        // Transition simulator to Exit state after all 5 steps pass.
        std::cout << "Lifecycle: Exiting..." << std::endl;
        simulator.Exit();
        std::cout << "Inverter Test Passed!" << std::endl;
    } catch (const std::exception& e) {
        // Log caught exception details and signal failure to test runner.
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    // All checks succeeded cleanly.
    return 0;
}
