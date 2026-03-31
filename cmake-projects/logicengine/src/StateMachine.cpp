/**
 * @file StateMachine.cpp
 * @brief Implementation of the Transactional State Machine.
 */

#include "quasar/logic/StateMachine.hpp"
#include <algorithm>

namespace quasar::logic {

// --- State Implementation ---

State::State(const std::string& name) : quasar::named::NamedObject(name) {}

std::shared_ptr<State> State::create(const std::string& name, std::shared_ptr<quasar::named::NamedObject> parent) {
    struct Helper : public State {
        explicit Helper(const std::string& n) : State(n) {}
    };
    std::shared_ptr<Helper> state = std::make_shared<Helper>(name);
    state->setSelf(state);
    if (parent) {
        state->setParent(parent);
    }
    return state;
}

void State::setInvariant(const Expression& expr) { m_invariant = expr; }
void State::setFailureState(const std::shared_ptr<State>& failureState) { m_failureState = failureState; }
std::shared_ptr<State> State::getFailureState() const { return m_failureState.lock(); }

void State::setOnEntry(std::shared_ptr<IAction> action) { m_onEntry = std::move(action); }
void State::setOnExit(std::shared_ptr<IAction> action) { m_onExit = std::move(action); }
void State::setOnDo(std::shared_ptr<IAction> action) { m_onDo = std::move(action); }

void State::addTransition(const std::shared_ptr<Transition>& transition) {
    m_transitions.push_back(transition);
    // Keep sorted by priority (higher first)
    std::sort(m_transitions.begin(), m_transitions.end(), [](const std::shared_ptr<Transition>& a, const std::shared_ptr<Transition>& b) {
        return a->getPriority() > b->getPriority();
    });
}

const std::vector<std::shared_ptr<Transition>>& State::getTransitions() const { return m_transitions; }

bool State::isChildOf(const std::shared_ptr<State>& other) const {
    std::shared_ptr<quasar::named::NamedObject> p = getParent();
    while (p) {
        if (p == other) return true;
        p = p->getParent();
    }
    return false;
}

// --- Transition Implementation ---

Transition::Transition(const std::string& name, std::shared_ptr<State> target) 
    : quasar::named::NamedObject(name), m_target(target) {}

std::shared_ptr<Transition> Transition::create(const std::string& name, const std::shared_ptr<State>& target, std::shared_ptr<quasar::named::NamedObject> parent) {
    struct Helper : public Transition {
        Helper(const std::string& n, std::shared_ptr<State> t) : Transition(n, std::move(t)) {}
    };
    std::shared_ptr<Helper> transition = std::make_shared<Helper>(name, target);
    transition->setSelf(transition);
    if (parent) {
        transition->setParent(parent);
    }
    return transition;
}

void Transition::setPreCondition(const Expression& expr) { m_preCondition = expr; }
void Transition::setPostCondition(const Expression& expr) { m_postCondition = expr; }
void Transition::setAction(std::shared_ptr<quasar::named::ICommand> action) { m_action = std::move(action); }
void Transition::setPriority(int priority) { m_priority = priority; }
std::shared_ptr<State> Transition::getTarget() const { return m_target.lock(); }

// --- StateMachine Implementation ---

StateMachine::StateMachine(const std::string& name) : LogicComponent(name), m_groundState(State::create("__GROUND__")) {
    m_currentState = m_groundState;
}

std::shared_ptr<StateMachine> StateMachine::create(const std::string& name, std::shared_ptr<quasar::named::NamedObject> parent) {
    struct Helper : public StateMachine {
        explicit Helper(const std::string& n) : StateMachine(n) {}
    };
    std::shared_ptr<Helper> sm = std::make_shared<Helper>(name);
    sm->setSelf(sm);
    if (parent) {
        sm->setParent(parent);
    }
    return sm;
}

void StateMachine::initialize() { setState(quasar::named::EntityState::Ready); }
void StateMachine::start() { setState(quasar::named::EntityState::Running); }
void StateMachine::stop() { setState(quasar::named::EntityState::Ready); }
void StateMachine::reset() { 
    m_currentState = m_groundState;
    setState(quasar::named::EntityState::Ready); 
}
void StateMachine::pause() { m_paused = true; }
void StateMachine::resume() { m_paused = false; }

void StateMachine::setContextRoot(std::shared_ptr<quasar::named::NamedObject> root) { m_contextRoot = std::move(root); }
void StateMachine::setInitialState(const std::shared_ptr<State>& state) { m_currentState = state; }

void StateMachine::step(duration /*dt*/) {
    if (m_paused || getState() != quasar::named::EntityState::Running) return;
    processCycle();
}

void StateMachine::processCycle() {
    if (!m_currentState) {
        m_currentState = m_groundState;
    }

    // 1. Invariant Verification
    if (!m_currentState->getInvariant().evaluate(m_contextRoot)) {
        std::shared_ptr<State> failState = m_currentState->getFailureState();
        m_currentState = failState ? failState : m_groundState;
        return; // Forced transition, end cycle.
    }

    // 2. Transition Selection
    std::vector<std::shared_ptr<Transition>> transitions = m_currentState->getTransitions();
    for (std::vector<std::shared_ptr<Transition>>::iterator it = transitions.begin(); it != transitions.end(); ++it) {
        const std::shared_ptr<Transition>& transition = *it;
        
        // 3. Pre-condition
        if (transition->getPreCondition().evaluate(m_contextRoot)) {
            // 4. Action Execution
            if (transition->getAction()) {
                transition->getAction()->execute(m_contextRoot);
            }

            // 5. Commitment (Post-condition AND Destination Invariant)
            std::shared_ptr<State> target = transition->getTarget();
            if (target) {
                bool postOk = transition->getPostCondition().evaluate(m_contextRoot);
                bool targetInvOk = target->getInvariant().evaluate(m_contextRoot);

                if (postOk && targetInvOk) {
                    // Success
                    m_currentState = target;
                } else {
                    // Fail to commit, move to target's failure state or ground
                    std::shared_ptr<State> failState = target->getFailureState();
                    m_currentState = failState ? failState : m_groundState;
                }
                return; // One transition per cycle for StateMachine
            }
        }
    }
}

} // namespace quasar::logic
