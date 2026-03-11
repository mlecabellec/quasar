#include <sol/sol.hpp>
#include <memory>

namespace quasar::scripting {

class LuaService;

/**
 * @brief Binds the quasar::named hierarchy and registry to Lua.
 * @param lua The sol::state to bind to.
 * @param service The host LuaService.
 */
void bindNamedTypes(sol::state_view lua, std::shared_ptr<LuaService> service);

} // namespace quasar::scripting
