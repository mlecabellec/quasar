#pragma once

#include "quasar/datalogger/IFilter.hpp"
#include <string>

namespace quasar::datalogger {

/**
 * @brief Gates data logging based on whether a specific data value exceeds a threshold.
 */
class ValueThresholdTrigger : public IFilter {
public:
    /**
     * @brief Constructs a new ValueThresholdTrigger.
     * @param targetSourcePath The path of the DataSample to monitor.
     * @param threshold The value threshold to cross.
     * @param triggerAbove If true, triggers when value > threshold. If false, triggers when value < threshold.
     */
    ValueThresholdTrigger(const std::string& targetSourcePath, double threshold, bool triggerAbove);

    /**
     * @brief Processes the entry, blocking it if the trigger is not active.
     * @param entry The log entry to process.
     * @return The entry if triggered, or nullopt if blocked.
     */
    [[nodiscard]] std::optional<LogEntry> process(LogEntry entry) override;

private:
    std::string m_targetSourcePath;
    double m_threshold;
    bool m_triggerAbove;
    bool m_isTriggered;
};

} // namespace quasar::datalogger
