#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>
#include <stdexcept>

namespace quasar::datalogger {

/**
 * @brief A thread-safe, fixed-capacity ring buffer for high-throughput data transfer.
 * 
 * Uses a timed mutex to comply with CS-0010 constraints on bounded waiting.
 */
template <typename T>
class RingBuffer {
public:
    /**
     * @brief Constructs a RingBuffer with the given capacity.
     * @param capacity The maximum number of items the buffer can hold.
     * @throws std::invalid_argument if capacity is zero.
     */
    explicit RingBuffer(size_t capacity) 
        : m_capacity(capacity), m_buffer(capacity), m_head(0), m_tail(0), m_size(0) {
        if (capacity == 0) {
            throw std::invalid_argument("Capacity must be > 0");
        }
    }

    /**
     * @brief Attempts to push an item into the buffer.
     * 
     * Overwrites the oldest data if the buffer is full.
     * 
     * @param item The item to push.
     * @return bool True if successful, false if the mutex could not be acquired.
     */
    bool push(const T& item) {
        std::unique_lock<std::timed_mutex> lock(m_mutex, std::defer_lock);
        // Wait up to 10ms to acquire the lock
        if (!lock.try_lock_for(std::chrono::milliseconds(10))) {
            return false;
        }
        
        if (m_size == m_capacity) {
            // Overwrite oldest data
            m_buffer[m_head] = item;
            m_head = (m_head + 1) % m_capacity;
            m_tail = (m_tail + 1) % m_capacity;
        } else {
            // Insert new data
            m_buffer[m_tail] = item;
            m_tail = (m_tail + 1) % m_capacity;
            ++m_size;
        }
        
        m_cv.notify_one();
        return true;
    }

    /**
     * @brief Attempts to push an item into the buffer using move semantics.
     * 
     * Overwrites the oldest data if the buffer is full.
     * 
     * @param item The item to push.
     * @return bool True if successful, false if the mutex could not be acquired.
     */
    bool push(T&& item) {
        std::unique_lock<std::timed_mutex> lock(m_mutex, std::defer_lock);
        // Wait up to 10ms to acquire the lock
        if (!lock.try_lock_for(std::chrono::milliseconds(10))) {
            return false;
        }
        
        if (m_size == m_capacity) {
            // Overwrite oldest data
            m_buffer[m_head] = std::move(item);
            m_head = (m_head + 1) % m_capacity;
            m_tail = (m_tail + 1) % m_capacity;
        } else {
            // Insert new data
            m_buffer[m_tail] = std::move(item);
            m_tail = (m_tail + 1) % m_capacity;
            ++m_size;
        }
        
        m_cv.notify_one();
        return true;
    }

    /**
     * @brief Pops an item from the buffer, waiting up to the specified timeout.
     * @param timeout The maximum duration to wait for an item.
     * @return An optional item, or nullopt if a timeout occurred or mutex failed.
     */
    std::optional<T> pop(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
        std::unique_lock<std::timed_mutex> lock(m_mutex, std::defer_lock);
        // Initial lock acquisition
        if (!lock.try_lock_for(timeout)) {
            return std::nullopt;
        }

        if (m_size == 0) {
            if (timeout == std::chrono::milliseconds(0)) {
                return std::nullopt;
            }
            // Wait for data
            if (!m_cv.wait_for(lock, timeout, [this]() { return m_size > 0; })) {
                return std::nullopt;
            }
        }

        T item = std::move(m_buffer[m_head]);
        m_head = (m_head + 1) % m_capacity;
        --m_size;
        return item;
    }

    /**
     * @brief Gets the current number of items in the buffer.
     * @return size_t The number of items, or 0 if the mutex could not be acquired.
     */
    size_t size() const {
        std::unique_lock<std::timed_mutex> lock(m_mutex, std::defer_lock);
        if (lock.try_lock_for(std::chrono::milliseconds(10))) {
            return m_size;
        }
        return 0;
    }

    /**
     * @brief Gets the maximum capacity of the buffer.
     * @return size_t The maximum capacity.
     */
    size_t capacity() const {
        return m_capacity;
    }

private:
    size_t m_capacity;
    std::vector<T> m_buffer;
    size_t m_head;
    size_t m_tail;
    size_t m_size;
    mutable std::timed_mutex m_mutex;
    std::condition_variable_any m_cv;
};

} // namespace quasar::datalogger
