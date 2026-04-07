#include "quasar/datalogger/ADevRecorder.hpp"

namespace quasar::datalogger {

ADevRecorder::ADevRecorder(const std::string& name, std::shared_ptr<quasar::named::NamedObject> parent)
    : quasar::named::NamedObject(name) {
    if (parent) {
        setParent(parent);
    }
}

} // namespace quasar::datalogger
