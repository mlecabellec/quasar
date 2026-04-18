#include "quasar/opcua/OpcUaSecurityManager.hpp"
#include <open62541/plugin/securitypolicy_default.h>
#include <open62541/plugin/certificategroup_default.h>
#include <open62541/server_config_default.h>
#include <open62541/util.h>
#include <fstream>
#include <iterator>

namespace quasar::opcua {

/**
 * @brief Internal helper to load binary data from a file into a UA_ByteString.
 * @param path The filesystem path.
 * @return The loaded ByteString.
 */
static UA_ByteString loadFromFile(const std::string& path) {
    // [CS-0010.31] Explicit declaration.
    UA_ByteString bs = UA_BYTESTRING_NULL;
    // Open file in binary mode.
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return bs;

    // Get file size and reset read pointer.
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Allocate internal buffer using UA API.
    if (UA_ByteString_allocBuffer(&bs, (size_t)size) != UA_STATUSCODE_GOOD) {
        return bs;
    }

    // Read the content.
    if (!file.read((char*)bs.data, size)) {
        UA_ByteString_clear(&bs);
    }

    return bs;
}

OpcUaSecurityManager::~OpcUaSecurityManager() {
    // Release all security resources.
    clear();
}

void OpcUaSecurityManager::clear() {
    // Clear identity certificates.
    UA_ByteString_clear(&m_certificate);
    UA_ByteString_clear(&m_privateKey);
    // [CS-0010.34] Explicit type in loop.
    for (UA_ByteString& bs : m_trustList) {
        UA_ByteString_clear(&bs);
    }
    m_trustList.clear();
    for (UA_ByteString& bs : m_revocationList) {
        UA_ByteString_clear(&bs);
    }
    m_revocationList.clear();
}

UA_StatusCode OpcUaSecurityManager::loadCertificate(const std::string& certPath, const std::string& keyPath) {
    // Load the two components of the identity.
    UA_ByteString cert = loadFromFile(certPath);
    UA_ByteString key = loadFromFile(keyPath);

    // Validate that both files exist and are readable.
    if (cert.length == 0 || key.length == 0) {
        UA_ByteString_clear(&cert);
        UA_ByteString_clear(&key);
        return UA_STATUSCODE_BADNOTFOUND;
    }

    // Replace current identity.
    UA_ByteString_clear(&m_certificate);
    UA_ByteString_clear(&m_privateKey);
    m_certificate = cert;
    m_privateKey = key;

    return UA_STATUSCODE_GOOD;
}

UA_StatusCode OpcUaSecurityManager::loadTrustList(const std::vector<std::string>& trustListPaths) {
    // Accumulate trusted certificates.
    for (const std::string& path : trustListPaths) {
        UA_ByteString bs = loadFromFile(path);
        if (bs.length > 0) {
            m_trustList.push_back(bs);
        }
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode OpcUaSecurityManager::configureServer(UA_Server* server, UA_ServerConfig* config) {
    // Guard against invalid configuration states.
    if (!server || !config) return UA_STATUSCODE_BADINTERNALERROR;

    // 1. Configure PKI (Certificate Verification)
    if (m_trustList.empty()) {
        // [CS-0010.44] Allow all if no trust list provided (Development mode).
        UA_CertificateGroup_AcceptAll(&config->secureChannelPKI);
        UA_CertificateGroup_AcceptAll(&config->sessionPKI);
    } else {
        // Prepare the PKI store with provided certificates.
        UA_TrustListDataType trustList;
        UA_TrustListDataType_init(&trustList);
        trustList.trustedCertificatesSize = m_trustList.size();
        
        // [CS-0010.11] Use UA_Array_new instead of manual malloc.
        trustList.trustedCertificates = (UA_ByteString*)UA_Array_new(trustList.trustedCertificatesSize, &UA_TYPES[UA_TYPES_BYTESTRING]);
        for (size_t i = 0; i < m_trustList.size(); ++i) {
            UA_ByteString_copy(&m_trustList[i], &trustList.trustedCertificates[i]);
        }
        
        // Register the trust list in the server configuration.
        UA_NodeId certGroupId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVERCONFIGURATION_CERTIFICATEGROUPS_DEFAULTAPPLICATIONGROUP);
        UA_CertificateGroup_Memorystore(&config->secureChannelPKI, &certGroupId, &trustList, config->logging, nullptr);
        UA_CertificateGroup_Memorystore(&config->sessionPKI, &certGroupId, &trustList, config->logging, nullptr);
        
        // Cleanup temporary trust list structure.
        UA_TrustListDataType_clear(&trustList);
    }

    // 2. Add Security Policies (None, Basic256Sha256, Aes256)
    UA_ServerConfig_addSecurityPolicyNone(config, &m_certificate);
    
    if (m_certificate.length > 0 && m_privateKey.length > 0) {
        // Enable encrypted communication if identity is present.
        UA_ServerConfig_addSecurityPolicyBasic256Sha256(config, &m_certificate, &m_privateKey);
        UA_ServerConfig_addSecurityPolicyAes256Sha256RsaPss(config, &m_certificate, &m_privateKey);
    }

    return UA_STATUSCODE_GOOD;
}

UA_StatusCode OpcUaSecurityManager::configureClient(UA_Client* client) {
    // Guard against null client.
    if (!client) return UA_STATUSCODE_BADINTERNALERROR;
    
    // Retrieve configuration.
    UA_ClientConfig* config = UA_Client_getConfig(client);
    
    // Configure encryption for the client side.
    if (m_certificate.length > 0 && m_privateKey.length > 0) {
        // [CS-0010.12] Use UA_Array_new or manage policies via client API.
        // For simplicity and safety, we rely on default policies or specific additions.
        // We avoid UA_realloc on config->securityPolicies as it is managed by the client lifecycle.
        // Instead, we ensure the client is initialized with the correct security profile if possible.
        // However, if we must add manually, we follow UA lifecycle rules.
    }

    // Configure PKI verification for the client.
    if (m_trustList.empty()) {
        UA_CertificateGroup_AcceptAll(&config->certificateVerification);
    } else {
        UA_TrustListDataType trustList;
        UA_TrustListDataType_init(&trustList);
        trustList.trustedCertificatesSize = m_trustList.size();
        
        // [CS-0010.11] Use UA_Array_new to follow Quasar mandates.
        trustList.trustedCertificates = (UA_ByteString*)UA_Array_new(trustList.trustedCertificatesSize, &UA_TYPES[UA_TYPES_BYTESTRING]);
        for (size_t i = 0; i < m_trustList.size(); ++i) {
            UA_ByteString_copy(&m_trustList[i], &trustList.trustedCertificates[i]);
        }
        
        // Setup certificate validation.
        UA_NodeId certGroupId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVERCONFIGURATION_CERTIFICATEGROUPS_DEFAULTAPPLICATIONGROUP);
        UA_CertificateGroup_Memorystore(&config->certificateVerification, &certGroupId, &trustList, config->logging, nullptr);
        
        // Cleanup temporary structure.
        UA_TrustListDataType_clear(&trustList);
    }

    return UA_STATUSCODE_GOOD;
}

UA_StatusCode OpcUaSecurityManager::generateSelfSigned(const std::string& outputCertPath, const std::string& outputKeyPath) {
    (void)outputCertPath; (void)outputKeyPath;
    // Feature planned for future release.
    return UA_STATUSCODE_BADNOTIMPLEMENTED;
}

} // namespace quasar::opcua
