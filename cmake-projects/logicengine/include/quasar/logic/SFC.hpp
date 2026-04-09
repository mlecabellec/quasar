/**
 * @file SFC.hpp
 * @brief Transactional Sequential Function Chart (SFC) execution engine.
 */

#ifndef QUASAR_LOGIC_SFC_HPP
#define QUASAR_LOGIC_SFC_HPP

#include "quasar/logic/LogicComponent.hpp"
#include "quasar/logic/State.hpp"
#include "quasar/logic/Transition.hpp"
#include <map>
#include <set>

namespace quasar::logic {

/**
 * @class SFC
 * @brief Executes an SFC with multi-token parallel transactional logic.
 * 
 * @reference [TSK-20260311-009.3] Sequential Function Chart (SFC) / Grafcet
 * @reference [FE-0240.3] Sequential Function Chart (SFC) / Grafcet
 */
class SFC : public LogicComponent {
public:
    static std::shared_ptr<SFC> create(const std::string& name, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    void initialize() override;
    void start() override;
    void stop() override;
    void reset() override;
    void pause() override;
    void resume() override;

    void step(duration dt) override;

    /** @brief Sets the variable context root. */
    void setContextRoot(std::shared_ptr<quasar::named::NamedObject> root);

    /** @brief Adds an initial active step. */
    void addInitialStep(const std::shared_ptr<State>& step);

    /** @brief Gets currently active states (for testing). */
    const std::set<std::shared_ptr<State>>& getActiveStates() const { return m_activeStates; }

protected:
    explicit SFC(const std::string& name);

private:
    std::shared_ptr<State> m_groundState;
    std::shared_ptr<quasar::named::NamedObject> m_contextRoot;
    
    /** @brief Set of currently active states (steps). */
    std::set<std::shared_ptr<State>> m_activeStates;
    
    bool m_paused{false};

    void processCycle(duration dt);
};

} // namespace quasar::logic

#endif // QUASAR_LOGIC_SFC_HPP
