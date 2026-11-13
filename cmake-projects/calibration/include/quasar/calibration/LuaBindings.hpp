/**
 * @file LuaBindings.hpp
 * @brief Lua scripting bindings for Quasar calibration objects and interfaces.
 * @details Declares the registration entry point exposing ICalibration, NamedLinearCalibration,
 * NamedPolynomialCalibration, and CompositeCalibration usertypes into a sol2 Lua state.
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @compliance Exposes scriptable calibration wrappers via clean Lua usertype bindings without
 * altering C++ ownership semantics (CS-0090.1, CS-0090.2).
 */
#ifndef QUASAR_CALIBRATION_LUABINDINGS_HPP
#define QUASAR_CALIBRATION_LUABINDINGS_HPP

// Include sol2 C++-to-Lua binding library header.
#include <sol/sol.hpp>

/**
 * @brief Root namespace for Quasar engineering-unit calibration types and bindings.
 * @details Groups polymorphic calibration interfaces, named calibration tree nodes,
 * lookup managers, and scripting engine adapters.
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @compliance Encloses all calibration declarations inside a dedicated namespace (CS-0010.33).
 */
namespace quasar::calibration {

/**
 * @brief Binds calibration types to a given Lua state.
 * @details Registers usertype metatables for ICalibration, NamedLinearCalibration,
 * NamedPolynomialCalibration, and CompositeCalibration so Lua scripts can inspect and invoke
 * engineering-unit conversions on calibration instances.
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @exposed Public scripting API invoked when initializing calibration-aware Lua states.
 * @compliance Documents parameter contracts, exception behavior, and containment for sol2
 * usertype registration (CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.8, CS-0090.9).
 * @param[in,out] lua
 *   Justification: Target Lua VM view receiving calibration usertype definitions.
 *   Usage: Registers global usertype tables via sol::state_view::new_usertype.
 *   Consumed: Mutates the referenced Lua state registry by inserting metatables; non-owning view.
 *   Prerequisites: Must refer to an initialized, valid sol::state_view before calling.
 * @pre lua references a live Lua state whose standard tables are open.
 * @post Global metatables for ICalibration, NamedLinearCalibration, NamedPolynomialCalibration,
 * and CompositeCalibration are registered in lua.
 * @throws sol::error or std::exception if Lua state allocation or usertype registration fails.
 * @containment
 *   Approach: Registers usertypes directly into the caller-provided sol::state_view without
 *   retaining static or global state outside the VM.
 *   Limitations: Partial registration may remain in the Lua state if an allocation exception
 *   interrupts the registration sequence.
 */
void bindCalibrationTypes(sol::state_view lua);

} // namespace quasar::calibration

#endif // QUASAR_CALIBRATION_LUABINDINGS_HPP
