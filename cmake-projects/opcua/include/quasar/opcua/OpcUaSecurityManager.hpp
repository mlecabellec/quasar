#ifndef QUASAR_OPCUA_OPCUASECURITYMANAGER_HPP
#define QUASAR_OPCUA_OPCUASECURITYMANAGER_HPP

#include <open62541/types.h>
#include <open62541/server.h>
#include <open62541/client.h>
#include <string>
#include <vector>

namespace quasar::opcua {

/**
 * @class OpcUaSecurityManager
 * @brief Manages X.509 certificates and security policies for OPC UA.
 * 
 * Fulfills [TSK-20260311-005.4] Security and Authentication.
 */
class OpcUaSecurityManager {
public:
    OpcUaSecurityManager() = default;
    ~OpcUaSecurityManager();

    /**
     * @brief Loads a certificate and private key from files.
     * @param certPath Path to the X.509 certificate (DER format).
     * @param keyPath Path to the private key (DER format).
     * @return UA_STATUSCODE_GOOD on success.
     */
    UA_StatusCode loadCertificate(const std::string& certPath, const std::string& keyPath);

    /**
     * @brief Loads trust list certificates.
     * @param trustListPaths List of paths to trusted certificates (DER format).
     * @return UA_STATUSCODE_GOOD on success.
     */
    UA_StatusCode loadTrustList(const std::vector<std::string>& trustListPaths);

    /**
     * @brief Configures a server with the loaded security settings.
     * @param server The server to configure.
     * @param config The server configuration.
     * @return UA_STATUSCODE_GOOD on success.
     */
    UA_StatusCode configureServer(UA_Server* server, UA_ServerConfig* config);

    /**
     * @brief Configures a client with the loaded security settings.
     * @param client The client to configure.
     * @return UA_STATUSCODE_GOOD on success.
     */
    UA_StatusCode configureClient(UA_Client* client);

    /**
     * @brief Generates a self-signed certificate if none exists.
     * @param outputCertPath Path where to save the certificate.
     * @param outputKeyPath Path where to save the private key.
     * @return UA_STATUSCODE_GOOD on success.
     */
    UA_StatusCode generateSelfSigned(const std::string& outputCertPath, const std::string& outputKeyPath);

private:
    UA_ByteString m_certificate = UA_BYTESTRING_NULL;
    UA_ByteString m_privateKey = UA_BYTESTRING_NULL;
    std::vector<UA_ByteString> m_trustList;
    std::vector<UA_ByteString> m_revocationList;

    void clear();
};

} // namespace quasar::opcua

#endif // QUASAR_OPCUA_OPCUASECURITYMANAGER_HPP
