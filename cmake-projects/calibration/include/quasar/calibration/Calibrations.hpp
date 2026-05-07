#ifndef QUASAR_CALIBRATION_CALIBRATIONS_HPP
#define QUASAR_CALIBRATION_CALIBRATIONS_HPP

#include "ICalibration.hpp"
#include <vector>
#include <stdexcept>
#include <cmath>
#include <map>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace quasar::calibration {

inline double toDouble(const Variant& v) {
    if (std::holds_alternative<double>(v)) return std::get<double>(v);
    if (std::holds_alternative<int64_t>(v)) return static_cast<double>(std::get<int64_t>(v));
    if (std::holds_alternative<uint64_t>(v)) return static_cast<double>(std::get<uint64_t>(v));
    throw std::invalid_argument("Variant does not hold a numeric type");
}

inline int64_t toInt(const Variant& v) {
    if (std::holds_alternative<int64_t>(v)) return std::get<int64_t>(v);
    if (std::holds_alternative<uint64_t>(v)) return static_cast<int64_t>(std::get<uint64_t>(v));
    if (std::holds_alternative<double>(v)) return static_cast<int64_t>(std::get<double>(v));
    throw std::invalid_argument("Variant does not hold an integer type");
}

class IdentityCalibration : public ICalibration {
public:
    Variant rawToEng(const Variant& raw) const override { return raw; }
    Variant engToRaw(const Variant& eng) const override { return eng; }
};

class LinearCalibration : public ICalibration {
public:
    LinearCalibration(double scale, double offset) : m_scale(scale), m_offset(offset) {
        if (m_scale == 0.0) throw std::invalid_argument("Scale cannot be zero");
    }

    Variant rawToEng(const Variant& raw) const override {
        double x = toDouble(raw);
        return x * m_scale + m_offset;
    }

    Variant engToRaw(const Variant& eng) const override {
        double y = toDouble(eng);
        return (y - m_offset) / m_scale;
    }
private:
    double m_scale;
    double m_offset;
};

class PolynomialCalibration : public ICalibration {
public:
    explicit PolynomialCalibration(std::vector<double> coeffs) : m_coeffs(std::move(coeffs)) {}

    /**
     * @brief Transforms raw value to engineering value using polynomial coefficients.
     * @param raw Raw variant value.
     * @return Engineering variant value.
     * @throws std::runtime_error If loop limit exceeded.
     */
    Variant rawToEng(const Variant& raw) const override {
        double x = toDouble(raw);
        double result = 0.0;
        double x_pow = 1.0;
        const size_t limit = 1000; // Safety limit [CS-0010.37]
        size_t count = 0;
        for (std::vector<double>::const_iterator it = m_coeffs.begin(); it != m_coeffs.end(); ++it) {
            if (++count > limit) throw std::runtime_error("Loop limit exceeded in PolynomialCalibration::rawToEng");
            double coeff = *it;
            result += coeff * x_pow;
            x_pow *= x;
        }
        return result;
    }

    /**
     * @brief Transforms engineering value back to raw value. Only supported for degree <= 1.
     * @param eng Engineering variant value.
     * @return Raw variant value.
     */
    Variant engToRaw(const Variant& eng) const override {
        if (m_coeffs.size() <= 2) {
            double a0 = m_coeffs.empty() ? 0.0 : m_coeffs[0];
            double a1 = m_coeffs.size() > 1 ? m_coeffs[1] : 0.0;
            if (a1 == 0.0) throw std::logic_error("Non-invertible polynomial");
            double y = toDouble(eng);
            return (y - a0) / a1;
        }
        throw std::logic_error("engToRaw for polynomial degree > 1 is not supported analytically");
    }
private:
    std::vector<double> m_coeffs;
};

/**
 * @class TableCalibration
 * @brief Calibration using a lookup table with linear interpolation.
 */
class TableCalibration : public ICalibration {
public:
    /** @brief Point in the lookup table. */
    struct Point {
        double x;
        double y;
    };

    /**
     * @brief Constructs a TableCalibration.
     * @param points Vector of points for the lookup table.
     */
    explicit TableCalibration(std::vector<Point> points) : m_points(std::move(points)) {
        if (m_points.empty()) throw std::invalid_argument("Point pairs cannot be empty");
        std::sort(m_points.begin(), m_points.end(), [](const Point& a, const Point& b) {
            return a.x < b.x;
        });
    }

    Variant rawToEng(const Variant& raw) const override {
        return interpolate(toDouble(raw), true);
    }

    Variant engToRaw(const Variant& eng) const override {
        return interpolate(toDouble(eng), false);
    }

private:
    std::vector<Point> m_points;

    /**
     * @brief Interpolates values within the table.
     * @param val Value to interpolate.
     * @param isRawToEng Direction of transformation.
     * @return Interpolated value.
     * @throws std::runtime_error If loop limit exceeded.
     */
    double interpolate(double val, bool isRawToEng) const {
        // Clamp to endpoints if out of range
        if (m_points.size() == 1) {
            return isRawToEng ? m_points[0].y : m_points[0].x;
        }
        
        if (isRawToEng) {
            if (val <= m_points.front().x) return m_points.front().y;
            if (val >= m_points.back().x) return m_points.back().y;
        } else {
            // engToRaw assumes monotonic y values. We handle mostly increasing bounds here.
            double min_y = m_points.front().y;
            double max_y = m_points.back().y;
            if (min_y > max_y) std::swap(min_y, max_y);
            if (val <= min_y) return (m_points.front().y == min_y) ? m_points.front().x : m_points.back().x;
            if (val >= max_y) return (m_points.back().y == max_y) ? m_points.back().x : m_points.front().x;
        }

        const size_t limit = 1000000; // Safety limit [CS-0010.37]
        size_t count = 0;
        for (size_t i = 0; i < m_points.size() - 1; ++i) {
            if (++count > limit) throw std::runtime_error("Loop limit exceeded in TableCalibration::interpolate");
            double x0 = m_points[i].x;
            double y0 = m_points[i].y;
            double x1 = m_points[i+1].x;
            double y1 = m_points[i+1].y;

            if (isRawToEng) {
                if (val >= x0 && val <= x1) {
                    if (x1 == x0) return y0;
                    return y0 + (y1 - y0) * (val - x0) / (x1 - x0);
                }
            } else {
                double min_val_y = std::min(y0, y1);
                double max_val_y = std::max(y0, y1);
                if (val >= min_val_y && val <= max_val_y) {
                    if (y1 == y0) return x0;
                    return x0 + (x1 - x0) * (val - y0) / (y1 - y0);
                }
            }
        }
        return isRawToEng ? m_points.back().y : m_points.back().x; 
    }
};

/**
 * @class EnumCalibration
 * @brief Calibration using a key-value mapping.
 */
class EnumCalibration : public ICalibration {
public:
    /**
     * @brief Constructs an EnumCalibration.
     * @param mapping Map from integer keys to string values.
     */
    explicit EnumCalibration(std::map<int64_t, std::string> mapping) : m_map(std::move(mapping)) {
        const size_t limit = 1000000;
        size_t count = 0;
        for (std::map<int64_t, std::string>::const_iterator it = m_map.begin(); it != m_map.end(); ++it) {
            if (++count > limit) throw std::runtime_error("Loop limit exceeded in EnumCalibration constructor");
            const int64_t& k = it->first;
            const std::string& v = it->second;
            m_reverse_map[v] = k;
        }
    }

    Variant rawToEng(const Variant& raw) const override {
        int64_t k = toInt(raw);
        std::map<int64_t, std::string>::const_iterator it = m_map.find(k);
        if (it != m_map.end()) return it->second;
        throw std::invalid_argument("Unknown enum key");
    }

    Variant engToRaw(const Variant& eng) const override {
        if (!std::holds_alternative<std::string>(eng)) throw std::invalid_argument("Expected string for enum");
        const std::string& sv = std::get<std::string>(eng);
        std::map<std::string, int64_t>::const_iterator it = m_reverse_map.find(sv);
        if (it != m_reverse_map.end()) return it->second;
        throw std::invalid_argument("Unknown enum string");
    }
private:
    std::map<int64_t, std::string> m_map;
    std::map<std::string, int64_t> m_reverse_map;
};

class FormatCalibration : public ICalibration {
public:
    FormatCalibration(int precision, std::string suffix) : m_precision(precision), m_suffix(std::move(suffix)) {}

    Variant rawToEng(const Variant& raw) const override {
        double v = toDouble(raw);
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(m_precision) << v;
        if (!m_suffix.empty()) oss << " " << m_suffix;
        return oss.str();
    }

    Variant engToRaw(const Variant& eng) const override {
        if (!std::holds_alternative<std::string>(eng)) throw std::invalid_argument("Expected string");
        std::string s = std::get<std::string>(eng);
        if (!m_suffix.empty() && s.size() > m_suffix.size() + 1) {
            if (s.substr(s.size() - m_suffix.size()) == m_suffix) {
                s = s.substr(0, s.size() - m_suffix.size() - 1);
            }
        }
        return std::stod(s);
    }
private:
    int m_precision;
    std::string m_suffix;
};

} // namespace quasar::calibration

#endif // QUASAR_CALIBRATION_CALIBRATIONS_HPP
