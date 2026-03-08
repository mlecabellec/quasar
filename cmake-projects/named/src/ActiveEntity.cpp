#include "quasar/named/ActiveEntity.hpp"
#include <stdexcept>
#include <algorithm>

namespace quasar::named {

ActiveEntity::ActiveEntity(const std::string& name)
    : NamedObject(name) {
}

EntityState ActiveEntity::getState() const {
    return m_state.load();
}

void ActiveEntity::setState(EntityState state) {
    m_state.store(state);
}

void ActiveEntity::subscribe(std::weak_ptr<IObserver> observer) {
    std::lock_guard<std::recursive_timed_mutex> lock(m_observerMutex);
    m_observers.push_back(observer);
}

void ActiveEntity::unsubscribe(std::weak_ptr<IObserver> observer) {
    std::lock_guard<std::recursive_timed_mutex> lock(m_observerMutex);
    auto spObs = observer.lock();
    if (!spObs) return;
    
    m_observers.erase(
        std::remove_if(m_observers.begin(), m_observers.end(),
                       [&spObs](const std::weak_ptr<IObserver>& wInfo) {
                           auto spInfo = wInfo.lock();
                           return !spInfo || spInfo == spObs;
                       }),
        m_observers.end());
}

void ActiveEntity::notifyObservers(std::shared_ptr<NamedObject> eventData) {
    std::lock_guard<std::recursive_timed_mutex> lock(m_observerMutex);
    auto it = m_observers.begin();
    while (it != m_observers.end()) {
        if (auto obs = it->lock()) {
            obs->notify(eventData);
            ++it;
        } else {
            // Remove expired observers
            it = m_observers.erase(it);
        }
    }
}

void ActiveEntity::registerField(const std::string& name, std::shared_ptr<NamedObject> field) {
    std::lock_guard<std::recursive_timed_mutex> lock(m_fieldMutex);
    m_fields[name] = field;
}

std::shared_ptr<NamedObject> ActiveEntity::getField(const std::string& name) const {
    std::lock_guard<std::recursive_timed_mutex> lock(m_fieldMutex);
    auto it = m_fields.find(name);
    if (it != m_fields.end()) {
        return it->second.lock();
    }
    return nullptr;
}

std::vector<std::string> ActiveEntity::listFields() const {
    std::lock_guard<std::recursive_timed_mutex> lock(m_fieldMutex);
    std::vector<std::string> keys;
    for (const auto& pair : m_fields) {
        keys.push_back(pair.first);
    }
    return keys;
}

void ActiveEntity::registerMethod(const std::string& name, std::shared_ptr<ICommand> command) {
    std::lock_guard<std::recursive_timed_mutex> lock(m_methodMutex);
    m_methods[name] = command;
}

std::shared_ptr<ICommand> ActiveEntity::getMethod(const std::string& methodName) const {
    std::lock_guard<std::recursive_timed_mutex> lock(m_methodMutex);
    auto it = m_methods.find(methodName);
    if (it != m_methods.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<NamedObject> ActiveEntity::execute(const std::string& methodName, std::shared_ptr<NamedObject> args) {
    auto cmd = getMethod(methodName);
    if (cmd) {
        return cmd->execute(args);
    }
    throw std::runtime_error("Method not found: " + methodName);
}

std::vector<std::string> ActiveEntity::listMethods() const {
    std::lock_guard<std::recursive_timed_mutex> lock(m_methodMutex);
    std::vector<std::string> keys;
    for (const auto& pair : m_methods) {
        keys.push_back(pair.first);
    }
    return keys;
}

} // namespace quasar::named
