#include "quasar/datalogger/DataView.hpp"
#include <utility>

namespace quasar::datalogger {

DataView::DataView(std::shared_ptr<quasar::named::NamedObject> target)
    : m_target(std::move(target)) {
}

std::optional<DataSample> DataView::sample() const {
    if (!m_target) {
        return std::nullopt;
    }
    
    // Stub: To be properly implemented with NamedInteger, etc.
    DataSample sample_data;
    sample_data.sourcePath = m_target->getName();
    sample_data.value = 0.0; 
    return sample_data;
}

} // namespace quasar::datalogger
