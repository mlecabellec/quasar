#pragma once

#include "quasar/datalogger/IFilter.hpp"
#include <string>

namespace quasar::datalogger {

/**
 * @brief Multiplies numeric data by a scale and adds an offset.
 */
class MathFilter : public IFilter {
public:
    /**
     * @brief Constructs a new MathFilter.
     * @param targetSourcePath The path of the DataSample to target.
     * @param scale The multiplier.
     * @param offset The addition.
     */
    MathFilter(const std::string& targetSourcePath, double scale, double offset);

    /**
     * @brief Processes the entry, applying math scaling to targeted data samples.
     * @param entry The log entry to process.
     * @return The modified entry.
     */
    [[nodiscard]] std::optional<LogEntry> process(LogEntry entry) override;

private:
    std::string m_targetSourcePath;
    double m_scale;
    double m_offset;
};

} // namespace quasar::datalogger
