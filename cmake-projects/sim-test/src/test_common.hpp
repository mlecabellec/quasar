/**
 * @file test_common.hpp
 * @brief Shared SMP simulation test harness utilities for library loading and lifecycle setup.
 *
 * @details Provides inline helper functions to load SMP model shared libraries across
 * build-tree relative paths and drive standard SMP component lifecycle transitions
 * (Publishing, Configuring, Connecting) before behavioral verification.
 *
 * @task TSK-20261113-003
 * @feature FE-0020
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8
 * @compliance Documents all helper signatures, parameters, return contracts, error containment,
 * and control structures while preserving unmodified test harness logic.
 */

#ifndef TEST_COMMON_HPP
#define TEST_COMMON_HPP

// SMP standard component, model, simulator, field, and entry-point interfaces.
#include <Smp/IComponent.h>
#include <Smp/IModel.h>
#include <Smp/ISimulator.h>
#include <Smp/ISimpleField.h>
#include <Smp/IEntryPoint.h>
// Quasar simulator implementation and standard C++ I/O, string, and exception headers.
#include <sim/Simulator.hpp>
#include <iostream>
#include <string>
#include <stdexcept>

/**
 * @brief Load an SMP model shared library into the given simulator instance.
 *
 * @details Probes candidate relative shared-library paths (`../../lib/`, `lib/`, and local
 * working directory) sequentially until `sim::Simulator::LoadLibrary` succeeds or the final
 * attempt propagates an exception.
 *
 * @task TSK-20261113-003
 * @feature FE-0020
 * @constraint CS-0010.20, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9
 * @exposed
 * @compliance Uses bounded three-stage fallback exception containment and documents all
 * parameters, failure modes, and preconditions without altering executable logic.
 *
 * @param[in,out] simulator
 *   Justification: Supplies the target SMP simulator registry that hosts the loaded library.
 *   Usage: Invoked via `LoadLibrary` across up to three fallback path candidates.
 *   Consumed: Mutated in-place (registers loaded module factories on success).
 *   Prerequisites: Valid initialized `sim::Simulator` reference ready to load libraries.
 * @param[in] libName
 *   Justification: Identifies the logical base name of the model library without `lib`/`.so`.
 *   Usage: Concatenated into candidate shared-object filenames during probe attempts.
 *   Consumed: No (read-only const reference).
 *   Prerequisites: Non-empty library stem matching an accessible `lib<libName>.so` artifact.
 *
 * @pre !libName.empty()
 * @post Requested shared library factories are registered in simulator on normal return.
 * @throws std::exception [Hardware / Bus / I/O] Propagated from the final `LoadLibrary` call if all three path probes fail.
 * @error
 *   - Hardware / Bus / I/O (shared library missing or unloadable at `../../lib/` and `lib/`): Caught internally and retried against subsequent path candidates; no partial registration occurs.
 *   - Hardware / Bus / I/O (final local path `lib<libName>.so` fails): Rethrows simulator exception to caller after diagnostic output; simulator remains without the library.
 * @containment
 *   Approach: Nested `try`/`catch (...)` blocks implement a bounded 3-attempt path fallback
 *   to tolerate varying CTest working directories without aborting early.
 *   Limitations: Catches all exceptions blindly (`...`) on the first two probes, masking
 *   non-path-related failures until the third attempt; requires caller handling if all paths fail.
 */
inline void LoadLibrary(sim::Simulator& simulator, const std::string& libName) {
    // Construct primary build-tree relative library path.
    std::string libPath = "../../lib/lib" + libName + ".so";
    std::cout << "Loading library: " << libPath << std::endl;
    // Attempt loading from primary path; fall back to secondary build layouts on failure.
    try {
        simulator.LoadLibrary(const_cast<char*>(libPath.c_str()));
    } catch (...) {
        // Primary relative path failed; retry under local `lib/` directory.
        libPath = "lib/lib" + libName + ".so";
        std::cout << "Retrying with: " << libPath << std::endl;
        // Attempt secondary path and fall back once more to current working directory.
        try {
            simulator.LoadLibrary(const_cast<char*>(libPath.c_str()));
        } catch (...) {
            // Final bounded fallback probe in current working directory (propagates on failure).
            libPath = "lib" + libName + ".so";
            std::cout << "Final retry with: " << libPath << std::endl;
            simulator.LoadLibrary(const_cast<char*>(libPath.c_str()));
        }
    }
}

/**
 * @brief Instantiate an SMP model by UUID, register it, and advance its lifecycle to Connected.
 *
 * @details Calls `CreateInstance`, validates the `Smp::IModel` interface via `dynamic_cast`,
 * transfers the model to `simulator.AddModel`, and steps the simulator through `Publish`,
 * `Configure`, and `Connect` states.
 *
 * @task TSK-20261113-003
 * @feature FE-0020
 * @constraint CS-0010.15, CS-0010.20, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
 * @exposed
 * @compliance Enforces explicit null/type validation before use and documents full parameter,
 * return, error, and containment contracts.
 *
 * @param[in,out] simulator
 *   Justification: Hosts factory instantiation, model ownership, and lifecycle transitions.
 *   Usage: Invoked to create the component, register the model, and drive state transitions.
 *   Consumed: Mutated in-place (retains model ownership and advances simulator state).
 *   Prerequisites: Library containing `uuid` must already be loaded in `simulator`.
 * @param[in] uuid
 *   Justification: Uniquely identifies the SMP component factory to instantiate.
 *   Usage: Passed by value to `simulator.CreateInstance`.
 *   Consumed: Yes (passed by value; trivial POD copy).
 *   Prerequisites: Valid registered SMP component UUID.
 * @param[in] name
 *   Justification: Assigns a diagnostic instance name to the created SMP component.
 *   Usage: Passed via `c_str()` to `CreateInstance` as the component name.
 *   Consumed: No (read-only const reference).
 *   Prerequisites: Non-empty valid SMP identifier string.
 *
 * @pre !name.empty()
 * @post Returned `Smp::IComponent*` is non-null, registered in `simulator`, and connected.
 * @return Smp::IComponent*
 *   Justification: Grants test callers direct SMP field and entry-point lookup access.
 *   Usage: Caller inspects fields (`GetField`) and child entry points (`GetChild`) without freeing.
 *   Consumed: Borrowed non-owning pointer; ownership remains with `simulator` container.
 *   Prerequisites: Returned pointer is guaranteed non-null on normal return.
 *   Limitations: Valid only for the lifetime of `simulator` or until `Exit`/destruction.
 *   Lifecycle: Owned by `simulator` from `AddModel` until simulator teardown.
 *   Modification / Deletion: Caller may invoke non-owning SMP inspection/field methods; MUST NOT `delete`.
 *   Specific Values: Non-null `Smp::IComponent*` implementing `Smp::IModel`.
 *   Error Behavior: Never returns `nullptr`; throws `std::runtime_error` on creation or cast failure.
 * @throws std::runtime_error [Validation / Contract] Thrown if `CreateInstance` returns null or `dynamic_cast<Smp::IModel*>` fails.
 * @error
 *   - Validation / Contract (factory returns null or component is not `Smp::IModel`): Throws `std::runtime_error` before `AddModel`; `simulator` model list remains unchanged.
 *   - Resource / Fatal (lifecycle transition throws inside `Publish`/`Configure`/`Connect`): Exception propagates to caller; `simulator` retains partial model state until destruction.
 * @containment
 *   Approach: Explicit null-pointer and `dynamic_cast` validation guards precede `AddModel`
 *   and lifecycle progression, failing fast before invalid pointers enter the simulator tree.
 *   Limitations: Does not roll back `AddModel` if `Publish`, `Configure`, or `Connect` throws.
 */
inline Smp::IComponent* CreateAndInitModel(sim::Simulator& simulator, Smp::Uuid uuid, const std::string& name) {
    // Instantiate the component via registered factory without parent composite.
    Smp::IComponent* component = simulator.CreateInstance(uuid, name.c_str(), "Test Instance", nullptr);
    // Reject null factory output before downcasting to Smp::IModel [CS-0010.15].
    if (!component) throw std::runtime_error("Failed to create instance");

    // Verify created component implements Smp::IModel prior to simulator registration.
    Smp::IModel* model = dynamic_cast<Smp::IModel*>(component);
    // Abort with diagnostic exception if component does not satisfy IModel contract.
    if (!model) throw std::runtime_error("Component is not an IModel");
    simulator.AddModel(model);

    // Advance simulator and model through standard SMP initialization states.
    std::cout << "Lifecycle: Publishing..." << std::endl;
    simulator.Publish();
    std::cout << "Lifecycle: Configuring..." << std::endl;
    simulator.Configure();
    // Connect component ports and finalize initialization.
    std::cout << "Lifecycle: Connecting..." << std::endl;
    simulator.Connect();

    // Return borrowed handle for subsequent field/entry-point verification.
    return component;
}

#endif
