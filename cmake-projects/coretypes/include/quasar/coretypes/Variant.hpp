/**
 * @file Variant.hpp
 * @brief Definition of the type-safe Variant class for dynamic data.
 */

#ifndef QUASAR_CORETYPES_VARIANT_HPP
#define QUASAR_CORETYPES_VARIANT_HPP

#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <typeindex>

namespace quasar::coretypes {

/**
 * @enum VariantType
 * @brief Enum representing the types that can be held by a Variant.
 */
enum class VariantType {
    Empty,    /**< No value held. */
    Boolean,  /**< Holding a bool. */
    Integer,  /**< Holding an int64_t. */
    Double,   /**< Holding a double. */
    String,   /**< Holding a std::string. */
    Buffer    /**< Holding a std::vector<uint8_t>. */
};

/**
 * @class Variant
 * @brief A type-safe union for Quasar primitive types.
 * 
 * This class provides a container that can hold a value of any of the Quasar 
 * core primitive types. It ensures type safety through std::variant and 
 * provides a unified interface for dynamic data handling.
 * 
 * **Compliance**:
 * - Fulfills [FE-0110.1.1] Provide a type-safe Variant class.
 * - Fulfills [CS-0010.34] No 'auto' keywords.
 * - Fulfills [CS-0010.45] Doxygen documentation on all members.
 */
class Variant {
public:
    /** @brief Internal variant storage type. */
    using ValueType = std::variant<
        std::monostate,
        bool,
        int64_t,
        double,
        std::string,
        std::vector<uint8_t>
    >;

    /**
     * @brief Constructs an empty Variant.
     * @compliance [CS-0010.32] Initialization.
     */
    Variant() : m_value(std::monostate{}) {}

    /**
     * @brief Constructs a Variant from a boolean.
     * @param value The boolean value.
     */
    explicit Variant(bool value) : m_value(value) {}

    /**
     * @brief Constructs a Variant from an integer.
     * @param value The integer value.
     */
    explicit Variant(int64_t value) : m_value(value) {}

    /**
     * @brief Constructs a Variant from a double.
     * @param value The double value.
     */
    explicit Variant(double value) : m_value(value) {}

    /**
     * @brief Constructs a Variant from a string.
     * @param value The string value.
     */
    explicit Variant(const std::string& value) : m_value(value) {}

    /**
     * @brief Constructs a Variant from a buffer.
     * @param value The buffer (vector of bytes).
     */
    explicit Variant(const std::vector<uint8_t>& value) : m_value(value) {}

    /** @brief Destructor. */
    virtual ~Variant() = default;

    /**
     * @brief Checks if the variant is holding a specific type.
     * @tparam T The type to check for.
     * @return true if matches.
     */
    template<typename T>
    bool holds() const {
        // [CS-0010.44] Type check using std::holds_alternative.
        return std::holds_alternative<T>(m_value);
    }

    /**
     * @brief Returns the type enum of the currently held value.
     * @return The VariantType.
     */
    VariantType getVariantType() const {
        // [CS-0010.44] Dispatching type enum based on variant index.
        if (std::holds_alternative<bool>(m_value)) return VariantType::Boolean;
        if (std::holds_alternative<int64_t>(m_value)) return VariantType::Integer;
        if (std::holds_alternative<double>(m_value)) return VariantType::Double;
        if (std::holds_alternative<std::string>(m_value)) return VariantType::String;
        if (std::holds_alternative<std::vector<uint8_t>>(m_value)) return VariantType::Buffer;
        return VariantType::Empty;
    }

    /**
     * @brief Retrieves the held value as type T.
     * @tparam T The target type.
     * @return The value.
     * @throws std::bad_variant_access if the type does not match.
     */
    template<typename T>
    const T& getAs() const {
        // [CS-0010.44] Explicitly returning the requested type.
        return std::get<T>(m_value);
    }

    /**
     * @brief Safe retrieval of the held value as type T.
     * @tparam T The target type.
     * @return A pointer to the value, or nullptr if type mismatch.
     */
    template<typename T>
    const T* getIf() const {
        // [CS-0010.44] Returning pointer for safe access.
        return std::get_if<T>(&m_value);
    }

    /**
     * @brief Returns a string representation of the held value.
     * @return The string representation.
     */
    std::string toString() const {
        // [CS-0010.44] Serializing variant content to string.
        if (const bool* b = std::get_if<bool>(&m_value)) {
             return *b ? "true" : "false";
        }
        if (const int64_t* i = std::get_if<int64_t>(&m_value)) {
             return std::to_string(*i);
        }
        if (const double* d = std::get_if<double>(&m_value)) {
             return std::to_string(*d);
        }
        if (const std::string* s = std::get_if<std::string>(&m_value)) {
             return *s;
        }
        if (const std::vector<uint8_t>* v = std::get_if<std::vector<uint8_t>>(&m_value)) {
            return "Buffer[" + std::to_string(v->size()) + "]";
        }
        return "Empty";
    }

    /** @brief Copy constructor. */
    Variant(const Variant&) = default;
    /** @brief Copy assignment. */
    Variant& operator=(const Variant&) = default;

    /** @brief Move constructor. */
    Variant(Variant&&) noexcept = default;
    /** @brief Move assignment. */
    Variant& operator=(Variant&&) noexcept = default;

private:
    /** @brief The actual variant storage. */
    ValueType m_value;
};

} // namespace quasar::coretypes

#endif // QUASAR_CORETYPES_VARIANT_HPP
