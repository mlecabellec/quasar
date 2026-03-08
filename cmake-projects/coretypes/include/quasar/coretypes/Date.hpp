/**
 * @file Date.hpp
 * @brief High-precision temporal Date core type.
 */

#ifndef QUASAR_CORETYPES_DATE_HPP
#define QUASAR_CORETYPES_DATE_HPP

#include "quasar/coretypes/Integer.hpp"
#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>

namespace quasar::coretypes {

/**
 * @class Date
 * @brief Date representing days since the Unix epoch.
 */
class Date : public Integer<int64_t> {
public:
  explicit Date(int64_t days_since_epoch) : Integer<int64_t>(days_since_epoch) {}

  /**
   * @brief Current date based on system clock
   */
  static Date now() {
      auto now = std::chrono::system_clock::now();
      auto duration = now.time_since_epoch();
      auto hours = std::chrono::duration_cast<std::chrono::hours>(duration).count();
      return Date(hours / 24);
  }

  /**
   * @brief Formats the date as ISO-8601 (YYYY-MM-DD).
   */
  std::string toISO8601() const {
      std::chrono::hours h(value() * 24);
      std::chrono::system_clock::time_point tp(h);
      std::time_t tt = std::chrono::system_clock::to_time_t(tp);
      std::tm tm;
      gmtime_r(&tt, &tm);
      
      std::ostringstream ss;
      ss << std::put_time(&tm, "%Y-%m-%d");
      return ss.str();
  }
};

} // namespace quasar::coretypes

#endif // QUASAR_CORETYPES_DATE_HPP
