#ifndef QUASAR_NAMED_TYPEDNAMEDMETHOD_HPP
#define QUASAR_NAMED_TYPEDNAMEDMETHOD_HPP

#include "quasar/named/NamedMethod.hpp"
#include <expected>
#include <system_error>

namespace datacodec {
    class ContainerDef;
}

namespace quasar::named {

/**
 * @enum MethodErrorCode
 * @brief Error codes for typed method validation.
 */
enum class MethodErrorCode {
    Success = 0,
    InvalidArguments,
    TypeMismatch,
    MissingField,
    ExecutionFailed
};

/**
 * @class TypedNamedMethod
 * @brief A NamedMethod with formal input/output signatures.
 * 
 * Provides "Heavy Consistency" checks by validating argument trees against
 * datacodec schemas before execution.
 * 
 * @reference [FE-0260.1] Reflexive Methods (NamedMethod)
 * @contribution TSK-20260328-001
 */
class TypedNamedMethod : public NamedMethod {
public:
    /**
     * @brief Factory method to create a new TypedNamedMethod.
     * @param name Name of the method.
     * @param method Implementation logic.
     * @param inputSchema Optional schema for validating arguments.
     * @param outputSchema Optional schema for validating results.
     * @param parent Optional parent node.
     * @return std::shared_ptr to the new method.
     */
    static std::shared_ptr<TypedNamedMethod> create(
        const std::string& name,
        MethodType method,
        std::shared_ptr<datacodec::ContainerDef> inputSchema = nullptr,
        std::shared_ptr<datacodec::ContainerDef> outputSchema = nullptr,
        std::shared_ptr<NamedObject> parent = nullptr
    );

    /**
     * @brief Performs "Heavy Consistency" validation on the argument tree.
     * @param args The input arguments to validate.
     * @return Success or a specific error code.
     * @compliance [CS-0020.60] Return value represents validation result.
     */
    [[nodiscard]] std::expected<void, MethodErrorCode> validateInput(const NamedObject& args) const;

    /**
     * @brief Executes the method with pre-validation.
     * @param args Input arguments.
     * @return The result tree.
     * @throws std::invalid_argument if validation fails.
     */
    std::shared_ptr<NamedObject> execute(std::shared_ptr<NamedObject> args) override;

    /** @brief Returns "TypedNamedMethod". */
    std::string getType() const override;

    /** @brief Gets the input schema. */
    std::shared_ptr<datacodec::ContainerDef> getInputSchema() const;

    /** @brief Gets the output schema. */
    std::shared_ptr<datacodec::ContainerDef> getOutputSchema() const;

protected:
    /**
     * @brief Constructor.
     * @param name Name of the method.
     * @param method Implementation logic.
     * @param inputSchema Input schema.
     * @param outputSchema Output schema.
     */
    TypedNamedMethod(
        const std::string& name,
        MethodType method,
        std::shared_ptr<datacodec::ContainerDef> inputSchema,
        std::shared_ptr<datacodec::ContainerDef> outputSchema
    );

    std::shared_ptr<datacodec::ContainerDef> m_inputSchema;
    std::shared_ptr<datacodec::ContainerDef> m_outputSchema;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_TYPEDNAMEDMETHOD_HPP
