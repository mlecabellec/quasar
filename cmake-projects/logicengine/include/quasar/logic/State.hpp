/**
 * @file State.hpp
 * @brief Represents a state with invariants and failure handling.
 */

#ifndef QUASAR_LOGIC_STATE_HPP
#define QUASAR_LOGIC_STATE_HPP

#include "quasar/logic/Expression.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/logic/Common.hpp"
#include <memory>
#include <vector>

namespace quasar::logic {

class Transition;

/**
 * @class State
 * @brief Represents a state in an HSM or SFC.
 */
class State : public quasar::named::NamedObject {
public:
    static std::shared_ptr<State> create(const std::string& name, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    /** @brief Sets state invariant (must always be true). */
    void setInvariant(const Expression& expr);
    /** @brief Sets the state to move to if invariant fails. */
    void setFailureState(const std::shared_ptr<State>& failureState);

    /** @brief Sets entry action. */
    void setOnEntry(std::shared_ptr<IAction> action);
    /** @brief Sets exit action. */
    void setOnExit(std::shared_ptr<IAction> action);
    /** @brief Sets 'do' activity. */
    void setOnDo(std::shared_ptr<IAction> action);

    /** @brief Gets invariant expression. */
    const Expression& getInvariant() const { return m_invariant; }
    /** @brief Gets failure state. */
    std::shared_ptr<State> getFailureState() const;
    
    /** @brief Adds an outgoing transition. */
    void addTransition(const std::shared_ptr<Transition>& transition);
    /** @brief Gets all outgoing transitions. */
    const std::vector<std::shared_ptr<Transition>>& getTransitions() const;

    /** @brief Executes entry action. */
    void enter(std::shared_ptr<quasar::named::NamedObject> ctx = nullptr);
    /** @brief Executes exit action. */
    void exit(std::shared_ptr<quasar::named::NamedObject> ctx = nullptr);

    /** @brief Checks if this state is a child of another. */
    bool isChildOf(const std::shared_ptr<State>& other) const;

    /** @brief Returns object type. */
    std::string getType() const override { return "State"; }

protected:
    explicit State(const std::string& name);

    /** @brief Overridden to auto-register transitions. */
    void addChild(std::shared_ptr<quasar::named::NamedObject> child) override;

private:
    Expression m_invariant;
    std::weak_ptr<State> m_failureState;
    
    std::shared_ptr<IAction> m_onEntry;
    std::shared_ptr<IAction> m_onExit;
    std::shared_ptr<IAction> m_onDo;

    std::vector<std::shared_ptr<Transition>> m_transitions;
};

} // namespace quasar::logic

#endif // QUASAR_LOGIC_STATE_HPP
