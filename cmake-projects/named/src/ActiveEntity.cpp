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

