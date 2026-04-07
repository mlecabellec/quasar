#ifndef QUASAR_OPCUA_OPCUASERVERSERVICE_HPP
#define QUASAR_OPCUA_OPCUASERVERSERVICE_HPP

#include "quasar/named/NamedService.hpp"
#include <open62541/server.h>
#include <open62541/plugin/log_stdout.h>
#include <memory>
#include <map>

namespace quasar::opcua {

/**
 * @class OpcUaServerService
 * @brief NamedService that exposes a NamedObject tree via OPC UA.
 * 
 * Fulfills [TSK-20260311-005.2] OPC UA Server as NamedService.
 */
class OpcUaServerService : public named::NamedService {
public:
    /**
     * @brief Factory method.
     * @param name Name of the service.
     * @param parent Optional parent.
     * @return Shared pointer to the new service.
     */
    static std::shared_ptr<OpcUaServerService> create(const std::string& name, std::shared_ptr<named::NamedObject> parent = nullptr);

    /**
     * @brief Destructor.
     */
    virtual ~OpcUaServerService();

    /**
     * @brief Sets the TCP port for the OPC UA server.
     * @param port The port number.
     */
    void setPort(uint16_t port);

    /**
     * @brief Sets the root object to expose.
     * If not set, the service itself is the root.
     * @param root The root object.
     */
    void setRootObject(std::shared_ptr<named::NamedObject> root);

    /**
     * @brief Starts the service.
     */
    void start() override;

    /**
     * @brief Stops the service.
     */
    void stop() override;

    /**
     * @brief Gets the type name.
     * @return "OpcUaServerService".
     */
    std::string getType() const override;

protected:
    /**
     * @brief Constructor.
     * @param name Name of the service.
     */
    explicit OpcUaServerService(const std::string& name);

    /**
     * @brief Initializes the OPC UA server and maps the tree.
     */
    void initializeOpcUa();

    /**
     * @brief Main loop for the server thread.
     */
    void runServer();

    /**
     * @brief Recursively maps a NamedObject tree to OPC UA nodes.
     * @param obj The NamedObject to map.
     * @param parentNodeId The OPC UA parent node ID.
     */
    void mapObject(std::shared_ptr<named::NamedObject> obj, UA_NodeId parentNodeId);

    /**
     * @brief Internal callback for object changes.
     * @param obj The object that changed.
     */
    void handleObjectChanged(std::shared_ptr<named::NamedObject> obj);

private:
    /** @brief OPC UA server instance. */
    UA_Server* m_server{nullptr};
    /** @brief Configuration for the server. */
    UA_ServerConfig* m_config{nullptr};
    /** @brief Port number. */
    uint16_t m_port{4840};
    /** @brief The root object to expose. */
    std::shared_ptr<named::NamedObject> m_rootObject;
    /** @brief Map from NamedObject to OPC UA NodeId. */
    std::map<std::shared_ptr<named::NamedObject>, UA_NodeId> m_objectToNodeMap;
    /** @brief Map from OPC UA NodeId to NamedObject (for callbacks). */
    // NodeId comparison is tricky, better use a custom comparator or string representation if needed.
    // For now we will use a simple mapping if possible.
    
    /** @brief Helper to track changes. */
    class TreeObserver;
    
    std::shared_ptr<TreeObserver> m_observer;

    /** @brief Static callbacks for open62541 */
    static void onWrite(UA_Server *server, const UA_NodeId *sessionId,
                        void *sessionContext, const UA_NodeId *nodeId,
                        void *nodeContext, const UA_NumericRange *range,
                        const UA_DataValue *data);


    static UA_StatusCode onMethodCall(UA_Server *server, const UA_NodeId *sessionId,
                                      void *sessionContext, const UA_NodeId *methodId,
                                      void *methodContext, const UA_NodeId *objectId,
                                      void *objectContext, size_t inputSize,
                                      const UA_Variant *input, size_t outputSize,
                                      UA_Variant *output);
};



} // namespace quasar::opcua

#endif // QUASAR_OPCUA_OPCUASERVERSERVICE_HPP
