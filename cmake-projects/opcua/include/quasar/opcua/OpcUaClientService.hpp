#ifndef QUASAR_OPCUA_OPCUACLIENTSERVICE_HPP
#define QUASAR_OPCUA_OPCUACLIENTSERVICE_HPP

#include "quasar/named/NamedService.hpp"
#include <open62541/client.h>
#include <open62541/client_config_default.h>
#include <open62541/client_subscriptions.h>
#include <memory>
#include <map>
#include <queue>
#include <functional>
#include <future>
#include <mutex>

namespace quasar::opcua {

/**
 * @class OpcUaClientService
 * @brief NamedService that mirrors a remote OPC UA tree locally.
 * 
 * Fulfills [TSK-20260311-005.3] OPC UA Client as NamedService.
 * Handles thread-safety by queuing all client operations to its service thread.
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

    /**
     * @brief Queues a task to be executed on the client thread.
     * @param task The task to execute.
     * @return A future that will contain the task result.
     */
    template<typename T>
    std::future<T> enqueueTask(std::function<T(UA_Client*)> task) {
        auto promise = std::make_shared<std::promise<T>>();
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_tasks.push([task, promise](UA_Client* client) {
                try {
                    promise->set_value(task(client));
                } catch (...) {
                    promise->set_exception(std::current_exception());
                }
            });
        }
        return promise->get_future();
    }

    /** @brief Specialization for void tasks. */
    std::future<void> enqueueTask(std::function<void(UA_Client*)> task) {
        auto promise = std::make_shared<std::promise<void>>();
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_tasks.push([task, promise](UA_Client* client) {
                try {
                    task(client);
                    promise->set_value();
                } catch (...) {
                    promise->set_exception(std::current_exception());
                }
            });
        }
        return promise->get_future();
    }

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

    /** @brief Processes all pending tasks in the queue. */
    void processTasks();

private:
    /** @brief OPC UA client instance. */
    UA_Client* m_client{nullptr};
    /** @brief Server URL. */
    std::string m_url{"opc.tcp://localhost:4840"};
    
    /** @brief Mapping from remote NodeId to local NamedObject. */
    std::map<std::string, std::shared_ptr<named::NamedObject>> m_nodeToLocalMap;

    /** @brief Mutex for the task queue. */
    std::mutex m_queueMutex;
    /** @brief Task queue for cross-thread client calls. */
    std::queue<std::function<void(UA_Client*)>> m_tasks;

    /** @brief Callback for data change notifications. */
    static void onDataChange(UA_Client *client, UA_UInt32 subId, void *subContext,
                             UA_UInt32 monId, void *monContext, UA_DataValue *value);
};

} // namespace quasar::opcua


#endif // QUASAR_OPCUA_OPCUACLIENTSERVICE_HPP
