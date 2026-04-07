#ifndef QUASAR_OPCUA_OPCUACLIENTSERVICE_HPP
#define QUASAR_OPCUA_OPCUACLIENTSERVICE_HPP

#include "quasar/named/NamedService.hpp"
#include <open62541/client.h>
#include <open62541/client_config_default.h>
#include <open62541/client_subscriptions.h>
#include <memory>
#include <map>

namespace quasar::opcua {

/**
 * @class OpcUaClientService
 * @brief NamedService that mirrors a remote OPC UA tree locally.
 * 
 * Fulfills [TSK-20260311-005.3] OPC UA Client as NamedService.
 */
class OpcUaClientService : public named::NamedService {
public:
    /**
     * @brief Factory method.
     * @param name Name of the service.
     * @param parent Optional parent.
     * @return Shared pointer to the new service.
     */
    static std::shared_ptr<OpcUaClientService> create(const std::string& name, std::shared_ptr<named::NamedObject> parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~OpcUaClientService();

    /**
     * @brief Sets the remote server endpoint URL.
     * @param url The endpoint URL (e.g., "opc.tcp://localhost:4840").
     */
    void setUrl(const std::string& url);

    /**
     * @brief Starts the client and connection.
     */
    void start() override;

    /**
     * @brief Stops the client and connection.
     */
    void stop() override;

    /**
     * @brief Gets the type name.
     * @return "OpcUaClientService".
     */
    std::string getType() const override;

protected:
    /**
     * @brief Constructor.
     * @param name Name of the service.
     */
    explicit OpcUaClientService(const std::string& name);

    /**
     * @brief Initializes the client and browses the remote tree.
     */
    void initializeClient();

    /**
     * @brief Recursively browses the remote server and creates local objects.
     * @param remoteNodeId The ID of the remote node to browse.
     * @param localParent The local parent object to attach to.
     */
    void browseAndMirror(UA_NodeId remoteNodeId, std::shared_ptr<named::NamedObject> localParent);

private:
    /** @brief OPC UA client instance. */
    UA_Client* m_client{nullptr};
    /** @brief Server URL. */
    std::string m_url{"opc.tcp://localhost:4840"};
    
    /** @brief Mapping from remote NodeId to local NamedObject. */
    // Using string representation of NodeId for simpler map key
    std::map<std::string, std::shared_ptr<named::NamedObject>> m_nodeToLocalMap;

    /** @brief Callback for data change notifications. */
    static void onDataChange(UA_Client *client, UA_UInt32 subId, void *subContext,
                             UA_UInt32 monId, void *monContext, UA_DataValue *value);
};

} // namespace quasar::opcua

#endif // QUASAR_OPCUA_OPCUACLIENTSERVICE_HPP
