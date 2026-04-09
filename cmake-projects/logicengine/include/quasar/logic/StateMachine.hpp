/**
 * @file StateMachine.hpp
 * @brief Transactional Hierarchical State Machine execution engine.
 */

#ifndef QUASAR_LOGIC_STATEMACHINE_HPP
#define QUASAR_LOGIC_STATEMACHINE_HPP

#include "quasar/logic/LogicComponent.hpp"
#include "quasar/logic/State.hpp"
#include "quasar/logic/Transition.hpp"
#include <queue>

namespace quasar::logic {

/**
 * @class StateMachine
 * @brief Executes a state machine with cyclic transactional guarantees.
 * 
 * @reference [TSK-20260311-009.2] State Machine (FSM/HSM) Support
 * @reference [FE-0240.2] State Machine Support (HSM)
 */
class StateMachine : public LogicComponent {
public:
    static std::shared_ptr<StateMachine> create(const std::string& name, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    void initialize() override;
    void start() override;
    void stop() override;
    void reset() override;
    void pause() override;
    void resume() override;

    /**
     * @brief Performs the transactional logic cycle.
     * @param dt Cycle duration.
     */
    void step(duration dt) override;

    /** @brief Sets the root of the variable tree for Lua 'ctx'. */
    void setContextRoot(std::shared_ptr<quasar::named::NamedObject> root);

    /** @brief Sets the initial state. */
    void setInitialState(const std::shared_ptr<State>& state);

    /** @brief Gets current state (for testing). */
    std::shared_ptr<State> getCurrentState() const { return m_currentState; }

protected:
    explicit StateMachine(const std::string& name);

private:
    std::shared_ptr<State> m_groundState;
    std::shared_ptr<State> m_currentState;
    std::shared_ptr<quasar::named::NamedObject> m_contextRoot;
    
    bool m_paused{false};

    /** @brief Performs one full transactional transition check. */
    void processCycle();
};

} // namespace quasar::logic

#endif // QUASAR_LOGIC_STATEMACHINE_HPP
