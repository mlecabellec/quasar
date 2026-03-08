/**
 * @file Unit.hpp
 * @brief Represents a physical unit with SI dimensions and scaling.
 */

#ifndef QUASAR_CORETYPES_UNIT_HPP
#define QUASAR_CORETYPES_UNIT_HPP

#include <string>

namespace quasar::coretypes {

struct Unit {
    int m = 0;
    int kg = 0;
    int s = 0;
    int A = 0;
    int K = 0;
    int mol = 0;
    int cd = 0;
    
    double scale = 1.0;
    double offset = 0.0;
    std::string symbol = "";

    bool hasSameDimensions(const Unit& o) const {
        return m == o.m && kg == o.kg && s == o.s && A == o.A && K == o.K && mol == o.mol && cd == o.cd;
    }

    Unit operator*(const Unit& o) const {
        std::string newSymbol = symbol.empty() || o.symbol.empty() ? "" : symbol + "*" + o.symbol;
        return {m + o.m, kg + o.kg, s + o.s, A + o.A, K + o.K, mol + o.mol, cd + o.cd, scale * o.scale, 0.0, newSymbol};
    }

    Unit operator/(const Unit& o) const {
        std::string newSymbol = symbol.empty() || o.symbol.empty() ? "" : symbol + "/" + o.symbol;
        return {m - o.m, kg - o.kg, s - o.s, A - o.A, K - o.K, mol - o.mol, cd - o.cd, scale / o.scale, 0.0, newSymbol};
    }
};

namespace Units {
    inline const Unit Dimensionless{0, 0, 0, 0, 0, 0, 0, 1.0, 0.0, ""};
    inline const Unit Meter{1, 0, 0, 0, 0, 0, 0, 1.0, 0.0, "m"};
    inline const Unit Kilometer{1, 0, 0, 0, 0, 0, 0, 1000.0, 0.0, "km"};
    inline const Unit Millimeter{1, 0, 0, 0, 0, 0, 0, 0.001, 0.0, "mm"};
    inline const Unit Kilogram{0, 1, 0, 0, 0, 0, 0, 1.0, 0.0, "kg"};
    inline const Unit Gram{0, 1, 0, 0, 0, 0, 0, 0.001, 0.0, "g"};
    inline const Unit Second{0, 0, 1, 0, 0, 0, 0, 1.0, 0.0, "s"};
    inline const Unit Ampere{0, 0, 0, 1, 0, 0, 0, 1.0, 0.0, "A"};
    inline const Unit Kelvin{0, 0, 0, 0, 1, 0, 0, 1.0, 0.0, "K"};
    inline const Unit Celsius{0, 0, 0, 0, 1, 0, 0, 1.0, 273.15, "C"};
    inline const Unit Mole{0, 0, 0, 0, 0, 1, 0, 1.0, 0.0, "mol"};
    inline const Unit Candela{0, 0, 0, 0, 0, 0, 1, 1.0, 0.0, "cd"};
    
    inline const Unit Area = Meter * Meter;
    inline const Unit Volume = Meter * Meter * Meter;
    inline const Unit Speed = Meter / Second;
    inline const Unit Volt{2, 1, -3, -1, 0, 0, 0, 1.0, 0.0, "V"};

    /**
     * @brief Looks up a unit by its symbol.
     * @return The unit if found, or Dimensionless if not found.
     */
    inline Unit fromSymbol(const std::string& symbol) {
        if (symbol == "m") return Meter;
        if (symbol == "km") return Kilometer;
        if (symbol == "mm") return Millimeter;
        if (symbol == "kg") return Kilogram;
        if (symbol == "g") return Gram;
        if (symbol == "s") return Second;
        if (symbol == "A") return Ampere;
        if (symbol == "K") return Kelvin;
        if (symbol == "C") return Celsius;
        if (symbol == "mol") return Mole;
        if (symbol == "cd") return Candela;
        if (symbol == "V") return Volt;
        return Dimensionless;
    }
}

} // namespace quasar::coretypes

#endif // QUASAR_CORETYPES_UNIT_HPP
