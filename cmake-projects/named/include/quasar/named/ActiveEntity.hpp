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
 * @compliance [FE-0130.1.3] Maintenance of an atomic internal state machine.
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
 * 
 * **Compliance**:
 * - Fulfills [FE-0130.1] Lifecycle Management.
 * - Fulfills [FE-0130.2] Observer and Producer Pattern.
 * - Fulfills [FE-0130.3] Field Reflexivity.
 * - Fulfills [FE-0130.4] Method Reflexivity.
 */
class ActiveEntity : public NamedObject {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~ActiveEntity() = default;

    // --- Phase 1: Lifecycle Management ---

    /**
     * @brief Initializes the entity.
     * @compliance [FE-0130.1.2] Lifecycle hook.
     */
    virtual void initialize() = 0;

    /**
     * @brief Starts the entity.
     * @compliance [FE-0130.1.2] Lifecycle hook.
     */
    virtual void start() = 0;

    /**
     * @brief Stops the entity.
     * @compliance [FE-0130.1.2] Lifecycle hook.
     */
    virtual void stop() = 0;

    /**
     * @brief Resets the entity.
     * @compliance [FE-0130.1.2] Lifecycle hook.
     */
    virtual void reset() = 0;

    /**
     * @brief Gets the current lifecycle state.
     * @return current state.
     */
    EntityState getState() const;

    // --- Phase 2: Observer/Producer Pattern inherited from NamedObject ---

    // --- Phase 3: Field Reflexivity ---


    /**
     * @brief Retrieves a registered child field by its registered string name.
     * @param name The registered reflexivity name of the field.
     * @return Shared pointer to the NamedObject representing the field, or nullptr if not found.
     * @compliance [FE-0130.3.2] API to retrieve fields dynamically.
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
     * @compliance [FE-0130.4.3] Executing bound methods dynamically.
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
     * @compliance [FE-0130.1.1] Define base class ActiveEntity.
     */
    ActiveEntity(const std::string& name);

    /**
     * @brief Sets the lifecycle state.
     * @param state The new state.
     */
    void setState(EntityState state);

    /**
     * @brief Registers a child NamedObject field to expose it via reflexivity.

     * @param name Name representing the field.
     * @param field The NamedObject to expose.
     * @compliance [FE-0130.3.1] Mechanism to register fields.
     */
    void registerField(const std::string& name, std::shared_ptr<NamedObject> field);

    /**
     * @brief Registers a callable method exposed via reflexivity.
     * @param name Name representing the method.
     * @param command Command abstraction that implements ICommand.
     * @compliance [FE-0130.4.1] Dynamic method dispatch system.
     */
    void registerMethod(const std::string& name, std::shared_ptr<ICommand> command);

protected:
    /** @brief Internal state machine. */
    std::atomic<EntityState> m_state{EntityState::Uninitialized};

    /** @brief Mutex for field map protection. */

    mutable std::recursive_timed_mutex m_fieldMutex;
    /** @brief Map of registered reflexive fields. */
    std::unordered_map<std::string, std::weak_ptr<NamedObject>> m_fields;

    /** @brief Mutex for method map protection. */
    mutable std::recursive_timed_mutex m_methodMutex;
    /** @brief Map of registered reflexive methods. */
    std::unordered_map<std::string, std::shared_ptr<ICommand>> m_methods;
};

/**
 * @brief Helper macro to register fields from within the derived class.
 */
#define REGISTER_FIELD(name, variable) this->registerField(name, variable)

/**
 * @class FunctionCommand
 * @brief Helper class to wrap std::function into an ICommand.
 * @compliance [FE-0130.4.2] Support binding std::function.
 */
class FunctionCommand : public ICommand {
public:
    /** @brief Function type for command. */
    using FuncType = std::function<std::shared_ptr<NamedObject>(std::shared_ptr<NamedObject>)>;
    
    /**
     * @brief Constructor.
     * @param func The function to wrap.
     */
    explicit FunctionCommand(FuncType func) : m_func(std::move(func)) {}
    
    /**
     * @brief Executes the wrapped function.
     * @param args The arguments.
     * @return The result.
     */
    std::shared_ptr<NamedObject> execute(std::shared_ptr<NamedObject> args) override {
        return m_func(args);
    }
protected:
    /** @brief The wrapped function. */
    FuncType m_func;
};

/**
 * @brief Helper macro to register std::function methods from within the derived class.
 */
#define REGISTER_METHOD(name, func) this->registerMethod(name, std::make_shared<quasar::named::FunctionCommand>(func))

} // namespace quasar::named

#endif // QUASAR_NAMED_ACTIVEENTITY_HPP

