/**
 * @file Transition.hpp
 * @brief Represents a transactional transition between states.
 */

#ifndef QUASAR_LOGIC_TRANSITION_HPP
#define QUASAR_LOGIC_TRANSITION_HPP

#include "quasar/logic/Expression.hpp"
#include "quasar/named/ICommand.hpp"
#include <memory>

namespace quasar::logic {

class State;

/**
 * @class Transition
 * @brief Transactional transition with pre/post-conditions and actions.
 */
class Transition : public quasar::named::NamedObject {
public:
    static std::shared_ptr<Transition> create(const std::string& name, const std::shared_ptr<State>& target, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    /** @brief Sets the pre-condition expression. */
    void setPreCondition(const Expression& expr);
    /** @brief Sets the post-condition expression. */
    void setPostCondition(const Expression& expr);
    /** @brief Sets the executable command for the transition. */
    void setAction(std::shared_ptr<quasar::named::ICommand> action);
    /** @brief Sets evaluation priority. */
    void setPriority(int priority);

    /** @brief Gets the target state. */
    std::shared_ptr<State> getTarget() const;
    /** @brief Gets the pre-condition. */
    const Expression& getPreCondition() const { return m_preCondition; }
    /** @brief Gets the post-condition. */
    const Expression& getPostCondition() const { return m_postCondition; }
    /** @brief Gets the action. */
    std::shared_ptr<quasar::named::ICommand> getAction() const { return m_action; }
    /** @brief Gets priority. */
    int getPriority() const { return m_priority; }

    /** @brief Returns object type. */
    std::string getType() const override { return "Transition"; }

protected:
    explicit Transition(const std::string& name, std::shared_ptr<State> target);

private:
    std::weak_ptr<State> m_target;
    Expression m_preCondition;
    Expression m_postCondition;
    std::shared_ptr<quasar::named::ICommand> m_action;
    int m_priority{0};
};

} // namespace quasar::logic

#endif // QUASAR_LOGIC_TRANSITION_HPP
