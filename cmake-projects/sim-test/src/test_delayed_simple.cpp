/**
 * @file test_delayed_simple.cpp
 * @brief Integration test verifying 2-step delay behavior on `sample-delayed-simple` scalar SMP fields.
 *
 * @details Loads `sample-delayed-simple`, resolves `Input_Int32`, `Output_Int32`, `Input_Float64`,
 * `Output_Float64`, and `Execute`, drives 6 bounded input steps, and verifies 2-step delayed outputs
 * prior to `simulator.Exit()`.
 *
 * @task TSK-20261113-003
 * @feature FE-0020
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8
 * @compliance Adds complete Doxygen file/function contracts, control-structure comments,
 * and CS-0010.44 body comments without modifying executable statements.
 */

// Shared SMP test harness utilities, console I/O, and vector containers.
#include "test_common.hpp"
#include <iostream>
#include <vector>

/**
 * @brief Entry point validating the 2-step delayed scalar SMP model.
 *
 * @details Creates and initializes `DelayedSimple`, drives a 6-element input sequence through
 * `Int32` and `Float64` simple fields, checks 2-step delay equivalence on every iteration,
 * and transitions the simulator to Exit state.
 *
 * @task TSK-20261113-003
 * @feature FE-0020
 * @constraint CS-0010.15, CS-0010.20, CS-0010.37, CS-0090.1, CS-0090.2, CS-0090.5, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
 * @compliance Documents bounded loop iteration (`inputs.size()` == 6), null/type checks,
 * exception containment, and full return value contract.
 *
 * @post Simulator exits cleanly or unwinds via RAII on error.
 * @return int
 *   Justification: Communicates test pass/fail result to CTest harness.
 *   Usage: Returned directly from `main` to C runtime exit handler.
 *   Consumed: Scalar exit code copied to OS process status.
 *   Prerequisites: `sample-delayed-simple` library must be reachable via `LoadLibrary`.
 *   Limitations: Truncates `Float64` output to `int` when comparing against integer `expected_outputs`.
 *   Lifecycle: Ephemeral scalar exit status at process termination.
 *   Modification / Deletion: Immutable prvalue returned to OS; no caller cleanup needed.
 *   Specific Values: 0 (all 6 delayed steps matched); 1 (missing interface, mismatch, or caught exception).
 *   Error Behavior: Catches `std::exception`, writes error message to `std::cerr`, and returns 1.
 * @retval 0 DelayedSimple test passed all 6 steps and exited cleanly.
 * @retval 1 Field/entry-point lookup failed, delayed output mismatched, or exception caught.
 * @throws std::exception [Resource / Fatal] Caught internally; non-`std::exception` throws terminate process.
 * @error
 *   - Hardware / Bus / I/O (`LoadLibrary` failure): Caught by `catch (const std::exception&)`; logs error and returns 1.
 *   - Validation / Contract (null fields/entry point or delayed value mismatch): Logs error and returns 1.
 * @containment
 *   Approach: Top-level `try`/`catch (const std::exception&)` isolates setup and execution faults,
 *   while `sim::Simulator` RAII destructor frees registered models on unwind.
 *   Limitations: Non-`std::exception` throws are unhandled; `simulator.Exit()` is skipped on early failure returns.
 */
int main() {
    // Isolate simulator initialization, model execution, and teardown errors [CS-0010.20].
    try {
        // Instantiate simulator and load the delayed simple scalar model library.
        sim::Simulator simulator;
        LoadLibrary(simulator, "sample-delayed-simple");

        // Instantiate and drive DelayedSimple component to Connected state.
        Smp::Uuid delayedUuid = {0xAAA11111, 0x1111, 0x1111, 0x1111, 0x111111111111};
        Smp::IComponent* component = CreateAndInitModel(simulator, delayedUuid, "DelayedSimple");

        // Resolve Int32/Float64 input and output fields plus Execute entry point.
        Smp::ISimpleField* inInt32 = dynamic_cast<Smp::ISimpleField*>(component->GetField("Input_Int32"));
        Smp::ISimpleField* outInt32 = dynamic_cast<Smp::ISimpleField*>(component->GetField("Output_Int32"));
        Smp::ISimpleField* inFloat64 = dynamic_cast<Smp::ISimpleField*>(component->GetField("Input_Float64"));
        Smp::ISimpleField* outFloat64 = dynamic_cast<Smp::ISimpleField*>(component->GetField("Output_Float64"));
        Smp::IEntryPoint* ep = dynamic_cast<Smp::IEntryPoint*>(component->GetChild("Execute"));

        // Abort before dereferencing if any field or entry-point pointer is null [CS-0010.15].
        if (!inInt32 || !outInt32 || !inFloat64 || !outFloat64 || !ep) {
            std::cerr << "Fields or entry point not found!" << std::endl;
            return 1;
        }

        // Define deterministic 6-step stimulus and expected 2-step delayed output vectors.
        std::vector<int> inputs = {10, 20, 30, 40, 50, 60};
        std::vector<int> expected_outputs = {0, 0, 10, 20, 30, 40}; // 2-step delay

        // Step over fixed-length `inputs` sequence (hard bound = 6 iterations) [CS-0010.37].
        for (size_t i = 0; i < inputs.size(); ++i) {
            // Apply synchronized Int32 and Float64 stimulus before executing one tick.
            inInt32->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int32, inputs[i]));
            inFloat64->SetValue(Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Float64, (double)inputs[i]));
            
            ep->Execute();

            // Read back Int32 and Float64 outputs produced by the model step.
            int outValInt = (int)outInt32->GetValue();
            double outValFloat = (double)outFloat64->GetValue();
            
            std::cout << "Iteration " << i << ": Input=" << inputs[i] << ", OutputInt=" << outValInt << ", OutputFloat=" << outValFloat << std::endl;

            // Fail immediately if either scalar output diverges from the 2-step delayed expectation.
            if (outValInt != expected_outputs[i] || (int)outValFloat != expected_outputs[i]) {
                std::cerr << "Test failed at iteration " << i << ". Expected " << expected_outputs[i] << " but got " << outValInt << std::endl;
                return 1;
            }
        }

        // Transition simulator to Exit state after all 6 steps pass.
        std::cout << "Lifecycle: Exiting..." << std::endl;
        simulator.Exit();
        std::cout << "DelayedSimple Test Passed!" << std::endl;
    } catch (const std::exception& e) {
        // Log caught exception details and signal failure to test runner.
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    // All checks succeeded cleanly.
    return 0;
}
