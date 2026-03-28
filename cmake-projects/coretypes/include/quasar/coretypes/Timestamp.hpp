/**
 * @file Timestamp.hpp
 * @brief High-precision temporal Timestamp core type.
 */

#ifndef QUASAR_CORETYPES_TIMESTAMP_HPP
#define QUASAR_CORETYPES_TIMESTAMP_HPP

#include "quasar/coretypes/Integer.hpp"
#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>

namespace quasar::coretypes {

/**
 * @class Timestamp
 * @brief High-precision timestamp representing microseconds since the Unix epoch.
 */
class Timestamp : public Integer<int64_t> {
public:
  explicit Timestamp(int64_t microseconds_since_epoch) : Integer<int64_t>(microseconds_since_epoch) {}

  /**
   * @brief Current time using the system clock (wall clock)
   */
  static Timestamp now() {
      std::chrono::system_clock::time_point now_tp = std::chrono::system_clock::now();
      std::chrono::system_clock::duration duration = now_tp.time_since_epoch();
      return Timestamp(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
  }

  /**
   * @brief Formats the timestamp as ISO-8601.
   */
  std::string toISO8601() const {
      std::chrono::microseconds us(value());
      std::chrono::system_clock::time_point tp(us);
      std::time_t tt = std::chrono::system_clock::to_time_t(tp);
      std::tm tm;
      gmtime_r(&tt, &tm);
      
      int64_t fractional = value() % 1000000;
      if (fractional < 0) fractional += 1000000;
      
      std::ostringstream ss;
      ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
      ss << "." << std::setfill('0') << std::setw(6) << fractional << "Z";
      return ss.str();
  }
};

} // namespace quasar::coretypes

#endif // QUASAR_CORETYPES_TIMESTAMP_HPP
