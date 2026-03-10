#include "quasar/named/ActiveEntity.hpp"
#include "quasar/named/NamedConfig.hpp"
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
    // [CS-0010.21], [CS-0010.26] timed lock.
    std::unique_lock<std::recursive_timed_mutex> lock(m_observerMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring observer mutex in subscribe");
    }
    m_observers.push_back(observer);
}

void ActiveEntity::unsubscribe(std::weak_ptr<IObserver> observer) {
    // [CS-0010.21], [CS-0010.26] timed lock.
    std::unique_lock<std::recursive_timed_mutex> lock(m_observerMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring observer mutex in unsubscribe");
    }
    std::shared_ptr<IObserver> spObs = observer.lock();
    if (!spObs) return;
    
    // [CS-0010.34] auto forbidden.
    m_observers.erase(
        std::remove_if(m_observers.begin(), m_observers.end(),
                       [&spObs](const std::weak_ptr<IObserver>& wInfo) {
                           std::shared_ptr<IObserver> spInfo = wInfo.lock();
                           return !spInfo || spInfo == spObs;
                       }),
        m_observers.end());
}

void ActiveEntity::notifyObservers(std::shared_ptr<NamedObject> eventData) {
    // [CS-0010.21], [CS-0010.26] timed lock.
    std::unique_lock<std::recursive_timed_mutex> lock(m_observerMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring observer mutex in notifyObservers");
    }
    
    // [CS-0010.34] auto forbidden.
    std::vector<std::weak_ptr<IObserver>>::iterator it = m_observers.begin();
    std::size_t iterations = 0;
    while (it != m_observers.end()) {
        // [CS-0010.37] Hard limit on loops.
        if (++iterations > config::HARD_LIMIT_ITERATIONS) {
            throw std::runtime_error("Hard limit reached in notifyObservers loop");
        }
        
        if (std::shared_ptr<IObserver> obs = it->lock()) {
            obs->notify(eventData);
            ++it;
        } else {
            // Remove expired observers
            it = m_observers.erase(it);
        }
    }
}

void ActiveEntity::registerField(const std::string& name, std::shared_ptr<NamedObject> field) {
    // [CS-0010.21], [CS-0010.26] timed lock.
    std::unique_lock<std::recursive_timed_mutex> lock(m_fieldMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring field mutex in registerField");
    }
    // [CS-0010.2] use move semantics.
    m_fields[name] = std::move(field);
}

std::shared_ptr<NamedObject> ActiveEntity::getField(const std::string& name) const {
    // [CS-0010.21], [CS-0010.26] timed lock.
    std::unique_lock<std::recursive_timed_mutex> lock(m_fieldMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring field mutex in getField");
    }
    // [CS-0010.34] auto forbidden.
    std::unordered_map<std::string, std::weak_ptr<NamedObject>>::const_iterator it = m_fields.find(name);
    if (it != m_fields.end()) {
        return it->second.lock();
    }
    return nullptr;
}

std::vector<std::string> ActiveEntity::listFields() const {
    // [CS-0010.21], [CS-0010.26] timed lock.
    std::unique_lock<std::recursive_timed_mutex> lock(m_fieldMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring field mutex in listFields");
    }
    std::vector<std::string> keys;
    // [CS-0010.34] auto forbidden.
    for (std::unordered_map<std::string, std::weak_ptr<NamedObject>>::const_iterator it = m_fields.begin(); it != m_fields.end(); ++it) {
        keys.push_back(it->first);
    }
    return keys;
}

void ActiveEntity::registerMethod(const std::string& name, std::shared_ptr<ICommand> command) {
    // [CS-0010.21], [CS-0010.26] timed lock.
    std::unique_lock<std::recursive_timed_mutex> lock(m_methodMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring method mutex in registerMethod");
    }
    // [CS-0010.2] use move semantics.
    m_methods[name] = std::move(command);
}

std::shared_ptr<ICommand> ActiveEntity::getMethod(const std::string& methodName) const {
    // [CS-0010.21], [CS-0010.26] timed lock.
    std::unique_lock<std::recursive_timed_mutex> lock(m_methodMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring method mutex in getMethod");
    }
    // [CS-0010.34] auto forbidden.
    std::unordered_map<std::string, std::shared_ptr<ICommand>>::const_iterator it = m_methods.find(methodName);
    if (it != m_methods.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<NamedObject> ActiveEntity::execute(const std::string& methodName, std::shared_ptr<NamedObject> args) {
    // [CS-0010.34] auto forbidden.
    std::shared_ptr<ICommand> cmd = getMethod(methodName);
    if (cmd) {
        return cmd->execute(args);
    }
    throw std::runtime_error("Method not found: " + methodName);
}

std::vector<std::string> ActiveEntity::listMethods() const {
    // [CS-0010.21], [CS-0010.26] timed lock.
    std::unique_lock<std::recursive_timed_mutex> lock(m_methodMutex, config::DEFAULT_LOCK_TIMEOUT);
    if (!lock.owns_lock()) {
        throw std::runtime_error("Timeout acquiring method mutex in listMethods");
    }
    std::vector<std::string> keys;
    // [CS-0010.34] auto forbidden.
    for (std::unordered_map<std::string, std::shared_ptr<ICommand>>::const_iterator it = m_methods.begin(); it != m_methods.end(); ++it) {
        keys.push_back(it->first);
    }
    return keys;
}

} // namespace quasar::named

