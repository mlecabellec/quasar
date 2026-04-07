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

inline UA_String UA_String_fromStdString(const std::string& s) {
    UA_String uas;
    uas.length = s.length();
    uas.data = (UA_Byte*)UA_malloc(uas.length);
    memcpy(uas.data, s.c_str(), uas.length);
    return uas;
}

inline UA_ByteString UA_ByteString_fromStdString(const std::string& s) {
    UA_ByteString uabs;
    uabs.length = s.length();
    uabs.data = (UA_Byte*)UA_malloc(uabs.length);
    memcpy(uabs.data, s.c_str(), uabs.length);
    return uabs;
}

inline UA_Variant toUaVariant(std::shared_ptr<named::NamedObject> obj) {
    UA_Variant v;
    UA_Variant_init(&v);
    
    if (!obj) return v;

    std::string type = obj->getType();
    if (type == "NamedInteger") {
        if (auto ni32 = std::dynamic_pointer_cast<named::NamedInteger<int32_t>>(obj)) {
            UA_Int32 val = ni32->value();
            UA_Variant_setScalarCopy(&v, &val, &UA_TYPES[UA_TYPES_INT32]);
        } else if (auto ni64 = std::dynamic_pointer_cast<named::NamedInteger<int64_t>>(obj)) {
            UA_Int64 val = ni64->value();
            UA_Variant_setScalarCopy(&v, &val, &UA_TYPES[UA_TYPES_INT64]);
        } else if (auto num = std::dynamic_pointer_cast<coretypes::Number>(obj)) {
            UA_Int64 val = num->toInt64();
            UA_Variant_setScalarCopy(&v, &val, &UA_TYPES[UA_TYPES_INT64]);
        }
    } else if (type == "NamedBoolean") {
        UA_Boolean val = std::dynamic_pointer_cast<named::NamedBoolean>(obj)->booleanValue();
        UA_Variant_setScalarCopy(&v, &val, &UA_TYPES[UA_TYPES_BOOLEAN]);
    } else if (type == "NamedString") {
        std::string s = std::dynamic_pointer_cast<named::NamedString>(obj)->toString();
        UA_String uas = UA_STRING_ALLOC(s.c_str());
        UA_Variant_setScalarCopy(&v, &uas, &UA_TYPES[UA_TYPES_STRING]);
        UA_String_clear(&uas);
    } else if (auto num = std::dynamic_pointer_cast<coretypes::Number>(obj)) {
        UA_Double val = num->toDouble();
        UA_Variant_setScalarCopy(&v, &val, &UA_TYPES[UA_TYPES_DOUBLE]);
    }
    return v;
}

inline void fromUaVariant(const UA_Variant *v, std::shared_ptr<named::NamedObject> obj) {
    if (!obj || !v) return;
    std::string type = obj->getType();
    
    if (type == "NamedInteger") {
        if (v->type == &UA_TYPES[UA_TYPES_INT32]) {
            int32_t val = *(UA_Int32*)v->data;
            if (auto ni = std::dynamic_pointer_cast<named::NamedInteger<int32_t>>(obj)) ni->setValue(val);
            else if (auto ni64 = std::dynamic_pointer_cast<named::NamedInteger<int64_t>>(obj)) ni64->setValue(val);
        } else if (v->type == &UA_TYPES[UA_TYPES_INT64]) {
            int64_t val = *(UA_Int64*)v->data;
            if (auto ni64 = std::dynamic_pointer_cast<named::NamedInteger<int64_t>>(obj)) ni64->setValue(val);
            else if (auto ni = std::dynamic_pointer_cast<named::NamedInteger<int32_t>>(obj)) ni->setValue(static_cast<int32_t>(val));
        }
    } else if (type == "NamedBoolean") {
        if (v->type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
            bool val = *(UA_Boolean*)v->data;
            if (auto nb = std::dynamic_pointer_cast<named::NamedBoolean>(obj)) nb->setValue(val);
        }
    } else if (type == "NamedString") {
        if (v->type == &UA_TYPES[UA_TYPES_STRING]) {
            UA_String uas = *(UA_String*)v->data;
            std::string s((char*)uas.data, uas.length);
            if (auto ns = std::dynamic_pointer_cast<named::NamedString>(obj)) ns->setValue(s);
        }
    } else if (type == "NamedFloatingPoint") {
        if (v->type == &UA_TYPES[UA_TYPES_DOUBLE]) {
            double val = *(UA_Double*)v->data;
            if (auto nf = std::dynamic_pointer_cast<named::NamedFloatingPoint<double>>(obj)) nf->setValue(val);
        }
    }
}

} // namespace quasar::opcua

#endif // QUASAR_OPCUA_UA_COMMON_HPP
