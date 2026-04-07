#include "quasar/opcua/OpcUaSecurityManager.hpp"
#include <open62541/plugin/securitypolicy_default.h>
#include <open62541/plugin/certificategroup_default.h>
#include <open62541/server_config_default.h>
#include <open62541/util.h>
#include <fstream>
#include <iterator>

namespace quasar::opcua {

static UA_ByteString loadFromFile(const std::string& path) {
    UA_ByteString bs = UA_BYTESTRING_NULL;
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return bs;

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (UA_ByteString_allocBuffer(&bs, (size_t)size) != UA_STATUSCODE_GOOD) {
        return bs;
    }

    if (!file.read((char*)bs.data, size)) {
        UA_ByteString_clear(&bs);
    }

    return bs;
}

OpcUaSecurityManager::~OpcUaSecurityManager() {
    clear();
}

void OpcUaSecurityManager::clear() {
    UA_ByteString_clear(&m_certificate);
    UA_ByteString_clear(&m_privateKey);
    for (auto& bs : m_trustList) UA_ByteString_clear(&bs);
    m_trustList.clear();
    for (auto& bs : m_revocationList) UA_ByteString_clear(&bs);
    m_revocationList.clear();
}

UA_StatusCode OpcUaSecurityManager::loadCertificate(const std::string& certPath, const std::string& keyPath) {
    UA_ByteString cert = loadFromFile(certPath);
    UA_ByteString key = loadFromFile(keyPath);

    if (cert.length == 0 || key.length == 0) {
        UA_ByteString_clear(&cert);
        UA_ByteString_clear(&key);
        return UA_STATUSCODE_BADNOTFOUND;
    }

    UA_ByteString_clear(&m_certificate);
    UA_ByteString_clear(&m_privateKey);
    m_certificate = cert;
    m_privateKey = key;

    return UA_STATUSCODE_GOOD;
}

UA_StatusCode OpcUaSecurityManager::loadTrustList(const std::vector<std::string>& trustListPaths) {
    for (const auto& path : trustListPaths) {
        UA_ByteString bs = loadFromFile(path);
        if (bs.length > 0) {
            m_trustList.push_back(bs);
        }
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode OpcUaSecurityManager::configureServer(UA_Server* server, UA_ServerConfig* config) {
    if (!server || !config) return UA_STATUSCODE_BADINTERNALERROR;

    // 1. Configure PKI (Certificate Verification)
    if (m_trustList.empty()) {
        UA_CertificateGroup_AcceptAll(&config->secureChannelPKI);
        UA_CertificateGroup_AcceptAll(&config->sessionPKI);
    } else {
        UA_TrustListDataType trustList;
        UA_TrustListDataType_init(&trustList);
        trustList.trustedCertificatesSize = m_trustList.size();
        trustList.trustedCertificates = (UA_ByteString*)UA_malloc(sizeof(UA_ByteString) * m_trustList.size());
        for (size_t i = 0; i < m_trustList.size(); ++i) {
            UA_ByteString_copy(&m_trustList[i], &trustList.trustedCertificates[i]);
        }
        
        UA_NodeId certGroupId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_CERTIFICATEGROUP_DEFAULTAPPLICATIONGROUP);
        UA_CertificateGroup_Memorystore(&config->secureChannelPKI, &certGroupId, &trustList, config->logging, nullptr);
        UA_CertificateGroup_Memorystore(&config->sessionPKI, &certGroupId, &trustList, config->logging, nullptr);
        
        UA_TrustListDataType_clear(&trustList);
    }

    // 2. Add Security Policies
    // Always add "None" for internal use/tests if needed, but in production we might want to disable it
    UA_ServerConfig_addSecurityPolicyNone(config, &m_certificate);
    
    if (m_certificate.length > 0 && m_privateKey.length > 0) {
        UA_ServerConfig_addSecurityPolicyBasic256Sha256(config, &m_certificate, &m_privateKey);
        UA_ServerConfig_addSecurityPolicyAes256Sha256RsaPss(config, &m_certificate, &m_privateKey);
    }

    return UA_STATUSCODE_GOOD;
}

UA_StatusCode OpcUaSecurityManager::configureClient(UA_Client* client) {
    if (!client) return UA_STATUSCODE_BADINTERNALERROR;
    
    UA_ClientConfig* config = UA_Client_getConfig(client);
    
    if (m_certificate.length > 0 && m_privateKey.length > 0) {
        UA_ByteString_copy(&m_certificate, &config->clientCertificate);
        UA_ByteString_copy(&m_privateKey, &config->clientPrivateKey);
    }

    if (m_trustList.empty()) {
        UA_CertificateGroup_AcceptAll(&config->certificateVerification);
    } else {
        UA_TrustListDataType trustList;
        UA_TrustListDataType_init(&trustList);
        trustList.trustedCertificatesSize = m_trustList.size();
        trustList.trustedCertificates = (UA_ByteString*)UA_malloc(sizeof(UA_ByteString) * m_trustList.size());
        for (size_t i = 0; i < m_trustList.size(); ++i) {
            UA_ByteString_copy(&m_trustList[i], &trustList.trustedCertificates[i]);
        }
        
        UA_NodeId certGroupId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_CERTIFICATEGROUP_DEFAULTAPPLICATIONGROUP);
        UA_CertificateGroup_Memorystore(&config->certificateVerification, &certGroupId, &trustList, config->logging, nullptr);
        
        UA_TrustListDataType_clear(&trustList);
    }

    return UA_STATUSCODE_GOOD;
}

UA_StatusCode OpcUaSecurityManager::generateSelfSigned(const std::string& outputCertPath, const std::string& outputKeyPath) {
    (void)outputCertPath; (void)outputKeyPath;
    // Self-generation requires complex OpenSSL/mbedTLS calls or using open62541's tools if exposed.
    // For now, we expect certificates to be provided or generated via external scripts.
    return UA_STATUSCODE_BADNOTIMPLEMENTED;
}

} // namespace quasar::opcua
