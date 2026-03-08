#include "quasar/scripting/ObjectTracker.hpp"
#include <algorithm>

namespace quasar::scripting {

void ObjectTracker::track(std::shared_ptr<named::NamedObject> obj) {
    if (!obj) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_trackedObjects[obj.get()] = std::weak_ptr<named::NamedObject>(obj);
}

bool ObjectTracker::isAlive(std::shared_ptr<named::NamedObject> obj) const {
    if (!obj) return false;
    std::lock_guard<std::mutex> lock(m_mutex);
    std::map<named::NamedObject*, std::weak_ptr<named::NamedObject>>::const_iterator it = m_trackedObjects.find(obj.get());
    if (it != m_trackedObjects.end()) {
        return !it->second.expired();
    }
    return false;
}

size_t ObjectTracker::getTrackedCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_trackedObjects.size();
}

void ObjectTracker::cleanup() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (std::map<named::NamedObject*, std::weak_ptr<named::NamedObject>>::iterator it = m_trackedObjects.begin(); it != m_trackedObjects.end(); ) {
        if (it->second.expired()) {
            it = m_trackedObjects.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace quasar::scripting
