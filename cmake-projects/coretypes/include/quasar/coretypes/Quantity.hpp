/**
 * @file Quantity.hpp
 * @brief Represents a physical quantity with a unit.
 */

#ifndef QUASAR_CORETYPES_QUANTITY_HPP
#define QUASAR_CORETYPES_QUANTITY_HPP

#include "quasar/coretypes/FloatingPoint.hpp"
#include "quasar/coretypes/Unit.hpp"
#include <string>
#include <stdexcept>

namespace quasar::coretypes {

/**
 * @class Quantity
 * @brief Represents a physical value with a typed SI unit.
 *
 * Supports conversion between compatible units and arithmetic on quantities.
 *
 * **Compliance**:
 * - Fulfills [TSK-20260303-002.6] Physical Modeling & Quantities.
 */
class Quantity : public FloatingPoint<double> {
public:
  Quantity(double val, const Unit& unit)
      : FloatingPoint<double>(val), m_unit(unit) {}

  Unit getUnit() const { return m_unit; }

  /**
   * @brief Converts this quantity to a given compatible unit.
   * @throws std::invalid_argument if the target unit has different dimensions.
   */
  Quantity convertTo(const Unit& targetUnit) const {
      if (!m_unit.hasSameDimensions(targetUnit)) {
          throw std::invalid_argument("Incompatible dimensions for unit conversion");
      }
      double baseValue = (this->value() * m_unit.scale) + m_unit.offset;
      double targetValue = (baseValue - targetUnit.offset) / targetUnit.scale;
      return Quantity(targetValue, targetUnit);
  }

  /**
   * @brief Adds two quantities with the same dimension.
   * @throws std::invalid_argument if dimensions differ.
   */
  Quantity operator+(const Quantity& o) const {
      if (!m_unit.hasSameDimensions(o.getUnit())) {
          throw std::invalid_argument("Incompatible dimensions for addition");
      }
      Quantity converted = o.convertTo(m_unit);
      return Quantity(this->value() + converted.value(), m_unit);
  }

  /**
   * @brief Multiplies two quantities; dimensions are combined.
   */
  Quantity operator*(const Quantity& o) const {
      return Quantity(this->value() * o.value(), m_unit * o.getUnit());
  }

  std::string toString() const override {
      return std::to_string(this->value()) + " " + m_unit.symbol;
  }

  std::string getType() const override { return "Quantity"; }

private:
  Unit m_unit;
};

} // namespace quasar::coretypes

#endif // QUASAR_CORETYPES_QUANTITY_HPP
