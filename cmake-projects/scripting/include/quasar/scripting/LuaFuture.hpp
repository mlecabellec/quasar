#pragma once

#include "quasar/scripting/LuaEngine.hpp"
#include <sol/sol.hpp>
#include <future>
#include <memory>

namespace quasar::scripting {

/**
 * @brief Represents the result of an asynchronous Lua execution.
 * 
 * LuaFuture owns the LuaEngine used for execution to ensure that the
 * sol::protected_function_result (which depends on the Lua state)
 * remains valid until consumed.
 */
class LuaFuture {
public:
    using Result = sol::protected_function_result;

    LuaFuture() = default;
    LuaFuture(std::shared_ptr<LuaEngine> engine, std::future<Result>&& future) 
        : m_engine(engine), m_future(std::move(future)) {}

    /**
     * @brief Blocks until the result is available.
     * @return True if the result is ready.
     */
    bool wait() const {
        if (!m_future.valid()) return false;
        m_future.wait();
        return true;
    }

    /**
     * @brief Blocks and returns the Lua result.
     * @return sol::protected_function_result containing values or error.
     */
    Result get() {
        if (!m_future.valid()) throw std::runtime_error("LuaFuture is invalid or result already consumed");
        return m_future.get();
    }

    /**
     * @brief Checks if the result is ready without blocking.
     */
    bool isReady() const {
        if (!m_future.valid()) return false;
        return m_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    bool valid() const { return m_future.valid(); }

    /**
     * @brief Returns the engine used for this execution.
     */
    std::shared_ptr<LuaEngine> getEngine() const { return m_engine; }

protected:
    std::shared_ptr<LuaEngine> m_engine;
    std::future<Result> m_future;
};

} // namespace quasar::scripting
