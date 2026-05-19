#ifndef QUASAR_NET_EVENTTRAMPOLINE_HPP
#define QUASAR_NET_EVENTTRAMPOLINE_HPP

#include <functional>
#include <mutex>
#include <vector>

namespace quasar::net {

/**
 * @class EventTrampoline
 * @brief Thread-safe queue for deferring ASIO background callbacks into the main Lua thread.
 */
class EventTrampoline {
public:
    static EventTrampoline& getInstance();

    void defer(std::function<void()> func) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.emplace_back(std::move(func));
    }

    void poll() {
        std::vector<std::function<void()>> current;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_queue.empty() == true) return;
            current.swap(m_queue);
        }
        for (std::vector<std::function<void()>>::iterator it = current.begin(); it != current.end(); ++it) {
            std::function<void()>& func = *it;
            try {
                if (func != nullptr) func();
            } catch (...) {
                // Ignore exceptions in trampoline to prevent crashing the main loop
            }
        }
    }

private:
    EventTrampoline() = default;
    EventTrampoline(const EventTrampoline&) = delete;
    EventTrampoline& operator=(const EventTrampoline&) = delete;

    std::mutex m_mutex;
    std::vector<std::function<void()>> m_queue;
};

} // namespace quasar::net

#endif // QUASAR_NET_EVENTTRAMPOLINE_HPP
