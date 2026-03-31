/**
 * @file Common.hpp
 * @brief Common interfaces for the logic engine.
 */

#ifndef QUASAR_LOGIC_COMMON_HPP
#define QUASAR_LOGIC_COMMON_HPP

#include "quasar/named/NamedObject.hpp"
#include <memory>
#include <chrono>

namespace quasar::logic {

/**
 * @interface ICondition
 * @brief Interface for boolean condition evaluation.
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260311-009.1.2] Define base interfaces.
 */
class ICondition {
public:
    virtual ~ICondition() = default;
    
    /**
     * @brief Evaluates the condition.
     * @return true if condition is met.
     */
    virtual bool evaluate() const = 0;
};

/**
 * @interface IAction
 * @brief Interface for executable actions.
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260311-009.1.2] Define base interfaces.
 */
class IAction {
public:
    virtual ~IAction() = default;
    
    /**
     * @brief Executes the action.
     * @param context Context object (parameters/state).
     */
    virtual void execute(std::shared_ptr<quasar::named::NamedObject> context = nullptr) = 0;
};

/**
 * @interface ITrigger
 * @brief Interface for event-driven triggers.
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260311-009.1.2] Define base interfaces.
 */
class ITrigger {
public:
    virtual ~ITrigger() = default;
    
    /**
     * @brief Checks if the trigger has fired.
     * @return true if fired.
     */
    virtual bool isFired() const = 0;
};

/** @brief Typedef for duration used in step() methods. */
using duration = std::chrono::nanoseconds;

} // namespace quasar::logic

#endif // QUASAR_LOGIC_COMMON_HPP
