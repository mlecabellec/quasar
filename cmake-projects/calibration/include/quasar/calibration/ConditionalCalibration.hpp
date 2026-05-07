#ifndef QUASAR_CALIBRATION_CONDITIONALCALIBRATION_HPP
#define QUASAR_CALIBRATION_CONDITIONALCALIBRATION_HPP

#include "NamedCalibration.hpp"
#include <map>
#include <functional>

namespace quasar::calibration {

/**
 * @class ConditionalCalibration
 * @brief Calibration that selects an underlying calibration based on a condition.
 */
class ConditionalCalibration : public NamedCalibration {
private:
    /** @brief Default calibration if no case matches. */
    std::shared_ptr<ICalibration> m_defaultCase;
    /** @brief Function to evaluate the condition. */
    std::function<Variant()> m_conditionEvaluator;
    /** @brief Map of conditions to calibrations. */
    std::map<Variant, std::shared_ptr<ICalibration>> m_cases;
    
public:
    /**
     * @brief Constructs a ConditionalCalibration.
     * @param name Name of the calibration.
     */
    explicit ConditionalCalibration(const std::string& name) : NamedCalibration(name) {}

    /**
     * @brief Returns the type of the object.
     * @return "ConditionalCalibration"
     */
    std::string getType() const override { return "ConditionalCalibration"; }

    std::shared_ptr<NamedObject> clone(quasar::named::CopyPolicy policy = quasar::named::CopyPolicy::DUPLICATE) const override {
        std::shared_ptr<ConditionalCalibration> comp = std::make_shared<ConditionalCalibration>(getName());
        comp->m_defaultCase = m_defaultCase;
        comp->m_conditionEvaluator = m_conditionEvaluator;
        comp->m_cases = m_cases;
        return comp;
    }

    /**
     * @brief Sets the condition evaluator function.
     * @param eval The evaluator function.
     */
    void setEvaluator(std::function<Variant()> eval) { m_conditionEvaluator = std::move(eval); }

    /**
     * @brief Adds a calibration case for a specific condition value.
     * @param val The condition value.
     * @param cal The calibration to use for this value.
     */
    void addCase(const Variant& val, std::shared_ptr<ICalibration> cal) { m_cases[val] = cal; }

    /**
     * @brief Sets the default calibration.
     * @param cal The default calibration.
     */
    void setDefaultCase(std::shared_ptr<ICalibration> cal) { m_defaultCase = cal; }

    Variant rawToEng(const Variant& raw) const override {
        if (!m_conditionEvaluator) {
            if (m_defaultCase) return m_defaultCase->rawToEng(raw);
            return raw;
        }
        Variant cond = m_conditionEvaluator();
        std::map<Variant, std::shared_ptr<ICalibration>>::const_iterator it = m_cases.find(cond);
        if (it != m_cases.end() && it->second) return it->second->rawToEng(raw);
        if (m_defaultCase) return m_defaultCase->rawToEng(raw);
        return raw;
    }

    Variant engToRaw(const Variant& eng) const override {
        if (!m_conditionEvaluator) {
            if (m_defaultCase) return m_defaultCase->engToRaw(eng);
            return eng;
        }
        Variant cond = m_conditionEvaluator();
        std::map<Variant, std::shared_ptr<ICalibration>>::const_iterator it = m_cases.find(cond);
        if (it != m_cases.end() && it->second) return it->second->engToRaw(eng);
        if (m_defaultCase) return m_defaultCase->engToRaw(eng);
        return eng;
    }
};

} // namespace quasar::calibration

#endif // QUASAR_CALIBRATION_CONDITIONALCALIBRATION_HPP
