#ifndef QUASAR_NET_EVENTTRAMPOLINE_HPP
#define QUASAR_NET_EVENTTRAMPOLINE_HPP

#include <functional>
#include <mutex>
#include <vector>
#include <iostream>

namespace quasar::net {

/**
 * @class EventTrampoline
 * @brief Thread-safe queue for deferring ASIO background callbacks into the main Lua thread.
 */
class EventTrampoline {
public:
    static EventTrampoline& getInstance();

    void defer(std::function<void()> func) {
        std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(100));
        if (lock.owns_lock()) {
            m_queue.emplace_back(std::move(func));
        } else {
            std::cerr << "EventTrampoline defer timeout" << std::endl;
        }
    }

    void poll() {
        std::vector<std::function<void()>> current;
        {
            std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(100));
            if (!lock.owns_lock()) return;
            if (m_queue.empty()) return;
            current.swap(m_queue);
        }
        for (auto& func : current) {
            try {
                if (func) func();
            } catch (...) {
                // Ignore exceptions in trampoline to prevent crashing the main loop
            }
        }
    }

    void clear() {
        std::unique_lock<std::timed_mutex> lock(m_mutex, std::chrono::milliseconds(100));
        if (lock.owns_lock()) {
            m_queue.clear();
        }
    }

private:
    EventTrampoline() = default;
    EventTrampoline(const EventTrampoline&) = delete;
    EventTrampoline& operator=(const EventTrampoline&) = delete;

    std::timed_mutex m_mutex;
    std::vector<std::function<void()>> m_queue;
};

} // namespace quasar::net

#endif // QUASAR_NET_EVENTTRAMPOLINE_HPP
