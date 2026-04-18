#ifndef QUASAR_OPCUA_UA_COMMON_HPP
#define QUASAR_OPCUA_UA_COMMON_HPP

#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include <open62541/types.h>
#include <string>

namespace quasar::opcua {

/**
 * @brief Converts a std::string to a UA_String (allocated).
 * @param s The source string.
 * @return The allocated UA_String.
 * @compliance [CS-0010.45] Doxygen documentation.
 */
inline UA_String UA_String_fromStdString(const std::string& s) {
    // [CS-0010.11] Use UA_String_fromChars instead of manual UA_malloc to follow UA lifecycle.
    return UA_STRING_ALLOC(s.c_str());
}

/**
 * @brief Converts a std::string to a UA_ByteString (allocated).
 * @param s The source string.
 * @return The allocated UA_ByteString.
 */
inline UA_ByteString UA_ByteString_fromStdString(const std::string& s) {
    // [CS-0010.11] Use UA_ByteString_allocBuffer instead of manual UA_malloc.
    UA_ByteString bs;
    UA_ByteString_init(&bs);
    // Only allocate if source is not empty.
    if (s.length() > 0) {
        UA_ByteString_allocBuffer(&bs, s.length());
        // Copy content from std::string buffer.
        memcpy(bs.data, s.c_str(), bs.length);
    }
    return bs;
}

/**
 * @brief Converts a NamedObject value to a UA_Variant.
 * @param obj The NamedObject to convert.
 * @return The resulting UA_Variant.
 */
inline UA_Variant toUaVariant(std::shared_ptr<named::NamedObject> obj) {
    // Initialize variant to empty state.
    UA_Variant v;
    UA_Variant_init(&v);
    
    // Guard against null objects.
    if (!obj) return v;

    // Dispatch based on reflexive type name.
    std::string type = obj->getType();
    if (type == "NamedInteger") {
        // [CS-0010.34] Explicitly cast to shared_ptr of specific integer types.
        std::shared_ptr<named::NamedInteger<int32_t>> ni32 = std::dynamic_pointer_cast<named::NamedInteger<int32_t>>(obj);
        // Handle 32-bit signed integers.
        if (ni32) {
            UA_Int32 val = ni32->value();
            UA_Variant_setScalarCopy(&v, &val, &UA_TYPES[UA_TYPES_INT32]);
        } else {
            // Handle 64-bit signed integers.
            std::shared_ptr<named::NamedInteger<int64_t>> ni64 = std::dynamic_pointer_cast<named::NamedInteger<int64_t>>(obj);
            if (ni64) {
                UA_Int64 val = ni64->value();
                UA_Variant_setScalarCopy(&v, &val, &UA_TYPES[UA_TYPES_INT64]);
            } else {
                // Fallback for any core Number type to Int64.
                std::shared_ptr<coretypes::Number> num = std::dynamic_pointer_cast<coretypes::Number>(obj);
                if (num) {
                    UA_Int64 valInt = num->toInt64();
                    UA_Variant_setScalarCopy(&v, &valInt, &UA_TYPES[UA_TYPES_INT64]);
                }
            }
        }
    } else if (type == "NamedBoolean") {
        // Handle boolean values.
        std::shared_ptr<named::NamedBoolean> nb = std::dynamic_pointer_cast<named::NamedBoolean>(obj);
        UA_Boolean val = nb->booleanValue();
        UA_Variant_setScalarCopy(&v, &val, &UA_TYPES[UA_TYPES_BOOLEAN]);
    } else if (type == "NamedString") {
        // Handle string values with temporary UA_String allocation.
        std::shared_ptr<named::NamedString> ns = std::dynamic_pointer_cast<named::NamedString>(obj);
        std::string s = ns->toString();
        // Allocate temporary UA string for copying.
        UA_String uas = UA_STRING_ALLOC(s.c_str());
        UA_Variant_setScalarCopy(&v, &uas, &UA_TYPES[UA_TYPES_STRING]);
        UA_String_clear(&uas);
    } else {
        // Fallback for numeric types to double.
        std::shared_ptr<coretypes::Number> num = std::dynamic_pointer_cast<coretypes::Number>(obj);
        if (num) {
            UA_Double valDbl = num->toDouble();
            UA_Variant_setScalarCopy(&v, &valDbl, &UA_TYPES[UA_TYPES_DOUBLE]);
        }
    }
    return v;
}

/**
 * @brief Updates a NamedObject value from a UA_Variant.
 * @param v The source UA_Variant.
 * @param obj The target NamedObject.
 */
inline void fromUaVariant(const UA_Variant *v, std::shared_ptr<named::NamedObject> obj) {
    // Guard against null pointers.
    if (!obj || !v) return;
    std::string type = obj->getType();
    
    // Reverse sync logic for integer types.
    if (type == "NamedInteger") {
        // Convert from UA Int32.
        if (v->type == &UA_TYPES[UA_TYPES_INT32]) {
            int32_t val = *(UA_Int32*)v->data;
            std::shared_ptr<named::NamedInteger<int32_t>> ni32 = std::dynamic_pointer_cast<named::NamedInteger<int32_t>>(obj);
            if (ni32) ni32->setValue(val);
            else {
                // Try casting to 64-bit target.
                std::shared_ptr<named::NamedInteger<int64_t>> ni64 = std::dynamic_pointer_cast<named::NamedInteger<int64_t>>(obj);
                if (ni64) ni64->setValue(val);
            }
        } else if (v->type == &UA_TYPES[UA_TYPES_INT64]) {
            // Convert from UA Int64.
            int64_t val = *(UA_Int64*)v->data;
            std::shared_ptr<named::NamedInteger<int64_t>> ni64 = std::dynamic_pointer_cast<named::NamedInteger<int64_t>>(obj);
            if (ni64) ni64->setValue(val);
            else {
                // Try casting to 32-bit target.
                std::shared_ptr<named::NamedInteger<int32_t>> ni32 = std::dynamic_pointer_cast<named::NamedInteger<int32_t>>(obj);
                if (ni32) ni32->setValue(static_cast<int32_t>(val));
            }
        }
    } else if (type == "NamedBoolean") {
        // Sync boolean from UA to Quasar.
        if (v->type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
            bool val = *(UA_Boolean*)v->data;
            std::shared_ptr<named::NamedBoolean> nb = std::dynamic_pointer_cast<named::NamedBoolean>(obj);
            if (nb) nb->setValue(val);
        }
    } else if (type == "NamedString") {
        // Sync string from UA to Quasar.
        if (v->type == &UA_TYPES[UA_TYPES_STRING]) {
            UA_String uas = *(UA_String*)v->data;
            std::string s((char*)uas.data, uas.length);
            std::shared_ptr<named::NamedString> ns = std::dynamic_pointer_cast<named::NamedString>(obj);
            if (ns) ns->setValue(s);
        }
    } else if (type == "NamedFloatingPoint") {
        // Sync double from UA to Quasar.
        if (v->type == &UA_TYPES[UA_TYPES_DOUBLE]) {
            double val = *(UA_Double*)v->data;
            std::shared_ptr<named::NamedFloatingPoint<double>> nf = std::dynamic_pointer_cast<named::NamedFloatingPoint<double>>(obj);
            if (nf) nf->setValue(val);
        }
    }
}

} // namespace quasar::opcua

#endif // QUASAR_OPCUA_UA_COMMON_HPP
