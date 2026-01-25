#ifndef QUASAR_NAMED_NAMEDFLOATINGPOINT_H
#define QUASAR_NAMED_NAMEDFLOATINGPOINT_H

#include "quasar/named/NamedObject.h"
#include "quasar/coretypes/FloatingPoint.hpp"

namespace quasar::named {

template <typename T>
class NamedFloatingPoint : public NamedObject, public quasar::coretypes::FloatingPoint<T> {
public:
    virtual ~NamedFloatingPoint() = default;

    static std::shared_ptr<NamedFloatingPoint<T>> create(const std::string& name, T value, std::shared_ptr<NamedObject> parent = nullptr) {
        auto obj = std::make_shared<NamedFloatingPoint<T>>(name, value);
        if (parent) {
            obj->setParent(parent);
        }
        return obj;
    }

    std::shared_ptr<NamedObject> clone() const override {
        return create(this->getName(), this->value());
    }

    NamedFloatingPoint(const std::string& name, T value)
        : NamedObject(name), quasar::coretypes::FloatingPoint<T>(value) {}
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDFLOATINGPOINT_H
