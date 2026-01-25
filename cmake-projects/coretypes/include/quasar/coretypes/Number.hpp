#ifndef QUASAR_CORETYPES_NUMBER_HPP
#define QUASAR_CORETYPES_NUMBER_HPP

#include <string>

namespace quasar {
namespace coretypes {

/**
 * @brief Abstract base class for all numeric types.
 * 
 * This class defines the common interface for numeric types, mainly focusing on
 * conversion capabilities to primitive C++ types.
 */
class Number {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~Number() = default;

    /**
     * @brief Returns the value of the specified number as an int.
     * @return The numeric value represented by this object after conversion to type int.
     */
    virtual int toInt() const = 0;

    /**
     * @brief Returns the value of the specified number as a long.
     * @return The numeric value represented by this object after conversion to type long.
     */
    virtual long toLong() const = 0;

    /**
     * @brief Returns the value of the specified number as a float.
     * @return The numeric value represented by this object after conversion to type float.
     */
    virtual float toFloat() const = 0;

    /**
     * @brief Returns the value of the specified number as a double.
     * @return The numeric value represented by this object after conversion to type double.
     */
    virtual double toDouble() const = 0;

    /**
     * @brief Returns a string representation of the number.
     * @return Variable represented as a string.
     */
    virtual std::string toString() const = 0;
};

} // namespace coretypes
} // namespace quasar

#endif // QUASAR_CORETYPES_NUMBER_HPP
