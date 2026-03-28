#include "quasar/scripting/TypeBindings.hpp"
#include "quasar/coretypes/IntegerTypes.hpp"
#include "quasar/coretypes/FloatingPointTypes.hpp"
#include "quasar/coretypes/Timestamp.hpp"
#include "quasar/coretypes/Duration.hpp"
#include "quasar/coretypes/Unit.hpp"
#include "quasar/coretypes/Quantity.hpp"

namespace quasar::scripting {

using namespace quasar::coretypes;

void bindCoreTypes(sol::state& lua) {
    // --- Long (int64_t) ---
    sol::usertype<Long> utLong = lua.new_usertype<Long>("Long", sol::call_constructor, sol::constructors<Long(int64_t), Long(const std::string&, int)>());
    utLong["new"] = sol::constructors<Long(int64_t), Long(const std::string&, int)>();
    utLong["value"] = &Long::value;
    utLong["toInt"] = &Long::toInt;
    utLong["toString"] = static_cast<std::string(Long::*)() const>(&Long::toString);
    utLong[sol::meta_function::addition] = [](const Long& a, const Long& b) { return Long(a.value() + b.value()); };
    utLong[sol::meta_function::subtraction] = [](const Long& a, const Long& b) { return Long(a.value() - b.value()); };
    utLong[sol::meta_function::multiplication] = [](const Long& a, const Long& b) { return Long(a.value() * b.value()); };
    utLong[sol::meta_function::division] = [](const Long& a, const Long& b) { 
        if (b.value() == 0) throw std::runtime_error("Division by zero");
        return Long(a.value() / b.value()); 
    };
    utLong[sol::meta_function::equal_to] = [](const Long& a, const Long& b) { return a.value() == b.value(); };
    utLong[sol::meta_function::less_than] = [](const Long& a, const Long& b) { return a.value() < b.value(); };
    utLong[sol::meta_function::to_string] = static_cast<std::string(Long::*)() const>(&Long::toString);

    // --- ULong (uint64_t) ---
    sol::usertype<ULong> utULong = lua.new_usertype<ULong>("ULong", sol::call_constructor, sol::constructors<ULong(uint64_t), ULong(const std::string&, int)>());
    utULong["new"] = sol::constructors<ULong(uint64_t), ULong(const std::string&, int)>();
    utULong["value"] = &ULong::value;
    utULong["toInt"] = &ULong::toInt;
    utULong["toString"] = static_cast<std::string(ULong::*)() const>(&ULong::toString);
    utULong[sol::meta_function::addition] = [](const ULong& a, const ULong& b) { return ULong(a.value() + b.value()); };
    utULong[sol::meta_function::subtraction] = [](const ULong& a, const ULong& b) { return ULong(a.value() - b.value()); };
    utULong[sol::meta_function::multiplication] = [](const ULong& a, const ULong& b) { return ULong(a.value() * b.value()); };
    utULong[sol::meta_function::division] = [](const ULong& a, const ULong& b) { 
        if (b.value() == 0) throw std::runtime_error("Division by zero");
        return ULong(a.value() / b.value()); 
    };
    utULong[sol::meta_function::equal_to] = [](const ULong& a, const ULong& b) { return a.value() == b.value(); };
    utULong[sol::meta_function::less_than] = [](const ULong& a, const ULong& b) { return a.value() < b.value(); };
    utULong[sol::meta_function::to_string] = static_cast<std::string(ULong::*)() const>(&ULong::toString);

    // --- Double (double) ---
    sol::usertype<Double> utDouble = lua.new_usertype<Double>("Double", sol::call_constructor, sol::constructors<Double(double), Double(const std::string&)>());
    utDouble["new"] = sol::constructors<Double(double), Double(const std::string&)>();
    utDouble["value"] = &Double::value;
    utDouble["toString"] = static_cast<std::string(Double::*)() const>(&Double::toString);
    utDouble[sol::meta_function::addition] = [](const Double& a, const Double& b) { return Double(a.value() + b.value()); };
    utDouble[sol::meta_function::subtraction] = [](const Double& a, const Double& b) { return Double(a.value() - b.value()); };
    utDouble[sol::meta_function::multiplication] = [](const Double& a, const Double& b) { return Double(a.value() * b.value()); };
    utDouble[sol::meta_function::division] = [](const Double& a, const Double& b) { return Double(a.value() / b.value()); };
    utDouble[sol::meta_function::equal_to] = [](const Double& a, const Double& b) { return a.value() == b.value(); };
    utDouble[sol::meta_function::to_string] = static_cast<std::string(Double::*)() const>(&Double::toString);

    // --- Timestamp ---
    sol::usertype<Timestamp> utTimestamp = lua.new_usertype<Timestamp>("Timestamp", sol::call_constructor, sol::constructors<Timestamp(int64_t)>());
    utTimestamp["new"] = sol::constructors<Timestamp(int64_t)>();
    utTimestamp["now"] = &Timestamp::now;
    utTimestamp["toISO8601"] = &Timestamp::toISO8601;
    utTimestamp["value"] = &Timestamp::value;
    utTimestamp[sol::meta_function::to_string] = &Timestamp::toISO8601;

    // --- Duration ---
    sol::usertype<Duration> utDuration = lua.new_usertype<Duration>("Duration", sol::call_constructor, sol::constructors<Duration(int64_t)>());
    utDuration["new"] = sol::constructors<Duration(int64_t)>();
    utDuration["fromSeconds"] = &Duration::fromSeconds;
    utDuration["toSeconds"] = &Duration::toSeconds;
    utDuration["value"] = &Duration::value;
    utDuration[sol::meta_function::to_string] = [](const Duration& d) { return std::to_string(d.toSeconds()) + "s"; };

    // --- Unit ---
    sol::usertype<Unit> utUnit = lua.new_usertype<Unit>("Unit", sol::call_constructor, sol::constructors<Unit()>());
    utUnit["new"] = sol::constructors<Unit()>();
    utUnit["m"] = &Unit::m;
    utUnit["kg"] = &Unit::kg;
    utUnit["s"] = &Unit::s;
    utUnit["A"] = &Unit::A;
    utUnit["K"] = &Unit::K;
    utUnit["mol"] = &Unit::mol;
    utUnit["cd"] = &Unit::cd;
    utUnit["scale"] = &Unit::scale;
    utUnit["offset"] = &Unit::offset;
    utUnit["symbol"] = &Unit::symbol;
    utUnit[sol::meta_function::multiplication] = &Unit::operator*;
    utUnit[sol::meta_function::division] = &Unit::operator/;
    utUnit["fromSymbol"] = &Units::fromSymbol;

    // --- Quantity ---
    sol::usertype<Quantity> utQuantity = lua.new_usertype<Quantity>("Quantity", sol::call_constructor, sol::constructors<Quantity(double, const Unit&)>());
    utQuantity["new"] = sol::constructors<Quantity(double, const Unit&)>();
    utQuantity["value"] = &Quantity::value;
    utQuantity["getUnit"] = &Quantity::getUnit;
    utQuantity["convertTo"] = &Quantity::convertTo;
    utQuantity[sol::meta_function::addition] = &Quantity::operator+;
    utQuantity[sol::meta_function::multiplication] = &Quantity::operator*;
    utQuantity[sol::meta_function::to_string] = &Quantity::toString;
}

} // namespace quasar::scripting
