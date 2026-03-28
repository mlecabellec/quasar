#include "quasar/scripting/ObjectTracker.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/scripting/NamedLuaMethod.hpp"
#include <algorithm>
#include <iostream>

namespace quasar::scripting {

void ObjectTracker::track(std::shared_ptr<named::NamedObject> obj) {
    if (!obj) return;
    std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) return;
    
    m_trackedObjects[obj.get()] = obj;
    
    // [CS-0010.44] Specialized tracking for methods to allow invalidation on shutdown.
    if (obj->getType() == "NamedLuaMethod") {
        m_methods.push_back(obj);
        // [CS-0010.44] Periodically prune the methods vector during tracking to avoid unbounded growth.
        if (m_methods.size() % 100 == 0) {
            m_methods.erase(std::remove_if(m_methods.begin(), m_methods.end(), 
                [](const std::weak_ptr<named::NamedObject>& w) { return w.expired(); }), m_methods.end());
        }
    }
}

void ObjectTracker::trackStrong(std::shared_ptr<named::NamedObject> obj) {
    if (!obj) return;
    std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) return;
    
    m_strongObjects[obj.get()] = obj;
    m_trackedObjects[obj.get()] = obj;

    if (obj->getType() == "NamedLuaMethod") {
        m_methods.push_back(obj);
        if (m_methods.size() % 100 == 0) {
            m_methods.erase(std::remove_if(m_methods.begin(), m_methods.end(), 
                [](const std::weak_ptr<named::NamedObject>& w) { return w.expired(); }), m_methods.end());
        }
    }
}

void ObjectTracker::untrack(named::NamedObject* obj) {
    if (!obj) return;
    std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) return;
    
    m_trackedObjects.erase(obj);
    m_strongObjects.erase(obj);
}

bool ObjectTracker::isAlive(std::shared_ptr<named::NamedObject> obj) const {
    if (!obj) return false;
    std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) return false;
    
    std::map<named::NamedObject*, std::weak_ptr<named::NamedObject>>::const_iterator it = m_trackedObjects.find(obj.get());
    if (it == m_trackedObjects.end()) return false;
    
    return !it->second.expired();
}

size_t ObjectTracker::getTrackedCount() const {
    std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) return 0;
    return m_trackedObjects.size();
}

void ObjectTracker::cleanup() {
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

    m_methods.erase(std::remove_if(m_methods.begin(), m_methods.end(), 
        [](const std::weak_ptr<named::NamedObject>& w) { return w.expired(); }), m_methods.end());
}

void ObjectTracker::invalidateMethods() {
    std::unique_lock<std::timed_mutex> lock(m_mutex, named::config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) return;

    for (std::weak_ptr<named::NamedObject>& weak_obj : m_methods) {
        if (std::shared_ptr<named::NamedObject> obj = weak_obj.lock()) {
            if (std::shared_ptr<NamedLuaMethod> method = std::dynamic_pointer_cast<NamedLuaMethod>(obj)) {
                method->invalidate();
            }
        }
    }
    m_methods.clear();
}

} // namespace quasar::scripting
