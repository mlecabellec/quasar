#include "quasar/datalogger/ValueThresholdTrigger.hpp"
#include <cmath>

namespace quasar::datalogger {

ValueThresholdTrigger::ValueThresholdTrigger(const std::string& targetSourcePath, double threshold, bool triggerAbove)
    : m_targetSourcePath(targetSourcePath), m_threshold(threshold), m_triggerAbove(triggerAbove), m_isTriggered(false) {}

std::optional<LogEntry> ValueThresholdTrigger::process(LogEntry entry) {
    if (std::holds_alternative<DataSample>(entry.payload)) {
        const DataSample& ds = std::get<DataSample>(entry.payload);
        if (ds.sourcePath == m_targetSourcePath) {
            double numericValue = 0.0;
            bool isNumeric = false;

            if (std::holds_alternative<double>(ds.value)) {
                numericValue = std::get<double>(ds.value);
                isNumeric = true;
            } else if (std::holds_alternative<int64_t>(ds.value)) {
                numericValue = static_cast<double>(std::get<int64_t>(ds.value));
                isNumeric = true;
            }
            
            if (isNumeric) {
                if (m_triggerAbove) {
                    m_isTriggered = (numericValue > m_threshold);
                } else {
                    m_isTriggered = (numericValue < m_threshold);
                }
            }
        }
    }
    
    if (m_isTriggered) {
        return entry;
    }
    return std::nullopt;
}

} // namespace quasar::datalogger
