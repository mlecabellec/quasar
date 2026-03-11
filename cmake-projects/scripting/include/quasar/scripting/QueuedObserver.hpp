#pragma once

#include "quasar/named/IObserver.hpp"
#include "quasar/scripting/LuaService.hpp"
#include "quasar/scripting/LuaProxy.hpp"
#include <sol/sol.hpp>
#include <memory>
#include <atomic>
#include <iostream>

namespace quasar::scripting {

/**
 * @class QueuedObserver
 * @brief Observer that queues notifications for a LuaService worker thread.
 * 
 * Implements a high watermark to drop events if the queue grows too large,
 * preventing memory exhaustion or extreme latency.
 */
class QueuedObserver : public named::IObserver, public std::enable_shared_from_this<QueuedObserver> {
public:
    /**
     * @brief Constructor.
     * @param service The LuaService that will process the events.
     * @param callback The Lua function to call.
     * @param watermark Max number of events in the queue before dropping.
     */
    QueuedObserver(std::shared_ptr<LuaService> service, sol::function callback, size_t watermark = 1000)
        : m_service(service), m_callback(callback), m_watermark(watermark), m_pendingCount(0) {}

    /**
     * @brief Implementation of IObserver::notify.
     * Queues the event for the Lua worker thread.
     */
    void notify(std::shared_ptr<named::NamedObject> eventData) override {
        auto service = m_service.lock();
        if (!service) return;

        if (m_pendingCount >= m_watermark) {
            // Watermark exceeded, drop event (emergency measure)
            static std::atomic<size_t> droppedCount{0};
            if (droppedCount++ % 100 == 0) {
                 std::cerr << "QueuedObserver watermark exceeded (" << m_watermark << "), dropping events. Total dropped: " << droppedCount << std::endl;
            }
            return;
        }

        m_pendingCount++;
        
        // Capture a weak pointer to self to avoid keeping the observer alive indefinitely 
        // if the service persists but the observer is detached.
        std::weak_ptr<QueuedObserver> weakSelf = shared_from_this();

        service->postTask([weakSelf, eventData]() {
            auto self = weakSelf.lock();
            if (!self) return;

            self->m_pendingCount--;

            try {
                // Call the Lua function with a proxy of the event data
                auto result = self->m_callback(LuaProxy<named::NamedObject>(eventData));
                if (!result.valid()) {
                    sol::error err = result;
                    std::cerr << "QueuedObserver Lua error: " << err.what() << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "QueuedObserver exception: " << e.what() << std::endl;
            }
        });
    }

private:
    std::weak_ptr<LuaService> m_service;
    sol::function m_callback;
    size_t m_watermark;
    std::atomic<size_t> m_pendingCount;
};

} // namespace quasar::scripting
