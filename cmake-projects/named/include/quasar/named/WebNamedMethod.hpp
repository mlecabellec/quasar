#ifndef QUASAR_NAMED_WEBNAMEDMETHOD_HPP
#define QUASAR_NAMED_WEBNAMEDMETHOD_HPP

#include "quasar/named/TypedNamedMethod.hpp"
#include <string>

namespace sol {
    class state_view;
}

namespace quasar::named {

/**
 * @class WebNamedMethod
 * @brief Extension of TypedNamedMethod that includes HTTP/OpenAPI metadata.
 * 
 * Supports the Induced API by allowing direct mapping of RESTful verbs and URI aliases 
 * to reflexive executable logic.
 * 
 * @feature TSK-20260311-008 Web API Routing
 * @exposed
 */
class WebNamedMethod : public quasar::named::TypedNamedMethod {
public:
    /**
     * @brief Signature for the method implementation.
     * @exposed
     */
    using MethodType = quasar::named::NamedMethod::MethodType;

    /**
     * @brief Factory method.
     * @param name Name of the method.
     * @param method Execution logic.
     * @param httpVerb The HTTP method (GET, POST, PUT, DELETE).
     * @param alias The custom URI routing alias (e.g., "/api/custom/action").
     * @param oasSummary OpenAPI summary string.
     * @param inputSchema Optional input arguments schema.
     * @param outputSchema Optional return value schema.
     * @param parent Optional parent node.
     * @return Shared pointer to the new WebNamedMethod.
     * @exposed
     */
    [[nodiscard]] static std::shared_ptr<WebNamedMethod> create(
        const std::string& name,
        MethodType method,
        const std::string& httpVerb = "POST",
        const std::string& alias = "",
        const std::string& oasSummary = "",
        std::shared_ptr<datacodec::ContainerDef> inputSchema = nullptr,
        std::shared_ptr<datacodec::ContainerDef> outputSchema = nullptr,
        std::shared_ptr<quasar::named::NamedObject> parent = nullptr
    );

    /** 
     * @brief Returns "WebNamedMethod" 
     * @exposed
     */
    [[nodiscard]] std::string getType() const override;

    /** 
     * @brief Gets the HTTP verb. 
     * @exposed
     */
    [[nodiscard]] std::string getHttpVerb() const;

    /** 
     * @brief Sets the HTTP verb. 
     * @exposed
     */
    void setHttpVerb(const std::string& verb);

    /** 
     * @brief Gets the URI alias. 
     * @exposed
     */
    [[nodiscard]] std::string getAlias() const;

    /** 
     * @brief Sets the URI alias. 
     * @exposed
     */
    void setAlias(const std::string& alias);

    /** 
     * @brief Gets the OpenAPI summary. 
     * @exposed
     */
    [[nodiscard]] std::string getOasSummary() const;

    /** 
     * @brief Sets the OpenAPI summary. 
     * @exposed
     */
    void setOasSummary(const std::string& summary);

protected:
    /**
     * @brief Constructor.
     */
    WebNamedMethod(
        const std::string& name,
        MethodType method,
        const std::string& httpVerb,
        const std::string& alias,
        const std::string& oasSummary,
        std::shared_ptr<datacodec::ContainerDef> inputSchema,
        std::shared_ptr<datacodec::ContainerDef> outputSchema
    );

private:
    /** @brief The associated HTTP method. */
    std::string m_httpVerb;
    /** @brief Optional URI path alias. */
    std::string m_alias;
    /** @brief Human-readable summary for documentation. */
    std::string m_oasSummary;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_WEBNAMEDMETHOD_HPP
