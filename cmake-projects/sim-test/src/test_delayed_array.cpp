/**
 * @file test_delayed_array.cpp
 * @brief Integration test verifying 2-step delay behavior on `sample-delayed-array` SMP model.
 *
 * @details Loads `sample-delayed-array` dynamically, queries `InputArray`, `OutputArray`,
 * and `Execute`, drives 5 bounded simulation steps with synthetic `Int8` vectors, and checks
 * element-wise 2-step delay invariants before calling `simulator.Exit()`.
 *
 * @task TSK-20261113-003
 * @feature FE-0020
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8
 * @compliance Provides complete Doxygen header/entry documentation, bounded loop annotations,
 * control-flow comments, and exception containment analysis.
 */

// Shared SMP test harness helpers, array field interface, I/O, and vector storage.
#include "test_common.hpp"
#include <Smp/ISimpleArrayField.h>
#include <iostream>
#include <vector>

/**
 * @brief Entry point validating the 2-step delayed array SMP model.
 *
 * @details Initializes the simulator and `DelayedArray` component via `CreateAndInitModel`,
 * executes 5 bounded steps feeding `PTK_Int8` arrays, compares retrieved outputs against
 * two-step shift registers (`expected_mem1`, `expected_mem2`), and exits cleanly.
 *
 * @task TSK-20261113-003
 * @feature FE-0020
 * @constraint CS-0010.15, CS-0010.20, CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
 * @compliance Documents bounded loop counts (5 outer iterations and `size` inner elements),
 * `try`/`catch` containment, null guards, and full return semantics.
 *
 * @post Simulator exits cleanly or unwinds via RAII on error.
 * @return int
 *   Justification: Reports test execution pass/fail status to the CTest runner.
 *   Usage: Returned directly from `main` to the process exit handler.
 *   Consumed: Scalar integer copied to OS environment.
 *   Prerequisites: `sample-delayed-array` shared library must be discoverable.
 *   Limitations: Hardcoded 5-step verification horizon assumes 2-step delay model semantics.
 *   Lifecycle: Ephemeral scalar exit status at process termination.
 *   Modification / Deletion: Immutable prvalue returned to OS; no caller cleanup needed.
 *   Specific Values: 0 (all 5 steps matched 2-step delay expectation); 1 (missing interface, value mismatch, or caught exception).
 *   Error Behavior: Catches `std::exception`, prints diagnostic to `std::cerr`, and returns 1.
 * @retval 0 DelayedArray test passed all 5 iterations and exited cleanly.
 * @retval 1 Field/entry-point lookup failed, output mismatch occurred, or `std::exception` was caught.
 * @throws std::exception [Resource / Fatal] Caught internally; non-`std::exception` throws terminate process.
 * @error
 *   - Hardware / Bus / I/O (`LoadLibrary` failure): Caught by `catch (const std::exception&)`; logs error and returns 1.
 *   - Validation / Contract (null fields/entry point or delayed output mismatch): Logs error and returns 1.
 * @containment
 *   Approach: Outer `try`/`catch (const std::exception&)` block contains all setup, execution,
 *   and teardown failures while RAII destroys `sim::Simulator` on stack unwind.
 *   Limitations: Non-`std::exception` throws are not caught; `simulator.Exit()` is skipped on early failure returns.
 */
int main() {
    // Contain simulator setup, execution, and teardown exceptions cleanly [CS-0010.20].
    try {
        // Instantiate simulator and load the delayed array model library.
        sim::Simulator simulator;
        LoadLibrary(simulator, "sample-delayed-array");

        // Instantiate and initialize DelayedArray component through Connected state.
        Smp::Uuid delayedUuid = {0xAAA22222, 0x2222, 0x2222, 0x2222, 0x222222222222};
        Smp::IComponent* component = CreateAndInitModel(simulator, delayedUuid, "DelayedArray");

        // Resolve array fields and execution entry point via dynamic SMP interfaces.
        Smp::ISimpleArrayField* inArray = dynamic_cast<Smp::ISimpleArrayField*>(component->GetField("InputArray"));
        Smp::ISimpleArrayField* outArray = dynamic_cast<Smp::ISimpleArrayField*>(component->GetField("OutputArray"));
        Smp::IEntryPoint* ep = dynamic_cast<Smp::IEntryPoint*>(component->GetChild("Execute"));

        // Abort before dereferencing if any required SMP field or entry point is missing [CS-0010.15].
        if (!inArray || !outArray || !ep) {
            std::cerr << "Fields or entry point not found!" << std::endl;
            return 1;
        }

        // Allocate two-step delay reference buffers matching model array cardinality.
        size_t size = inArray->GetSize();
        std::vector<Smp::Int8> expected_mem1(size, 0);
        std::vector<Smp::Int8> expected_mem2(size, 0);

        // Execute bounded 5-step simulation loop to observe zero-init and delayed propagation [CS-0010.37].
        for (int i = 0; i < 5; ++i) {
            // Prepare per-step input vectors sized to the array field length.
            std::vector<Smp::AnySimple> input_anys(size);
            std::vector<Smp::Int8> current_input(size);
            // Populate each element up to hard bound `size` with deterministic Int8 test pattern.
            for (size_t j = 0; j < size; ++j) {
                current_input[j] = (Smp::Int8)(i * 10 + j);
                input_anys[j] = Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int8, current_input[j]);
            }
            
            // Write input array values and trigger one simulation execution step.
            inArray->SetValues(size, input_anys.data());
            
            ep->Execute();

            // Read back output array values for comparison against 2-step shift register.
            std::vector<Smp::AnySimple> output_anys(size);
            outArray->GetValues(size, output_anys.data());

            std::cout << "Iteration " << i << ": Testing 2-step delay." << std::endl;

            // Verify every element up to `size` matches expected 2-step delayed value.
            for (size_t j = 0; j < size; ++j) {
                Smp::Int8 outVal = (Smp::Int8)output_anys[j];
                // Fail immediately if actual delayed element diverges from reference pipeline.
                if (outVal != expected_mem2[j]) {
                    std::cerr << "Test failed at iteration " << i << " element " << j 
                              << ". Expected " << (int)expected_mem2[j] << " but got " << (int)outVal << std::endl;
                    return 1;
                }
            }

            // Advance two-stage software delay pipeline for the next iteration.
            expected_mem2 = expected_mem1;
            expected_mem1 = current_input;
        }

        // Transition simulator to Exit state after all step invariants pass.
        std::cout << "Lifecycle: Exiting..." << std::endl;
        simulator.Exit();
        std::cout << "DelayedArray Test Passed!" << std::endl;
    } catch (const std::exception& e) {
        // Report unexpected runtime or setup exception and return non-zero failure status.
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    // All checks succeeded cleanly.
    return 0;
}
