#pragma once

#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedConfig.hpp"
#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <any>

namespace quasar::scripting {

/**
 * @brief Tracks C++ objects exposed to Lua to monitor their lifecycle.
 * 
 * ObjectTracker provides a way to verify if a NamedObject is still
 * valid and present in C++ memory, even if Lua holds a shared_ptr
 * to it. It also manages strong references for standalone objects.
 */
class ObjectTracker {
public:
    static ObjectTracker& getInstance() {
        static ObjectTracker instance;
        return instance;
    }

    /**
     * @brief Registers an object for tracking (weak reference).
     */
    void track(std::shared_ptr<named::NamedObject> obj);

    /**
     * @brief Registers any shared object for tracking (strong reference, keeps it alive).
     */
    template<typename T>
    void trackStrong(std::shared_ptr<T> obj) {
        if (!obj) return;
        std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
        if (!lock.owns_lock()) return;
        
        m_anyStrongObjects.push_back(obj);

        // If it's a NamedObject, also track it in the weak map for isAlive() checks
        if constexpr (std::is_base_of_v<named::NamedObject, T>) {
            m_trackedObjects[obj.get()] = std::static_pointer_cast<named::NamedObject>(obj);
            if (obj->getType() == "NamedLuaMethod") {
                m_methods.push_back(std::static_pointer_cast<named::NamedObject>(obj));
            }
        }
    }

    /**
     * @brief Removes an object from tracking.
     */
    void untrack(named::NamedObject* obj);

    /**
     * @brief Checks if an object is still alive in the C++ hierarchy.
     */
    bool isAlive(std::shared_ptr<named::NamedObject> obj) const;

    /**
     * @brief Returns the number of tracked objects.
     */
    size_t getTrackedCount() const;

    /**
     * @brief Periodically cleans up dead weak pointers.
     */
    void cleanup();

    /**
     * @brief Invalidates all tracked NamedLuaMethod objects.
     */
    void invalidateMethods();

private:
    ObjectTracker() = default;
    
    mutable std::timed_mutex m_mutex;
    std::map<named::NamedObject*, std::weak_ptr<named::NamedObject>> m_trackedObjects;
    std::vector<std::shared_ptr<named::NamedObject>> m_methods; // Special handling for methods
    std::vector<std::any> m_anyStrongObjects;
};

} // namespace quasar::scripting
