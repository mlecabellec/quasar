#ifndef QUASAR_NAMED_NAMEDINTEGER_H
#define QUASAR_NAMED_NAMEDINTEGER_H

#include "quasar/named/NamedObject.h"
#include "quasar/coretypes/Integer.hpp"

namespace quasar::named {

template <typename T>
class NamedInteger : public NamedObject, public quasar::coretypes::Integer<T> {
public:
    virtual ~NamedInteger() = default;

    static std::shared_ptr<NamedInteger<T>> create(const std::string& name, T value, std::shared_ptr<NamedObject> parent = nullptr) {
        auto obj = std::make_shared<NamedInteger<T>>(name, value);
        if (parent) {
            obj->setParent(parent);
        }
        return obj;
    }

    std::shared_ptr<NamedObject> clone() const override {
        return create(this->getName(), this->value());
    }

    NamedInteger(const std::string& name, T value)
        : NamedObject(name), quasar::coretypes::Integer<T>(value) {}
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDINTEGER_H
