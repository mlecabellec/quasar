/**
 * @file NamedQuantity.hpp
 * @brief Named representation of a physical Quantity.
 */

#ifndef QUASAR_NAMED_NAMEDQUANTITY_HPP
#define QUASAR_NAMED_NAMEDQUANTITY_HPP

#include "quasar/coretypes/Quantity.hpp"
#include "quasar/named/NamedObject.hpp"

namespace quasar::named {

/**
 * @class NamedQuantity
 * @brief A named container wrapping a physical `coretypes::Quantity`.
 *
 * Inherits from both `NamedObject` (for hierarchy membership) and
 * `coretypes::Quantity` (for the value + typed unit).
 *
 * Two factory overloads are provided:
 *  - `create(name, value, Unit, parent)` — typed SI unit.
 *  - `create(name, value, symbol, parent)` — symbolic-only unit (e.g. "V", "%").
 *
 * **Compliance**: [TSK-20260303-002.6]
 */
class NamedQuantity : public NamedObject, public quasar::coretypes::Quantity {
public:
  virtual ~NamedQuantity() = default;

  // -------------------------------------------------------------------------
  // Factory methods
  // -------------------------------------------------------------------------

  /** Creates a NamedQuantity with a fully typed SI Unit. */
  static std::shared_ptr<NamedQuantity> create(
      const std::string &name,
      double value,
      const quasar::coretypes::Unit &unit,
      std::shared_ptr<NamedObject> parent = nullptr) {
    auto obj = std::make_shared<NamedQuantity>(name, value, unit);
    obj->setSelf(obj);
    if (parent) obj->setParent(parent);
    return obj;
  }

  /**
   * @brief Convenience overload: creates a NamedQuantity with a symbolic unit
   *        string (e.g. "V", "Hz", "%"). The `Unit` struct will only carry the
   *        symbol; dimensional checks are disabled for such units.
   */
  static std::shared_ptr<NamedQuantity> create(
      const std::string &name,
      double value,
      const std::string &symbol,
      std::shared_ptr<NamedObject> parent = nullptr) {
    quasar::coretypes::Unit u;
    u.symbol = symbol;
    return create(name, value, u, parent);
  }

  // -------------------------------------------------------------------------
  // Accessors
  // -------------------------------------------------------------------------

  /** Returns the raw numeric value. */
  double value() const { return quasar::coretypes::Quantity::value(); }

  /** Returns the unit. */
  quasar::coretypes::Unit getUnit() const { return quasar::coretypes::Quantity::getUnit(); }

  /** Returns the unit symbol as a plain string. */
  std::string getUnitSymbol() const { return getUnit().symbol; }

  // -------------------------------------------------------------------------
  // NamedObject interface
  // -------------------------------------------------------------------------

  std::shared_ptr<NamedObject> clone() const override {
    return NamedQuantity::create(getName(), value(), getUnit());
  }

  std::string getType() const override { return "NamedQuantity"; }

  // Public constructor (required by make_shared).
  NamedQuantity(const std::string &name, double value,
                const quasar::coretypes::Unit &unit)
      : NamedObject(name), quasar::coretypes::Quantity(value, unit) {}
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDQUANTITY_HPP
