#ifndef QUASAR_CALIBRATION_NAMEDCALIBRATION_HPP
#define QUASAR_CALIBRATION_NAMEDCALIBRATION_HPP

#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/calibration/Calibrations.hpp"
#include <memory>
#include <vector>
#include <map>

namespace quasar::calibration {

class NamedCalibration : public quasar::named::NamedObject, public ICalibration {
public:
    explicit NamedCalibration(const std::string& name) : NamedObject(name) {}
    virtual ~NamedCalibration() = default;
};

class NamedIdentityCalibration : public NamedCalibration {
private:
    IdentityCalibration m_cal;
public:
    explicit NamedIdentityCalibration(const std::string& name) : NamedCalibration(name) {}
    
    std::string getType() const override { return "NamedIdentityCalibration"; }
    
    std::shared_ptr<quasar::named::NamedObject> clone(quasar::named::CopyPolicy policy = quasar::named::CopyPolicy::DUPLICATE) const override {
        return std::make_shared<NamedIdentityCalibration>(getName());
    }

    Variant rawToEng(const Variant& raw) const override { return m_cal.rawToEng(raw); }
    Variant engToRaw(const Variant& eng) const override { return m_cal.engToRaw(eng); }
};

class NamedLinearCalibration : public NamedCalibration {
public:
    NamedLinearCalibration(const std::string& name, double scale, double offset) 
        : NamedCalibration(name) {
        addChild(quasar::named::NamedFloatingPoint<double>::create("scale", scale));
        addChild(quasar::named::NamedFloatingPoint<double>::create("offset", offset));
    }

    std::string getType() const override { return "NamedLinearCalibration"; }

    std::shared_ptr<quasar::named::NamedObject> clone(quasar::named::CopyPolicy policy = quasar::named::CopyPolicy::DUPLICATE) const override {
        return std::make_shared<NamedLinearCalibration>(getName(), getScale(), getOffset());
    }

    /**
     * @brief Returns the scale factor.
     * @return Scale factor.
     */
    double getScale() const {
        std::shared_ptr<quasar::named::NamedObject> child = getChild("scale");
        if (child && child->is<quasar::named::NamedFloatingPoint<double>>()) {
            return child->as<quasar::named::NamedFloatingPoint<double>>()->value();
        }
        return 1.0;
    }
    
    /**
     * @brief Returns the offset.
     * @return Offset value.
     */
    double getOffset() const {
        std::shared_ptr<quasar::named::NamedObject> child = getChild("offset");
        if (child && child->is<quasar::named::NamedFloatingPoint<double>>()) {
            return child->as<quasar::named::NamedFloatingPoint<double>>()->value();
        }
        return 0.0;
    }

    Variant rawToEng(const Variant& raw) const override {
        return LinearCalibration(getScale(), getOffset()).rawToEng(raw);
    }
    Variant engToRaw(const Variant& eng) const override {
        return LinearCalibration(getScale(), getOffset()).engToRaw(eng);
    }
};

/**
 * @class NamedPolynomialCalibration
 * @brief Named object for polynomial calibration.
 */
class NamedPolynomialCalibration : public NamedCalibration {
public:
    /**
     * @brief Constructs a NamedPolynomialCalibration.
     * @param name Name of the object.
     * @param coeffs Vector of polynomial coefficients.
     */
    NamedPolynomialCalibration(const std::string& name, const std::vector<double>& coeffs) 
        : NamedCalibration(name) {
        const size_t limit = 1000;
        for (size_t i = 0; i < coeffs.size(); ++i) {
            if (i >= limit) throw std::runtime_error("Coefficients limit exceeded");
            addChild(quasar::named::NamedFloatingPoint<double>::create("a" + std::to_string(i), coeffs[i]));
        }
    }

    std::string getType() const override { return "NamedPolynomialCalibration"; }

    std::shared_ptr<quasar::named::NamedObject> clone(quasar::named::CopyPolicy policy = quasar::named::CopyPolicy::DUPLICATE) const override {
        return std::make_shared<NamedPolynomialCalibration>(getName(), getCoeffs());
    }

    /**
     * @brief Retrieves coefficients from child objects.
     * @return Vector of coefficients.
     * @throws std::runtime_error If loop limit exceeded.
     */
    std::vector<double> getCoeffs() const {
        std::vector<double> coeffs;
        size_t i = 0;
        const size_t limit = 1000;
        while (true) {
            if (i >= limit) throw std::runtime_error("Loop limit exceeded in getCoeffs");
            std::shared_ptr<quasar::named::NamedObject> child = getChild("a" + std::to_string(i));
            if (child && child->is<quasar::named::NamedFloatingPoint<double>>()) {
                coeffs.push_back(child->as<quasar::named::NamedFloatingPoint<double>>()->value());
            } else {
                break;
            }
            i++;
        }
        return coeffs;
    }

    Variant rawToEng(const Variant& raw) const override {
        return PolynomialCalibration(getCoeffs()).rawToEng(raw);
    }
    Variant engToRaw(const Variant& eng) const override {
        return PolynomialCalibration(getCoeffs()).engToRaw(eng);
    }
};

// Note: Table, Enum, and Format can be similarly implemented. For brevity and functional correctness, 
// they can also be fully supported via children or serialized structurally. We'll leave them as basic implementations 
// if required, but mapping them to children allows serialization for free.

/**
 * @class CompositeCalibration
 * @brief Calibration that applies multiple calibrations in sequence.
 */
class CompositeCalibration : public NamedCalibration {
public:
    /**
     * @brief Constructs a CompositeCalibration.
     * @param name Name of the object.
     */
    explicit CompositeCalibration(const std::string& name) : NamedCalibration(name) {}

    std::string getType() const override { return "CompositeCalibration"; }

    std::shared_ptr<quasar::named::NamedObject> clone(quasar::named::CopyPolicy policy = quasar::named::CopyPolicy::DUPLICATE) const override {
        std::shared_ptr<CompositeCalibration> comp = std::make_shared<CompositeCalibration>(getName());
        return comp;
    }

    /**
     * @brief Adds a calibration to the sequence.
     * @param cal The calibration to add.
     */
    void addCalibration(std::shared_ptr<NamedCalibration> cal) {
        addChild(cal);
    }

    Variant rawToEng(const Variant& raw) const override {
        Variant current = raw;
        std::list<std::shared_ptr<NamedObject>> children = getChildren();
        const size_t limit = 1000000;
        size_t count = 0;
        for (std::list<std::shared_ptr<NamedObject>>::iterator it = children.begin(); it != children.end(); ++it) {
            if (++count > limit) throw std::runtime_error("Loop limit exceeded in CompositeCalibration::rawToEng");
            std::shared_ptr<NamedObject> child = *it;
            if (child->is<NamedCalibration>()) {
                current = child->as<NamedCalibration>()->rawToEng(current);
            }
        }
        return current;
    }

    Variant engToRaw(const Variant& eng) const override {
        Variant current = eng;
        std::list<std::shared_ptr<NamedObject>> children = getChildren();
        const size_t limit = 1000000;
        size_t count = 0;
        for (std::list<std::shared_ptr<NamedObject>>::reverse_iterator it = children.rbegin(); it != children.rend(); ++it) {
            if (++count > limit) throw std::runtime_error("Loop limit exceeded in CompositeCalibration::engToRaw");
            std::shared_ptr<NamedObject> child = *it;
            if (child->is<NamedCalibration>()) {
                current = child->as<NamedCalibration>()->engToRaw(current);
            }
        }
        return current;
    }
};

} // namespace quasar::calibration
#endif
