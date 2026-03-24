#ifndef QUASAR_CALIBRATION_CONDITIONALCALIBRATION_HPP
#define QUASAR_CALIBRATION_CONDITIONALCALIBRATION_HPP

#include "NamedCalibration.hpp"
#include <map>
#include <functional>

namespace quasar::calibration {

class ConditionalCalibration : public NamedCalibration {
private:
    std::shared_ptr<ICalibration> m_defaultCase;
    std::function<Variant()> m_conditionEvaluator;
    std::map<Variant, std::shared_ptr<ICalibration>> m_cases;
    
public:
    explicit ConditionalCalibration(const std::string& name) : NamedCalibration(name) {}

    std::string getType() const override { return "ConditionalCalibration"; }

    std::shared_ptr<NamedObject> clone(quasar::named::CopyPolicy policy = quasar::named::CopyPolicy::DUPLICATE) const override {
        auto comp = std::make_shared<ConditionalCalibration>(getName());
        comp->m_defaultCase = m_defaultCase;
        comp->m_conditionEvaluator = m_conditionEvaluator;
        comp->m_cases = m_cases;
        return comp;
    }

    void setEvaluator(std::function<Variant()> eval) { m_conditionEvaluator = std::move(eval); }
    void addCase(const Variant& val, std::shared_ptr<ICalibration> cal) { m_cases[val] = cal; }
    void setDefaultCase(std::shared_ptr<ICalibration> cal) { m_defaultCase = cal; }

    Variant rawToEng(const Variant& raw) const override {
        if (!m_conditionEvaluator) {
            if (m_defaultCase) return m_defaultCase->rawToEng(raw);
            return raw;
        }
        Variant cond = m_conditionEvaluator();
        auto it = m_cases.find(cond);
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
        auto it = m_cases.find(cond);
        if (it != m_cases.end() && it->second) return it->second->engToRaw(eng);
        if (m_defaultCase) return m_defaultCase->engToRaw(eng);
        return eng;
    }
};

} // namespace quasar::calibration

#endif // QUASAR_CALIBRATION_CONDITIONALCALIBRATION_HPP
