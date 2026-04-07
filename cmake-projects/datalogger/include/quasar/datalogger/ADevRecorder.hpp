#pragma once

#include "quasar/datalogger/IRecorder.hpp"
#include "quasar/named/NamedObject.hpp"
#include <string>
#include <memory>

namespace quasar::datalogger {

/**
 * @brief Abstract base class for device recorders, bridging IRecorder and NamedObject.
 */
class ADevRecorder : public quasar::named::NamedObject, public IRecorder {
public:
    /**
     * @brief Constructs a new ADevRecorder.
     * @param name The name of the recorder object.
     */
    explicit ADevRecorder(const std::string& name);
    
    /**
     * @brief Virtual destructor.
     */
    ~ADevRecorder() override = default;
};

} // namespace quasar::datalogger
