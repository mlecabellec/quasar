#ifndef QUASAR_CALIBRATION_LUABINDINGS_HPP
#define QUASAR_CALIBRATION_LUABINDINGS_HPP

#include <sol/sol.hpp>

namespace quasar::calibration {

/**
 * @brief Binds calibration types to a given Lua state.
 * @param lua The Lua state view.
 */
void bindCalibrationTypes(sol::state_view lua);

} // namespace quasar::calibration

#endif // QUASAR_CALIBRATION_LUABINDINGS_HPP
