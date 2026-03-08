/**
 * @file Duration.hpp
 * @brief High-precision temporal Duration core type.
 */

#ifndef QUASAR_CORETYPES_DURATION_HPP
#define QUASAR_CORETYPES_DURATION_HPP

#include "quasar/coretypes/Integer.hpp"
#include <chrono>
#include <string>

namespace quasar::coretypes {

/**
 * @class Duration
 * @brief High-precision duration representing microseconds.
 */
class Duration : public Integer<int64_t> {
public:
  explicit Duration(int64_t microsecond_duration) : Integer<int64_t>(microsecond_duration) {}

  /**
   * @brief Creates a Duration from seconds.
   */
  static Duration fromSeconds(double seconds) {
      return Duration(static_cast<int64_t>(seconds * 1000000.0));
  }

  /**
   * @brief Returns the duration in seconds as a double.
   */
  double toSeconds() const {
      return static_cast<double>(value()) / 1000000.0;
  }
};

} // namespace quasar::coretypes

#endif // QUASAR_CORETYPES_DURATION_HPP
