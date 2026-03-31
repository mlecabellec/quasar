/**
 * @file LogicComponent.hpp
 * @brief Base class for logic operational entities.
 */

#ifndef QUASAR_LOGIC_LOGICCOMPONENT_HPP
#define QUASAR_LOGIC_LOGICCOMPONENT_HPP

#include "quasar/named/ActiveEntity.hpp"
#include "quasar/logic/Common.hpp"

namespace quasar::logic {

/**
 * @class LogicComponent
 * @brief Represents an active logic component.
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260311-009.1.1] Base class.
 */
class LogicComponent : public quasar::named::ActiveEntity {
public:
    virtual ~LogicComponent() = default;

    /**
     * @brief Pauses the logic execution.
     */
    virtual void pause() = 0;

    /**
     * @brief Resumes the logic execution.
     */
    virtual void resume() = 0;

    /**
     * @brief Performs a single step of logic execution.
     * @param dt The elapsed time since the last step.
     */
    virtual void step(duration dt) = 0;

protected:
    /**
     * @brief Protected constructor.
     * @param name The name of the component.
     */
    LogicComponent(const std::string& name);
};

} // namespace quasar::logic

#endif // QUASAR_LOGIC_LOGICCOMPONENT_HPP
