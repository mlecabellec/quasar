/**
 * @file UA_Common.hpp
 * @brief Bidirectional conversion helpers between Quasar NamedObject primitives and open62541 UA types.
 *
 * @details Provides inline utility functions to allocate open62541 strings/byte-strings from
 * `std::string` and marshal scalar values bidirectionally between `quasar::named::NamedObject`
 * subclasses and `UA_Variant` payloads.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0140, FE-0180, FE-0200
 * @constraint CS-0010, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.6, CS-0090.7, CS-0090.8
 * @compliance Documents all namespace-level marshaling APIs with full parameter, return,
 * error-containment, and branch-justification contracts without altering conversion logic.
 */

#ifndef QUASAR_OPCUA_UA_COMMON_HPP
#define QUASAR_OPCUA_UA_COMMON_HPP

// Quasar named object and typed scalar primitives.
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
// open62541 core C type definitions and C++ standard string support.
#include <open62541/types.h>
#include <string>

/**
 * @brief Quasar OPC UA integration namespace housing server, client, security, and type-marshaling utilities.
 *
 * @details Encapsulates all open62541 adapter services (`OpcUaServerService`, `OpcUaClientService`,
 * `OpcUaSecurityManager`) and `UA_Variant` conversion helpers bridging Quasar `NamedObject` trees
 * with OPC UA address spaces.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0200
 * @constraint CS-0010.33, CS-0090.1, CS-0090.2
 * @compliance Isolates all OPC UA symbols inside `quasar::opcua` to prevent global namespace pollution.
 */
namespace quasar::opcua {

/**
 * @brief Converts a std::string to a UA_String (allocated).
 *
 * @details Delegates buffer allocation and copy to `UA_STRING_ALLOC` on the C-string view
 * of `s`, producing a heap-backed open62541 string owned by the caller.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0200
 * @constraint CS-0010.11, CS-0010.45, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.7, CS-0090.8, CS-0090.9
 * @exposed
 * @compliance [CS-0010.45] Doxygen documentation; avoids manual `malloc` by using `UA_STRING_ALLOC`.
 *
 * @param[in] s
 *   Justification: Supplies the C++ source text to marshal into an open62541 `UA_String`.
 *   Usage: Read via `c_str()` to allocate and populate the returned `UA_String`.
 *   Consumed: No (read-only const reference).
 *   Prerequisites: Valid initialized `std::string` without embedded NUL terminators if C-string semantics apply.
 *
 * @pre s.c_str() points to a valid null-terminated string buffer.
 * @post Returned `UA_String` holds a newly allocated copy of `s` (or `UA_STRING_NULL` if empty/OOM).
 * @return UA_String
 *   Justification: Supplies open62541 APIs with a self-contained heap-allocated `UA_String`.
 *   Usage: Caller passes value to open62541 functions and frees it via `UA_String_clear`.
 *   Consumed: Ownership transferred to caller; caller is responsible for open62541 cleanup.
 *   Prerequisites: Caller must release non-empty buffers with `UA_String_clear`.
 *   Limitations: Truncates at first embedded NUL byte because `UA_STRING_ALLOC` uses `c_str()`.
 *   Lifecycle: Valid from return until passed to `UA_String_clear(&ret)`.
 *   Modification / Deletion: Caller may mutate `data` within `length` and MUST delete via `UA_String_clear`.
 *   Specific Values: `length == 0, data == nullptr` when `s.empty()` or allocation fails; otherwise non-null buffer.
 *   Error Behavior: On open62541 allocation failure, `UA_STRING_ALLOC` yields `UA_STRING_NULL` without throwing.
 * @error
 *   - Resource / Allocation (open62541 heap allocation failure): `UA_STRING_ALLOC` returns `UA_STRING_NULL` (`data == nullptr`); no exception thrown.
 * @containment
 *   Approach: Uses open62541 allocator `UA_STRING_ALLOC` so all allocations match `UA_String_clear` deallocation.
 *   Limitations: Does not signal OOM separately from an empty input string; embeds no length check for embedded NULs.
 */
inline UA_String UA_String_fromStdString(const std::string& s) {
    // [CS-0010.11] Use UA_String_fromChars instead of manual UA_malloc to follow UA lifecycle.
    return UA_STRING_ALLOC(s.c_str());
}

/**
 * @brief Converts a std::string to a UA_ByteString (allocated).
 *
 * @details Initializes a `UA_ByteString`, allocates exact byte capacity via
 * `UA_ByteString_allocBuffer` when `s.length() > 0`, and copies raw octets via `memcpy`
 * so embedded NUL bytes are preserved.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0200
 * @constraint CS-0010.11, CS-0010.14, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
 * @exposed
 * @compliance Uses `UA_ByteString_allocBuffer` matched to `s.length()` prior to `memcpy` to prevent buffer overflow.
 *
 * @param[in] s
 *   Justification: Supplies raw binary/text octets (such as passwords or DER payloads) for open62541.
 *   Usage: Length is checked and raw bytes are copied into the allocated `UA_ByteString` buffer.
 *   Consumed: No (read-only const reference).
 *   Prerequisites: Valid initialized `std::string` whose `length()` fits addressable heap memory.
 *
 * @pre true (empty strings are safely handled and return `UA_BYTESTRING_NULL`).
 * @post Returned `UA_ByteString` contains exact `s.length()` bytes (or empty if `s.empty()`).
 * @return UA_ByteString
 *   Justification: Provides open62541 security/login APIs with a binary-safe `UA_ByteString`.
 *   Usage: Caller inspects `.length`/`.data` or hands ownership to open62541 APIs, then clears it.
 *   Consumed: Ownership transferred to caller; caller must invoke `UA_ByteString_clear`.
 *   Prerequisites: Verify `bs.length == s.length()` if allocation failure must be distinguished.
 *   Limitations: Does not propagate `UA_StatusCode` returned by `UA_ByteString_allocBuffer`.
 *   Lifecycle: Valid from return until cleared via `UA_ByteString_clear(&bs)`.
 *   Modification / Deletion: Caller may mutate buffer bytes within `length` and MUST free via `UA_ByteString_clear`.
 *   Specific Values: `UA_BYTESTRING_NULL` when `s.empty()` or alloc fails; otherwise `length == s.length()`.
 *   Error Behavior: If `UA_ByteString_allocBuffer` fails silently, `bs` remains initialized to null/empty.
 * @error
 *   - Resource / Allocation (`UA_ByteString_allocBuffer` fails): Returns unpopulated `UA_ByteString` (`length == 0`); note `memcpy` relies on `allocBuffer` succeeding when `s.length() > 0`.
 * @containment
 *   Approach: Zero-initializes `bs` with `UA_ByteString_init` and skips allocation/copy when `s.length() == 0`.
 *   Limitations: Does not check the return code of `UA_ByteString_allocBuffer` before `memcpy`; caller should avoid huge untrusted strings.
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
    // Empty input leaves initialized UA_BYTESTRING_NULL without heap allocation.
    return bs;
}

/**
 * @brief Converts a NamedObject value to a UA_Variant.
 *
 * @details Inspects `obj->getType()` to dispatch scalar conversion for `NamedInteger`
 * (`int32_t`, `int64_t`, or generic `coretypes::Number` fallback), `NamedBoolean`,
 * `NamedString`, or fallback numeric types (`double`), copying the scalar into `UA_Variant`.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0140, FE-0200
 * @constraint CS-0010.15, CS-0010.34, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.6, CS-0090.7, CS-0090.8, CS-0090.9
 * @exposed
 * @compliance Uses explicit `std::shared_ptr` types (no `auto`), null guard on `obj`, and RAII cleanup of temporary `UA_String`.
 *
 * @param[in] obj
 *   Justification: Supplies the Quasar reflexive `NamedObject` whose scalar value is marshaled to OPC UA.
 *   Usage: Inspected via `getType()` and `std::dynamic_pointer_cast` to read scalar value.
 *   Consumed: No (shared_ptr copied by value without altering pointed-to object value).
 *   Prerequisites: May be `nullptr` (yields empty `UA_Variant`); if `NamedBoolean` or `NamedString`, dynamic cast must succeed.
 *
 * @pre obj == nullptr || obj->getType() returns a valid Quasar reflexive type tag.
 * @post Returned `UA_Variant` is empty (`UA_Variant_isEmpty(&v)`) or holds a deep-copied scalar.
 * @return UA_Variant
 *   Justification: Produces an open62541 variant value ready for variable attribute init or server writes.
 *   Usage: Caller passes value to `UA_Server_writeValue` or node attributes, then calls `UA_Variant_clear`.
 *   Consumed: Ownership of variant payload transferred to caller.
 *   Prerequisites: Caller must check `!UA_Variant_isEmpty(&v)` before writing to server nodes.
 *   Limitations: Only handles scalar types (`Int32`, `Int64`, `Boolean`, `String`, `Double`); unsupported types yield empty variant.
 *   Lifecycle: Valid until caller invokes `UA_Variant_clear(&v)`.
 *   Modification / Deletion: Caller owns payload and MUST release it with `UA_Variant_clear`.
 *   Specific Values: Empty `UA_Variant` when `obj == nullptr` or unsupported type; otherwise scalar `Int32`/`Int64`/`Boolean`/`String`/`Double`.
 *   Error Behavior: Returns empty `UA_Variant` for null or non-numeric unsupported types; propagates exceptions if `obj` accessors throw.
 * @throws std::exception [Validation / Contract] Any exception thrown by `NamedObject` accessors propagates after `UA_Variant` stack cleanup.
 * @error
 *   - Validation / Contract (`obj == nullptr` or unknown non-Number type): Returns initialized empty `UA_Variant`; no state mutated.
 *   - Resource / Allocation (`UA_Variant_setScalarCopy` / `UA_STRING_ALLOC` OOM): Variant stays empty or unpopulated.
 * @containment
 *   Approach: Initializes `UA_Variant_init(&v)` unconditionally, guards `if (!obj)`, and clears
 *   temporary `UA_String` immediately after `UA_Variant_setScalarCopy` to prevent leaks.
 *   Limitations: Assumes `NamedBoolean` and `NamedString` type tags accurately match their dynamic C++ subclasses.
 */
inline UA_Variant toUaVariant(std::shared_ptr<named::NamedObject> obj) {
    // Initialize variant to empty state.
    UA_Variant v;
    UA_Variant_init(&v);
    
    // Guard against null objects.
    if (!obj) return v;

    // Dispatch based on reflexive type name.
    std::string type = obj->getType();
    // Branch on Quasar reflexive type string to select matching UA scalar representation.
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
            // Use Int64 variant when object is a 64-bit NamedInteger.
            if (ni64) {
                UA_Int64 val = ni64->value();
                UA_Variant_setScalarCopy(&v, &val, &UA_TYPES[UA_TYPES_INT64]);
            } else {
                // Fallback for any core Number type to Int64.
                std::shared_ptr<coretypes::Number> num = std::dynamic_pointer_cast<coretypes::Number>(obj);
                // Convert generic Number integer widths to 64-bit UA_Int64.
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
        // Copy floating-point scalar when object inherits from coretypes::Number.
        if (num) {
            UA_Double valDbl = num->toDouble();
            UA_Variant_setScalarCopy(&v, &valDbl, &UA_TYPES[UA_TYPES_DOUBLE]);
        }
    }
    return v;
}

/**
 * @brief Updates a NamedObject value from a UA_Variant.
 *
 * @details Validates `v` and `obj` non-null, checks `obj->getType()` and `v->type`,
 * extracts the typed scalar from `v->data`, and mutates the target `NamedObject` via
 * `setValue(...)`.
 *
 * @task TSK-20261113-003, TSK-20260311-005
 * @feature FE-0140, FE-0200
 * @constraint CS-0010.15, CS-0020.59, CS-0090.1, CS-0090.2, CS-0090.3, CS-0090.6, CS-0090.8, CS-0090.9
 * @exposed
 * @compliance Validates pointer inputs (`v`, `obj`) and `UA_Variant::type` descriptors before dereferencing `v->data`.
 *
 * @param[in] v
 *   Justification: Supplies incoming OPC UA variant data from a write callback or subscription notification.
 *   Usage: Type pointer and scalar `data` payload are inspected read-only to extract the new value.
 *   Consumed: No (const pointer; caller retains ownership of `UA_Variant`).
 *   Prerequisites: May be `nullptr` (safe no-op); if non-null and matching `UA_TYPES_*`, `v->data` must be non-null scalar.
 * @param[in,out] obj
 *   Justification: Identifies the target Quasar `NamedObject` node whose value will be synchronized.
 *   Usage: Cast via `std::dynamic_pointer_cast` and updated in-place using `setValue`.
 *   Consumed: Mutated in-place (value updated when `v->type` matches `obj->getType()`); no-op if null.
 *   Prerequisites: May be `nullptr` (safe no-op); otherwise valid `NamedObject` instance.
 *
 * @pre (v == nullptr || obj == nullptr) || (v->data != nullptr whenever v->type matches a supported scalar type)
 * @post If `v` and `obj` are non-null and their scalar types match, `obj` stores the new value from `v`.
 * @throws std::exception [Validation / Contract] Any exception raised by `NamedObject::setValue` propagates to caller.
 * @error
 *   - Validation / Contract (`v == nullptr`, `obj == nullptr`, or mismatched `v->type`): Ignored safely as a no-op; `obj` remains unchanged.
 * @containment
 *   Approach: Explicit null guards on `v` and `obj`, strict `v->type` pointer equality checks,
 *   and safe `dynamic_pointer_cast` checks before calling `setValue`.
 *   Limitations: Assumes `v->data` is non-null whenever `v->type` equals one of the checked `UA_TYPES` entries; narrows `int64_t` via `static_cast<int32_t>` when writing to 32-bit `NamedInteger`.
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
            // Update 32-bit target directly or fall back to 64-bit integer target.
            if (ni32) ni32->setValue(val);
            else {
                // Try casting to 64-bit target.
                std::shared_ptr<named::NamedInteger<int64_t>> ni64 = std::dynamic_pointer_cast<named::NamedInteger<int64_t>>(obj);
                // Write widened 32-bit value if target holds int64_t.
                if (ni64) ni64->setValue(val);
            }
        } else if (v->type == &UA_TYPES[UA_TYPES_INT64]) {
            // Convert from UA Int64.
            int64_t val = *(UA_Int64*)v->data;
            std::shared_ptr<named::NamedInteger<int64_t>> ni64 = std::dynamic_pointer_cast<named::NamedInteger<int64_t>>(obj);
            // Update 64-bit target directly or narrow to 32-bit target if needed.
            if (ni64) ni64->setValue(val);
            else {
                // Try casting to 32-bit target.
                std::shared_ptr<named::NamedInteger<int32_t>> ni32 = std::dynamic_pointer_cast<named::NamedInteger<int32_t>>(obj);
                // Narrow 64-bit UA value explicitly when target is 32-bit NamedInteger [CS-0020.59].
                if (ni32) ni32->setValue(static_cast<int32_t>(val));
            }
        }
        // Unsupported variant type for NamedInteger leaves object unchanged.
    } else if (type == "NamedBoolean") {
        // Sync boolean from UA to Quasar.
        if (v->type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
            bool val = *(UA_Boolean*)v->data;
            std::shared_ptr<named::NamedBoolean> nb = std::dynamic_pointer_cast<named::NamedBoolean>(obj);
            // Apply boolean update when dynamic cast confirms NamedBoolean.
            if (nb) nb->setValue(val);
        }
    } else if (type == "NamedString") {
        // Sync string from UA to Quasar.
        if (v->type == &UA_TYPES[UA_TYPES_STRING]) {
            UA_String uas = *(UA_String*)v->data;
            std::string s((char*)uas.data, uas.length);
            std::shared_ptr<named::NamedString> ns = std::dynamic_pointer_cast<named::NamedString>(obj);
            // Apply string update when dynamic cast confirms NamedString.
            if (ns) ns->setValue(s);
        }
    } else if (type == "NamedFloatingPoint") {
        // Sync double from UA to Quasar.
        if (v->type == &UA_TYPES[UA_TYPES_DOUBLE]) {
            double val = *(UA_Double*)v->data;
            std::shared_ptr<named::NamedFloatingPoint<double>> nf = std::dynamic_pointer_cast<named::NamedFloatingPoint<double>>(obj);
            // Apply double update when dynamic cast confirms NamedFloatingPoint<double>.
            if (nf) nf->setValue(val);
        }
    }
    // Unsupported NamedObject subtypes are ignored safely.
}

} // namespace quasar::opcua

#endif // QUASAR_OPCUA_UA_COMMON_HPP
