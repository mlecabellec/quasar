#include "quasar/named/WebNamedMethod.hpp"

namespace quasar::named {

std::shared_ptr<WebNamedMethod> WebNamedMethod::create(
    const std::string& name,
    MethodType method,
    const std::string& httpVerb,
    const std::string& alias,
    const std::string& oasSummary,
    std::shared_ptr<datacodec::ContainerDef> inputSchema,
    std::shared_ptr<datacodec::ContainerDef> outputSchema,
    std::shared_ptr<quasar::named::NamedObject> parent) 
{
    // [CS-0010.44] Factory helper for shared_ptr instantiation.
    struct Helper : public WebNamedMethod {
        Helper(const std::string& n, MethodType m, const std::string& v, const std::string& a, const std::string& oas, 
               std::shared_ptr<datacodec::ContainerDef> in, std::shared_ptr<datacodec::ContainerDef> out)
            : WebNamedMethod(n, m, v, a, oas, in, out) {}
    };

    // Instantiate and initialize weak self reference.
    std::shared_ptr<WebNamedMethod> obj = std::make_shared<Helper>(name, method, httpVerb, alias, oasSummary, inputSchema, outputSchema);
    obj->setSelf(obj);
    
    // Attach to hierarchy if parent provided.
    if (parent) {
        obj->setParent(parent);
    }
    return obj;
}

WebNamedMethod::WebNamedMethod(
    const std::string& name,
    MethodType method,
    const std::string& httpVerb,
    const std::string& alias,
    const std::string& oasSummary,
    std::shared_ptr<datacodec::ContainerDef> inputSchema,
    std::shared_ptr<datacodec::ContainerDef> outputSchema)
    : quasar::named::TypedNamedMethod(name, method, inputSchema, outputSchema),
      m_httpVerb(httpVerb), m_alias(alias), m_oasSummary(oasSummary) 
{
    // [CS-0010.32] Explicit initialization in constructor initializer list.
}

std::string WebNamedMethod::getType() const {
    return "WebNamedMethod";
}

std::string WebNamedMethod::getHttpVerb() const {
    // [CS-0010.46] Guarded access to shared string field.
    std::lock_guard<std::recursive_timed_mutex> lock(m_mutex);
    return m_httpVerb;
}

void WebNamedMethod::setHttpVerb(const std::string& verb) {
    // [CS-0010.46] Guarded modification with structural tracking.
    std::lock_guard<std::recursive_timed_mutex> lock(m_mutex);
    m_httpVerb = verb;
    incrementTreeVersion();
    notifyObservers(getSelf());
}

std::string WebNamedMethod::getAlias() const {
    // [CS-0010.46] Guarded access to URI alias.
    std::lock_guard<std::recursive_timed_mutex> lock(m_mutex);
    return m_alias;
}

void WebNamedMethod::setAlias(const std::string& alias) {
    // [CS-0010.46] Guarded modification of URI alias.
    std::lock_guard<std::recursive_timed_mutex> lock(m_mutex);
    m_alias = alias;
    incrementTreeVersion();
    notifyObservers(getSelf());
}

std::string WebNamedMethod::getOasSummary() const {
    // [CS-0010.46] Guarded access to OpenAPI summary.
    std::lock_guard<std::recursive_timed_mutex> lock(m_mutex);
    return m_oasSummary;
}

void WebNamedMethod::setOasSummary(const std::string& summary) {
    // [CS-0010.46] Guarded modification of documentation metadata.
    std::lock_guard<std::recursive_timed_mutex> lock(m_mutex);
    m_oasSummary = summary;
    incrementTreeVersion();
    notifyObservers(getSelf());
}

} // namespace quasar::named
