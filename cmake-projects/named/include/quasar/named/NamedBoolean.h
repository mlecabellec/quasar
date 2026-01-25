#ifndef QUASAR_NAMED_NAMEDBOOLEAN_H
#define QUASAR_NAMED_NAMEDBOOLEAN_H

#include "quasar/named/NamedObject.h"
#include "quasar/coretypes/Boolean.hpp"

namespace quasar::named {

class NamedBoolean : public NamedObject, public quasar::coretypes::Boolean {
public:
    virtual ~NamedBoolean() = default;

    /**
     * @brief Creates a new NamedBoolean.
     * @param name The name of the object.
     * @param value The boolean value.
     * @param parent The optional parent of the object.
     * @return A shared_ptr to the created object.
     */
    static std::shared_ptr<NamedBoolean> create(const std::string& name, bool value, std::shared_ptr<NamedObject> parent = nullptr);

    std::shared_ptr<NamedObject> clone() const override {
        return create(getName(), booleanValue());
    }

    NamedBoolean(const std::string& name, bool value);
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDBOOLEAN_H
