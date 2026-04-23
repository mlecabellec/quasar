#ifndef QUASAR_SCRIPTING_LUAOBSERVER_HPP
#define QUASAR_SCRIPTING_LUAOBSERVER_HPP

#include "quasar/named/IObserver.hpp"
#include <sol/sol.hpp>
#include <memory>
#include <mutex>

namespace quasar::scripting {

class LuaEngine;
class LuaService;

/**
 * @class LuaObserver
 * @brief An implementation of IObserver that forwards notifications to a Lua function.
 * 
 * It handles thread-safe marshalling of events from C++ threads into the Lua state.
 */
class LuaObserver : public quasar::named::IObserver {
public:
    /**
     * @brief Constructor.
     * @param func The Lua function to call.
     * @param engine The LuaEngine associated with the function.
     * @param service Optional LuaService to post tasks to (for asynchronous handling).
     */
    LuaObserver(sol::function func, LuaEngine* engine, std::shared_ptr<LuaService> service = nullptr);

    virtual ~LuaObserver();

    /**
     * @brief Notifies the Lua function of an event.
     * @param eventData The event payload.
     */
    void notify(std::shared_ptr<quasar::named::NamedObject> eventData) override;

    /**
     * @brief Invalidates the observer, abandoning the Lua function reference.
     */
    void invalidate();

protected:
    sol::function m_func;
    LuaEngine* m_engine;
    std::weak_ptr<LuaService> m_service;
    bool m_valid{true};
    std::recursive_mutex m_mutex;
};

} // namespace quasar::scripting

#endif // QUASAR_SCRIPTING_LUAOBSERVER_HPP
