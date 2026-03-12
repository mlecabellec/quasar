#pragma once

#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedConfig.hpp"
#include <map>
#include <memory>
#include <mutex>
#include <vector>

namespace quasar::scripting {

/**
 * @brief Tracks C++ objects exposed to Lua to monitor their lifecycle.
 * 
 * ObjectTracker provides a way to verify if a NamedObject is still
 * valid and present in C++ memory, even if Lua holds a shared_ptr
 * to it.
 */
class ObjectTracker {
public:
    static ObjectTracker& getInstance() {
        static ObjectTracker instance;
        return instance;
    }

    /**
     * @brief Registers an object for tracking.
     */
    void track(std::shared_ptr<named::NamedObject> obj);

    /**
     * @brief Checks if an object is still alive in the C++ hierarchy.
     * @param obj The object to check.
     * @return True if the object is still managed by C++.
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

private:
    ObjectTracker() = default;
    
    mutable std::timed_mutex m_mutex;
    std::map<named::NamedObject*, std::weak_ptr<named::NamedObject>> m_trackedObjects;
};

} // namespace quasar::scripting
