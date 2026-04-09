#pragma once

#include <memory>
#include <sol/sol.hpp>
#include <string>
#include <stdexcept>
#include <typeindex>
#include <map>
#include <functional>
#include <type_traits>

namespace quasar::named {
class NamedObject;
}

namespace quasar::scripting {

/**
 * @brief Non-templated base interface for all Lua proxies.
 */
class ILuaProxy {
public:
    virtual ~ILuaProxy() = default;
    virtual bool isAlive() const = 0;
    virtual std::shared_ptr<quasar::named::NamedObject> lockAsNamedObject() const = 0;
};

/**
 * @brief Exception thrown when a Lua script attempts to access a destroyed C++ object.
 */
class LuaProxyInvalidException : public std::runtime_error {
public:
    explicit LuaProxyInvalidException(const std::string& typeName) 
        : std::runtime_error("Attempted to access destroyed Quasar object of type: " + typeName) {}
};

/**
 * @brief A Proxy wrapper that uses std::weak_ptr to safely interact with C++ objects from Lua.
 * 
 * According to the Proxy design pattern requested, this class decorrelates 
 * the object lifetime from the Lua engine. Lua holds a weak reference, 
 * allowing C++ to reclaim the memory when needed.
 */
template<typename T>
class LuaProxy : public ILuaProxy {
public:
    /**
     * @brief Constructs a proxy from a shared pointer.
     */
    explicit LuaProxy(std::shared_ptr<T> obj) : m_weak(obj) {}

    /**
     * @brief Checks if the underlying C++ object is still alive.
     */
    bool isAlive() const override {
        return !m_weak.expired();
    }

    /**
     * @brief Safely obtains a shared pointer to the underlying object.
     * @throws LuaProxyInvalidException if the object has been destroyed.
     */
    std::shared_ptr<T> lock() const {
        std::shared_ptr<T> ptr = m_weak.lock();
        if (!ptr) {
            throw LuaProxyInvalidException(typeid(T).name());
        }
        return ptr;
    }

    /**
     * @brief Helper to get the underlying pointer (use with caution).
     */
    T* get() const {
        return lock().get();
    }

    /**
     * @brief Equality operator for Lua.
     */
    bool operator==(const LuaProxy<T>& other) const {
        return m_weak.lock() == other.m_weak.lock();
    }

    /**
     * @brief Implementation of ILuaProxy interface.
     */
    std::shared_ptr<quasar::named::NamedObject> lockAsNamedObject() const override {
        std::shared_ptr<T> ptr = m_weak.lock();
        if (!ptr) return nullptr;

        // Compile-time branching for base-class check
        if constexpr (std::is_base_of_v<quasar::named::NamedObject, T>) {
            return std::static_pointer_cast<quasar::named::NamedObject>(ptr);
        } else if constexpr (std::is_polymorphic_v<T>) {
            return std::dynamic_pointer_cast<quasar::named::NamedObject>(ptr);
        } else {
            // T is not a NamedObject and not polymorphic, cannot be cast to NamedObject
            return nullptr;
        }
    }

private:
    std::weak_ptr<T> m_weak;
};

} // namespace quasar::scripting
