/**
 * @file LogicEngine.hpp
 * @brief Orchestration layer for logic components.
 */

#ifndef QUASAR_LOGIC_LOGICENGINE_HPP
#define QUASAR_LOGIC_LOGICENGINE_HPP

#include "quasar/named/NamedObject.hpp"
#include "quasar/logic/LogicComponent.hpp"
#include <vector>
#include <memory>

namespace quasar::logic {

/**
 * @class LogicEngine
 * @brief Manages multiple logic components and coordinates their execution cycles.
 * 
 * **Compliance**:
 * - Fulfills [TSK-20260311-009.1.1] Core logic project.
 * - @reference [FE-0240.1] Core Logic Project
 */
class LogicEngine : public quasar::named::NamedObject {
public:
    /**
     * @brief Factory method for creating a LogicEngine.
     * @param name Name of the engine.
     * @param parent Optional parent.
     * @return Shared pointer to the new engine.
     */
    static std::shared_ptr<LogicEngine> create(const std::string& name, std::shared_ptr<quasar::named::NamedObject> parent = nullptr);

    /**
     * @brief Adds a logic component to the engine.
     * @param component The component to add.
     */
    void addComponent(std::shared_ptr<LogicComponent> component);

    /**
     * @brief Runs a single execution cycle for all managed components.
     * @param dt Elapsed time for this cycle.
     */
    void runCycle(duration dt);

    /**
     * @brief Returns the object type.
     * @return "LogicEngine"
     */
    std::string getType() const override { return "LogicEngine"; }

protected:
    /**
     * @brief Protected constructor.
     * @param name Name of the engine.
     */
    explicit LogicEngine(const std::string& name);

private:
    /** @brief List of managed logic components. */
    std::vector<std::shared_ptr<LogicComponent>> m_components;
    /** @brief Mutex for protecting component list. */
    mutable std::recursive_timed_mutex m_engineMutex;
};

} // namespace quasar::logic

#endif // QUASAR_LOGIC_LOGICENGINE_HPP
