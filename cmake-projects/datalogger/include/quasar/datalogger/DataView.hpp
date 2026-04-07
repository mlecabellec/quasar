#pragma once

#include "quasar/datalogger/IDataAccessor.hpp"
#include "quasar/named/NamedObject.hpp"
#include <memory>

namespace quasar::datalogger {

/**
 * @brief Concrete implementation of IDataAccessor wrapping a NamedObject.
 */
class DataView : public IDataAccessor {
public:
    /**
     * @brief Constructs a DataView for a specific NamedObject.
     * @param target The target object to sample.
     */
    explicit DataView(std::shared_ptr<quasar::named::NamedObject> target);

    [[nodiscard]] std::optional<DataSample> sample() const override;

private:
    std::shared_ptr<quasar::named::NamedObject> m_target;
};

} // namespace quasar::datalogger
