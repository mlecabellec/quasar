#include "quasar/named/NamedMethod.hpp"
#include <stdexcept>

namespace quasar::named {

NamedMethod::NamedMethod(const std::string& name, MethodType method)
    : NamedObject(name), m_method(std::move(method)) {}

std::shared_ptr<NamedMethod> NamedMethod::create(const std::string& name, MethodType method, std::shared_ptr<NamedObject> parent) {
    if (!method) {
        throw std::invalid_argument("NamedMethod: method implementation cannot be null");
    }
    // [CS-0010.10] Use of new or delete keywords is forbidden.
    // Using a private struct helper to allow std::make_shared with protected constructor.
    struct make_shared_enabler : public NamedMethod {
        explicit make_shared_enabler(const std::string& n, MethodType m) : NamedMethod(n, std::move(m)) {}
    };
    std::shared_ptr<NamedMethod> self = std::make_shared<make_shared_enabler>(name, std::move(method));
    self->setSelf(self);
    if (parent) {
        self->setParent(parent);
    }
    return self;
}

std::shared_ptr<NamedObject> NamedMethod::execute(std::shared_ptr<NamedObject> args) {
    // [CS-0010.44] Execute the stored method implementation, passing the parent as owner and the provided arguments.
    return m_method(getParent(), args);
}

std::string NamedMethod::getType() const {
    return "NamedMethod";
}

std::shared_ptr<NamedObject> NamedMethod::clone(CopyPolicy policy) const {
    // [CS-0010.44] CopyPolicy is not used in this implementation as method logic is shared.
    (void)policy;
    return NamedMethod::create(getName(), m_method);
}

} // namespace quasar::named
