#include <sol/sol.hpp>
#include <memory>

namespace quasar::named {
class NamedObject;
}

namespace quasar::scripting {

class LuaService;

/**
 * @brief Binds the quasar::named hierarchy and registry to Lua.
 * @param lua The sol::state to bind to.
 * @param service The host LuaService.
 */
void bindNamedTypes(sol::state_view lua, std::shared_ptr<LuaService> service);

/**
 * @brief Extracts a NamedObject from a Lua object (proxy or scriptable).
 * @param obj The Lua object.
 * @return Shared pointer to the NamedObject, or nullptr.
 */
std::shared_ptr<quasar::named::NamedObject> extractNamedObject(sol::object obj);

/**
 * @brief Retrieves the unique engine ID from a Lua state context.
 * @param L The current Lua state.
 * @return The ID of the managed LuaEngine, or 0 if unmanaged.
 */
size_t getEngineId(sol::this_state L);

} // namespace quasar::scripting
