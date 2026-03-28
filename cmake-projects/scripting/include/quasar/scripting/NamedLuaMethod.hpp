#ifndef QUASAR_SCRIPTING_NAMEDLUAMETHOD_HPP
#define QUASAR_SCRIPTING_NAMEDLUAMETHOD_HPP

#include "quasar/named/NamedMethod.hpp"
#include <sol/sol.hpp>
#include <memory>
#include <mutex>

namespace quasar::scripting {

/**
 * @struct NamedLuaMethodImpl
 * @brief Internal implementation for NamedLuaMethod to allow safe invalidation.
 */
struct NamedLuaMethodImpl {
    sol::function func;
    std::recursive_mutex mutex;
};

/**
 * @class NamedLuaMethod
 * @brief A NamedMethod that executes a Lua function.
 */
class NamedLuaMethod : public quasar::named::NamedMethod {
public:
    /**
     * @brief Factory method to create a new NamedLuaMethod.
     * @param name The name of the method.
     * @param func The Lua function to execute.
     * @param parent Optional parent.
     * @return A shared_ptr to the newly created NamedLuaMethod.
     */
    static std::shared_ptr<NamedLuaMethod> create(const std::string& name, sol::function func, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~NamedLuaMethod();

    /**
     * @brief Returns the type of the object.
     * @return "NamedLuaMethod".
     */
    std::string getType() const override;

    /**
     * @brief Invalidates the method by clearing the Lua function reference.
     * This is used during engine shutdown.
     */
    void invalidate();

protected:
    /**
     * @brief Protected constructor.
     * @param name The name of the method.
     * @param func The Lua function.
     */
    NamedLuaMethod(const std::string& name, sol::function func);

    /** @brief Internal constructor for sharing Impl. */
    NamedLuaMethod(const std::string& name, std::shared_ptr<NamedLuaMethodImpl> impl);

private:
    std::shared_ptr<NamedLuaMethodImpl> m_impl;
};

} // namespace quasar::scripting

#endif // QUASAR_SCRIPTING_NAMEDLUAMETHOD_HPP
