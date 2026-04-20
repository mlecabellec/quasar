#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/scripting/NamedLuaMethod.hpp"
#include "quasar/scripting/LuaEngine.hpp"
#include <algorithm>
#include <iostream>

namespace quasar::scripting {

void ObjectTracker::track(std::shared_ptr<named::NamedObject> obj) {
    // Guard against invalid inputs.
    if (!obj) return;
    std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) return;
    
    // Register the raw pointer for existence tracking.
    m_trackedObjects[obj.get()] = obj;
    
    // Specialized pool for method invalidation.
    if (obj->getType() == "NamedLuaMethod") {
        m_methods.push_back(obj);
    }
}

void ObjectTracker::untrack(named::NamedObject* obj) {
    // Check if the pointer was being tracked.
    if (!obj) return;
    std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) return;
    
    m_trackedObjects.erase(obj);
}

bool ObjectTracker::isAlive(std::shared_ptr<named::NamedObject> obj) const {
    // Verify object presence in the hierarchy.
    if (!obj) return false;
    std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) return false;
    
    std::map<named::NamedObject*, std::weak_ptr<named::NamedObject>>::const_iterator it = m_trackedObjects.find(obj.get());
    if (it == m_trackedObjects.end()) return false;
    
    // If the weak pointer is expired, the C++ object is gone.
    return !it->second.expired();
}

void ObjectTracker::cleanup() {
    // Periodically remove stale references to keep the maps lean.
    std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) return;
    
    std::map<named::NamedObject*, std::weak_ptr<named::NamedObject>>::iterator it = m_trackedObjects.begin();
    while (it != m_trackedObjects.end()) {
        if (it->second.expired()) {
            it = m_trackedObjects.erase(it);
        } else {
            ++it;
        }
    }

    // Clean up the global method pool.
    m_methods.erase(std::remove_if(m_methods.begin(), m_methods.end(), 
        [](const std::shared_ptr<named::NamedObject>& s) { return s.use_count() <= 1; }), m_methods.end());
}

void ObjectTracker::invalidateMethods(size_t engineId) {
    // [CS-0010.44] Only invalidate methods belonging to the specified engine.
    std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) return;

    std::vector<std::shared_ptr<named::NamedObject>>::iterator it = m_methods.begin();
    while (it != m_methods.end()) {
        // [CS-0010.44] Attempt to cast to Lua method and verify engine ID.
        std::shared_ptr<NamedLuaMethod> method = std::dynamic_pointer_cast<NamedLuaMethod>(*it);
        if (method) {
            // Only invalidate if the method belongs to the engine being shut down.
            if (method->getEngineId() == engineId) {
                // Detach from Lua and remove from tracking pool.
                method->invalidate();
                it = m_methods.erase(it);
                continue;
            }
        }
        ++it;
    }
}

void ObjectTracker::untrackAll(size_t engineId) {
    // [CS-0010.21] Release all strong references kept alive by this engine.
    std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) return;

    m_scopedStrongObjects.erase(engineId);
}

} // namespace quasar::scripting
