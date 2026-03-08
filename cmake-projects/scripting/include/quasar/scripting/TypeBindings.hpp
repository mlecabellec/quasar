#pragma once
#include <sol/sol.hpp>

namespace quasar::scripting {

/**
 * @brief Binds Quasar core types (Integer, FloatingPoint, Timestamp, Duration, Unit, Quantity) to Lua.
 * @param lua The sol::state to bind to.
 */
void bindCoreTypes(sol::state& lua);

} // namespace quasar::scripting
