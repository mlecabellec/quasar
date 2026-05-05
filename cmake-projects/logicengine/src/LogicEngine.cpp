/**
 * @file LogicEngine.cpp
 * @brief Implementation of LogicEngine.
 */

#include "quasar/logic/LogicEngine.hpp"
#include <mutex>

namespace quasar::logic {

LogicEngine::LogicEngine(const std::string& name) : quasar::named::NamedObject(name) {
    // Constructor logic
}

std::shared_ptr<LogicEngine> LogicEngine::create(const std::string& name, std::shared_ptr<quasar::named::NamedObject> parent) {
    /** @brief Helper struct to allow make_shared with protected constructor. */
    struct Helper : public LogicEngine {
        explicit Helper(const std::string& n) : LogicEngine(n) {}
    };

    // [CS-0010.10] Use of new or delete keywords is forbidden. Use make_shared instead.
    std::shared_ptr<Helper> engine = std::make_shared<Helper>(name);
    
    // Properly initialize self-reference
    engine->setSelf(engine);
    
    if (parent) {
        engine->setParent(parent);
    }
    return engine;
}

void LogicEngine::addComponent(std::shared_ptr<LogicComponent> component) {
    // [CS-0010.46] Member field modification protected by timed recursive mutex.
    std::unique_lock<std::recursive_timed_mutex> lock(m_engineMutex, std::chrono::milliseconds(5000));
    if (lock.owns_lock()) {
        m_components.push_back(std::move(component));
    }
}

void LogicEngine::runCycle(duration dt) {
    // [CS-0010.46] Access protected by mutex.
    std::unique_lock<std::recursive_timed_mutex> lock(m_engineMutex, std::chrono::milliseconds(5000));
    if (!lock.owns_lock()) return;
    
    // [CS-0010.34] Use of "auto" is forbidden. Explicit types only.
    for (std::vector<std::shared_ptr<LogicComponent>>::iterator it = m_components.begin(); it != m_components.end(); ++it) {
        std::shared_ptr<LogicComponent>& component = *it;
        if (component->getState() == quasar::named::EntityState::Running) {
            component->step(dt);
        }
    }
}

} // namespace quasar::logic
