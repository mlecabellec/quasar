#include "quasar/datalogger/MathFilter.hpp"

namespace quasar::datalogger {

MathFilter::MathFilter(const std::string& targetSourcePath, double scale, double offset)
    : m_targetSourcePath(targetSourcePath), m_scale(scale), m_offset(offset) {}

std::optional<LogEntry> MathFilter::process(LogEntry entry) {
    if (std::holds_alternative<DataSample>(entry.payload)) {
        auto& ds = std::get<DataSample>(entry.payload);
        if (ds.sourcePath == m_targetSourcePath) {
            if (std::holds_alternative<double>(ds.value)) {
                ds.value = std::get<double>(ds.value) * m_scale + m_offset;
            } else if (std::holds_alternative<int64_t>(ds.value)) {
                ds.value = static_cast<double>(std::get<int64_t>(ds.value)) * m_scale + m_offset;
            }
        }
    }
    return entry;
}

} // namespace quasar::datalogger
