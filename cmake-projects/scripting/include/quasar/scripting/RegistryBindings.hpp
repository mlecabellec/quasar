#pragma once
#include <sol/sol.hpp>

namespace quasar::scripting {

/**
 * @brief Binds the quasar::named hierarchy and registry to Lua.
 * @param lua The sol::state to bind to.
 */
void bindNamedTypes(sol::state_view lua);

} // namespace quasar::scripting
