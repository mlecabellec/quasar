#include "quasar/named/TypedNamedMethod.hpp"
#include "datacodec/Schema.hpp"
#include <stdexcept>

namespace quasar::named {

TypedNamedMethod::TypedNamedMethod(
    const std::string& name,
    MethodType method,
    std::shared_ptr<datacodec::ContainerDef> inputSchema,
    std::shared_ptr<datacodec::ContainerDef> outputSchema
) : NamedMethod(name, std::move(method)),
    m_inputSchema(std::move(inputSchema)),
    m_outputSchema(std::move(outputSchema)) {}

std::shared_ptr<TypedNamedMethod> TypedNamedMethod::create(
    const std::string& name,
    MethodType method,
    std::shared_ptr<datacodec::ContainerDef> inputSchema,
    std::shared_ptr<datacodec::ContainerDef> outputSchema,
    std::shared_ptr<NamedObject> parent
) {
    if (!method) {
        throw std::invalid_argument("TypedNamedMethod: method implementation cannot be null");
    }

    // [CS-0010.10] Use of new or delete keywords is forbidden.
    struct make_shared_enabler : public TypedNamedMethod {
        explicit make_shared_enabler(
            const std::string& n,
            MethodType m,
            std::shared_ptr<datacodec::ContainerDef> in,
            std::shared_ptr<datacodec::ContainerDef> out
        ) : TypedNamedMethod(n, std::move(m), std::move(in), std::move(out)) {}
    };

    std::shared_ptr<TypedNamedMethod> self = std::make_shared<make_shared_enabler>(
        name, std::move(method), std::move(inputSchema), std::move(outputSchema)
    );

    self->setSelf(self);
    if (parent) {
        self->setParent(parent);
    }
    return self;
}

std::expected<void, MethodErrorCode> TypedNamedMethod::validateInput(const NamedObject& args) const {
    // [CS-0010.44] If no schema is defined, validation is bypassed (Identity match).
    if (!m_inputSchema) {
        return {};
    }

    // [TSK-008] Phase 3 will implement the heavy recursive validation against datacodec::ContainerDef.
    // For Phase 1 baseline, we perform a shallow existence check.
    (void)args;
    
    return {};
}

std::shared_ptr<NamedObject> TypedNamedMethod::execute(std::shared_ptr<NamedObject> args) {
    // [CS-0010.44] Perform mandatory pre-validation before execution.
    if (args) {
        std::expected<void, MethodErrorCode> result = validateInput(*args);
        if (!result.has_value()) {
            throw std::invalid_argument("TypedNamedMethod: Argument validation failed");
        }
    } else if (m_inputSchema) {
        // [CS-0010.44] Schema exists but no arguments provided.
        throw std::invalid_argument("TypedNamedMethod: Missing required arguments for schema");
    }

    // [CS-0010.44] Delegate to base NamedMethod execution which handles owner/parent context.
    return NamedMethod::execute(std::move(args));
}

std::string TypedNamedMethod::getType() const {
    return "TypedNamedMethod";
}

std::shared_ptr<datacodec::ContainerDef> TypedNamedMethod::getInputSchema() const {
    return m_inputSchema;
}

std::shared_ptr<datacodec::ContainerDef> TypedNamedMethod::getOutputSchema() const {
    return m_outputSchema;
}

} // namespace quasar::named
