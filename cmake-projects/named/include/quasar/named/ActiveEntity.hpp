#ifndef QUASAR_NAMED_ACTIVEENTITY_HPP
#define QUASAR_NAMED_ACTIVEENTITY_HPP

#include "quasar/named/NamedObject.hpp"
#include "quasar/named/IObserver.hpp"
#include "quasar/named/ICommand.hpp"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace quasar::named {

/**
 * @enum EntityState
 * @brief Represents the lifecycle state of an ActiveEntity.
 */
enum class EntityState {
    Uninitialized,
    Ready,
    Running,
    Error
};

/**
 * @class ActiveEntity
 * @brief Generic interface and base implementation for objects with lifecycle management,
 *        producer/observer patterns, and reflexivity capabilities.
 */
class ActiveEntity : public NamedObject {
public:
    virtual ~ActiveEntity() = default;

    // --- Phase 1: Lifecycle Management ---

    virtual void initialize() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void reset() = 0;

    /**
     * @brief Gets the current lifecycle state.
     * @return current state.
     */
    EntityState getState() const;

    // --- Phase 2: Observer/Producer Pattern ---

    /**
     * @brief Subscribes an observer to this entity's events.
     * @param observer Weak pointer to the observer.
     */
    void subscribe(std::weak_ptr<IObserver> observer);

    /**
     * @brief Unsubscribes an observer.
     * @param observer Weak pointer to the observer to remove.
     */
    void unsubscribe(std::weak_ptr<IObserver> observer);

    // --- Phase 3: Field Reflexivity ---

    /**
     * @brief Retrieves a registered child field by its registered string name.
     * @param name The registered reflexivity name of the field.
     * @return Shared pointer to the NamedObject representing the field, or nullptr if not found.
     */
    std::shared_ptr<NamedObject> getField(const std::string& name) const;

    /**
     * @brief Lists the names of all registered fields.
     * @return Vector of string names.
     */
    std::vector<std::string> listFields() const;

    // --- Phase 4: Method Reflexivity ---

    /**
     * @brief Retrieves a registered method/command.
     * @param methodName The name of the registered command.
     * @return Shared pointer to the ICommand executable, or nullptr if not found.
     */
    std::shared_ptr<ICommand> getMethod(const std::string& methodName) const;

    /**
     * @brief Dynamically executes a registered method by name.
     * @param methodName The name of the method to execute.
     * @param args The arguments to pass.
     * @return The result object as a NamedObject.
     * @throws std::runtime_error if the method is not found.
     */
    std::shared_ptr<NamedObject> execute(const std::string& methodName, std::shared_ptr<NamedObject> args);

    /**
     * @brief Lists the names of all registered methods.
     * @return Vector of registered method names.
     */
    std::vector<std::string> listMethods() const;

protected:
    /**
     * @brief Protected constructor.
     * @param name The name of the entity.
     */
    ActiveEntity(const std::string& name);

    /**
     * @brief Sets the lifecycle state.
     * @param state The new state.
     */
    void setState(EntityState state);

    /**
     * @brief Broadcasts an event to all subscribed observers.
     * @param eventData The data payload to notify.
     */
    void notifyObservers(std::shared_ptr<NamedObject> eventData);

    /**
     * @brief Registers a child NamedObject field to expose it via reflexivity.
     * @param name Name representing the field.
     * @param field The NamedObject to expose.
     */
    void registerField(const std::string& name, std::shared_ptr<NamedObject> field);

    /**
     * @brief Registers a callable method exposed via reflexivity.
     * @param name Name representing the method.
     * @param command Command abstraction that implements ICommand.
     */
    void registerMethod(const std::string& name, std::shared_ptr<ICommand> command);

private:
    std::atomic<EntityState> m_state{EntityState::Uninitialized};

    mutable std::recursive_timed_mutex m_observerMutex;
    std::vector<std::weak_ptr<IObserver>> m_observers;

    mutable std::recursive_timed_mutex m_fieldMutex;
    std::unordered_map<std::string, std::weak_ptr<NamedObject>> m_fields;

    mutable std::recursive_timed_mutex m_methodMutex;
    std::unordered_map<std::string, std::shared_ptr<ICommand>> m_methods;
};

// Helper macro to register fields from within the derived class
#define REGISTER_FIELD(name, variable) this->registerField(name, variable)

/**
 * @class FunctionCommand
 * @brief Helper class to wrap std::function into an ICommand.
 */
class FunctionCommand : public ICommand {
public:
    using FuncType = std::function<std::shared_ptr<NamedObject>(std::shared_ptr<NamedObject>)>;
    FunctionCommand(FuncType func) : m_func(std::move(func)) {}
    std::shared_ptr<NamedObject> execute(std::shared_ptr<NamedObject> args) override {
        return m_func(args);
    }
private:
    FuncType m_func;
};

// Helper macro to register std::function methods from within the derived class
#define REGISTER_METHOD(name, func) this->registerMethod(name, std::make_shared<quasar::named::FunctionCommand>(func))

} // namespace quasar::named

#endif // QUASAR_NAMED_ACTIVEENTITY_HPP
