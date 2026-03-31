/**
 * @file SFC.cpp
 * @brief Implementation of Transactional SFC.
 */

#include "quasar/logic/SFC.hpp"

namespace quasar::logic {

SFC::SFC(const std::string& name) : LogicComponent(name), m_groundState(State::create("__SFC_GROUND__")) {
}

std::shared_ptr<SFC> SFC::create(const std::string& name, std::shared_ptr<quasar::named::NamedObject> parent) {
    struct Helper : public SFC {
        explicit Helper(const std::string& n) : SFC(n) {}
    };
    std::shared_ptr<Helper> sfc = std::make_shared<Helper>(name);
    sfc->setSelf(sfc);
    if (parent) {
        sfc->setParent(parent);
    }
    return sfc;
}

void SFC::initialize() { setState(quasar::named::EntityState::Ready); }
void SFC::start() { setState(quasar::named::EntityState::Running); }
void SFC::stop() { setState(quasar::named::EntityState::Ready); }
void SFC::reset() { 
    m_activeStates.clear();
    setState(quasar::named::EntityState::Ready); 
}
void SFC::pause() { m_paused = true; }
void SFC::resume() { m_paused = false; }

void SFC::setContextRoot(std::shared_ptr<quasar::named::NamedObject> root) { m_contextRoot = std::move(root); }
void SFC::addInitialStep(const std::shared_ptr<State>& step) { m_activeStates.insert(step); }

void SFC::step(duration dt) {
    if (m_paused || getState() != quasar::named::EntityState::Running) return;
    processCycle(dt);
}

void SFC::processCycle(duration /*dt*/) {
    // [TSK-20260311-009.4] Resource Safety
    const std::size_t MAX_TOKENS = 100;
    if (m_activeStates.size() > MAX_TOKENS) {
        m_activeStates.clear();
        m_activeStates.insert(m_groundState);
        return;
    }

    std::set<std::shared_ptr<State>> nextActiveStates;
    std::set<std::shared_ptr<State>> statesToProcess = m_activeStates;

    for (std::set<std::shared_ptr<State>>::iterator it = statesToProcess.begin(); it != statesToProcess.end(); ++it) {
        std::shared_ptr<State> currentState = *it;

        // 1. Invariant Verification
        if (!currentState->getInvariant().evaluate(m_contextRoot)) {
            std::shared_ptr<State> failState = currentState->getFailureState();
            nextActiveStates.insert(failState ? failState : m_groundState);
            continue; 
        }

        // 2. Evaluate all outgoing transitions
        // Transitions are already sorted by priority in State::addTransition.
        // We assume names are unique, providing deterministic order for same priority.
        const std::vector<std::shared_ptr<Transition>>& transitions = currentState->getTransitions();
        bool transitioned = false;

        for (std::vector<std::shared_ptr<Transition>>::const_iterator tIt = transitions.begin(); tIt != transitions.end(); ++tIt) {
            const std::shared_ptr<Transition>& transition = *tIt;

            if (transition->getPreCondition().evaluate(m_contextRoot)) {
                // 3. Action
                if (transition->getAction()) {
                    transition->getAction()->execute(m_contextRoot);
                }

                // 4. Commitment
                std::shared_ptr<State> target = transition->getTarget();
                if (target) {
                    bool postOk = transition->getPostCondition().evaluate(m_contextRoot);
                    bool targetInvOk = target->getInvariant().evaluate(m_contextRoot);

                    if (postOk && targetInvOk) {
                        nextActiveStates.insert(target);
                    } else {
                        std::shared_ptr<State> failState = target->getFailureState();
                        nextActiveStates.insert(failState ? failState : m_groundState);
                    }
                    transitioned = true;
                }
            }
        }

        // If no transition fired, stay in current state
        if (!transitioned) {
            nextActiveStates.insert(currentState);
        }
    }

    // 5. Merge (std::set automatically merges identical states reached in same cycle)
    m_activeStates = nextActiveStates;
}

} // namespace quasar::logic
