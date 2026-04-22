#ifndef QUASAR_NET_RECONNECTIONPOLICY_HPP
#define QUASAR_NET_RECONNECTIONPOLICY_HPP

#include <chrono>
#include <algorithm>
#include <cmath>

namespace quasar::net {

/**
 * @struct ReconnectionPolicy
 * @brief Configuration for exponential backoff reconnection.
 */
struct ReconnectionPolicy {
    bool enabled = false;
    std::chrono::milliseconds initialDelay = std::chrono::milliseconds(500);
    std::chrono::milliseconds maxDelay = std::chrono::milliseconds(30000);
    double multiplier = 2.0;
    int maxAttempts = -1; // -1 for infinite

    /**
     * @brief Calculates delay for the next attempt.
     * @param attempt Current attempt count (0-indexed).
     * @return Delay duration.
     */
    std::chrono::milliseconds calculateDelay(int attempt) const {
        if (attempt <= 0) return initialDelay;
        
        // [CS-0010.44] Exponential backoff calculation.
        double delayMs = initialDelay.count() * std::pow(multiplier, attempt);
        return std::chrono::milliseconds(static_cast<long long>(std::min(static_cast<double>(maxDelay.count()), delayMs)));
    }
};

} // namespace quasar::net

#endif // QUASAR_NET_RECONNECTIONPOLICY_HPP
