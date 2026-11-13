/**
 * @file JsonMapper.hpp
 * @brief Streaming schema-driven mapper converting JSON documents into NamedObject trees.
 * @details Uses jsoncons::json_stream_cursor together with ContainerDef and ICodec metadata
 * to validate JSON structures against schemas and materialize typed NamedObject children without
 * throwing errors across expected validation failures.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100, FE-0020
 * @compliance Enforces file-level Doxygen, enumerator documentation, and control-flow comments (CS-0090.1..CS-0090.9).
 */
#ifndef DATACODEC_JSONMAPPER_HPP
#define DATACODEC_JSONMAPPER_HPP

// Include schema definitions and Quasar NamedObject value node wrappers.
#include "datacodec/Schema.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
// Primitive NamedObject value node wrappers for float, string, and boolean.
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedBoolean.hpp"
// Streaming JSON parser header and std::expected header.
#include <jsoncons/json_cursor.hpp>
#include <expected>
#include <string>
// Standard container, stringstream, and console error reporting headers.
#include <vector>
#include <sstream>
#include <iostream>

/**
 * @brief Namespace for Quasar binary and schema-driven data codecs and mappers.
 * @details Houses JsonMapper and its error enumeration JsonMapperErrorCode.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @compliance Scopes JSON mapping components inside namespace datacodec (CS-0010.33).
 */
namespace datacodec {

/**
 * @enum JsonMapperErrorCode
 * @brief Error codes for JSON to NamedObject mapping.
 * @details Categorizes structural, syntactic, and schema-conformance failure conditions
 * returned inside std::expected by JsonMapper::toNamedObject().
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100
 * @exposed Public error-code enum inspected by callers checking std::expected results.
 * @compliance Enum and every enumerator documented with brief, constraint, and compliance proof (CS-0090.1, CS-0090.2).
 */
enum class JsonMapperErrorCode {
    // Baseline success and JSON syntax parse status codes.
    Success = 0,    ///< @brief Mapping succeeded cleanly. @constraint Returned inside expected error paths only as baseline zero value. @compliance CS-0090.1, CS-0090.2 explicit zero sentinel.
    InvalidJson,    ///< @brief Malformed JSON syntax or unexpected StAX cursor token encountered. @constraint Catch-all parse error code. @compliance CS-0090.1, CS-0090.2 parsing failure containment.
    SchemaMismatch, ///< @brief Overall structure violates schema expectations. @constraint Reserved schema-level mismatch category. @compliance CS-0090.1, CS-0090.2 explicit error enum coverage.
    // Schema field presence and value type validation error codes.
    MissingField,   ///< @brief JSON object omitted one or more fields defined in ContainerDef. @constraint Returned when matchedFields < schema.getFields().size(). @compliance CS-0090.1, CS-0090.2 completeness check.
    TypeMismatch,   ///< @brief Root is not a JSON object or field value type mismatches codec type. @constraint Enforces strict codec type matching. @compliance CS-0090.1, CS-0090.2 strong type validation.
    ExtraField      ///< @brief JSON object contains a key absent from ContainerDef. @constraint Rejects unknown keys for strict schema fidelity. @compliance CS-0090.1, CS-0090.2 unknown-key containment.
};

/**
 * @class JsonMapper
 * @brief High-performance, streaming utility for schema-driven JSON-to-NamedObject mapping.
 * @details Parses JSON input via StAX streaming cursor and enforces strict schema field presence
 * and codec type matching, returning errors via std::expected instead of uncaught parse exceptions.
 * @task TSK-20261113-003, TSK-20260303-002
 * @feature FE-0010, FE-0100, FE-0020
 * @exposed Public JSON-to-NamedObject deserialization utility.
 * @compliance Documents public/private methods, full return/error contracts, loop bounds, and branch comments (CS-0090.1..CS-0090.9).
 * @invariant Stateless mapping utility holding no mutable instance fields.
 */
class JsonMapper {
public:
    /**
     * @brief Maps a JSON string to a NamedObject hierarchy based on a schema using streaming.
     * @details Opens an input string stream and json_stream_cursor, confirms the root JSON token
     * is begin_object, and delegates key-value parsing to mapContainerStreaming() while catching
     * jsoncons::ser_error parse exceptions.
     * @task TSK-20261113-003, TSK-20260303-002
     * @feature FE-0010, FE-0100, FE-0020
     * @exposed Public entry point converting JSON payloads into validated NamedObject trees.
     * @compliance Documents parameters, 9-aspect return contract, expected error codes, and try/catch containment (CS-0090.1..CS-0090.9).
     * @param[in] jsonString
     *   Justification: UTF-8 JSON object payload to parse and validate against schema.
     *   Usage: Wrapped in std::istringstream and consumed token-by-token by json_stream_cursor.
     *   Consumed: Read-only const reference; caller retains original string untouched.
     *   Prerequisites: Should contain a JSON object matching ContainerDef schema fields.
     * @param[in] schema
     *   Justification: ContainerDef specifying expected field names and codec types.
     *   Usage: Passed by const reference to mapContainerStreaming for root naming and field lookup.
     *   Consumed: Read-only reference; schema is not mutated or stored.
     *   Prerequisites: Every FieldDef in schema must have a non-null ICodec.
     * @return Expected containing root std::shared_ptr<NamedObject> on success or JsonMapperErrorCode on failure.
     *   Justification: Propagates structured validation outcomes deterministically via std::expected.
     *   Usage: Checked via result.has_value(); value accessed via *result / result.value() or error via result.error().
     *   Consumed: Shared ownership of constructed NamedObject hierarchy transferred on success; value error code otherwise.
     *   Prerequisites: Non-empty JSON object string and valid ContainerDef schema with non-null codecs.
     *   Limitations: Supports flat container fields of Integer, FloatingPoint, String, and Boolean types.
     *   Lifecycle: Returned NamedObject tree lives until all caller shared_ptrs expire; error enum is trivial scalar.
     *   Modification / Deletion: Caller owns the returned NamedObject tree and may mutate, traverse, or destroy it.
     *   Specific Values: Root NamedObject named schema.getName() on success; unexpected JsonMapperErrorCode otherwise.
     *   Error Behavior: Parse exceptions map to InvalidJson; root/value mismatches map to TypeMismatch/ExtraField/MissingField.
     * @retval Root NamedObject tree On complete schema-compliant JSON object parse.
     * @retval JsonMapperErrorCode::InvalidJson On malformed JSON syntax caught from jsoncons::ser_error or bad token stream.
     * @retval JsonMapperErrorCode::TypeMismatch When input is empty, root is not an object, or field value type differs from codec.
     * @retval JsonMapperErrorCode::ExtraField When JSON contains an unknown key absent from schema.
     * @retval JsonMapperErrorCode::MissingField When fewer than schema.getFields().size() fields were matched.
     * @pre Every FieldDef in schema has a valid non-null getCodec() pointer.
     * @post On success, returned NamedObject has children attached for every schema field; schema and jsonString remain unmodified.
     * @throws std::bad_alloc or std::exception if NamedObject allocation fails outside jsoncons::ser_error;
     * expected parse/schema failures return std::unexpected(JsonMapperErrorCode).
     * @error
     *   - Syntax / Parse (jsoncons::ser_error): Caught, logged to std::cerr, and returned as InvalidJson.
     *   - Schema / Type (root non-object, unknown key, wrong type, missing field): Returned via std::unexpected without exceptions.
     * @containment
     *   Approach: Wraps StAX streaming in try/catch(const jsoncons::ser_error&) and propagates validation errors
     *   via std::expected without partial-tree return to caller.
     *   Limitations: Does not handle nested sub-containers, duplicate-key detection beyond matchedFields count, or std::bad_alloc inside catch.
     */
    [[nodiscard]] static std::expected<std::shared_ptr<quasar::named::NamedObject>, JsonMapperErrorCode> toNamedObject(
        const std::string& jsonString,
        const ContainerDef& schema
    ) {
        // Guard streaming JSON cursor parsing against syntax exceptions.
        try {
            std::istringstream is(jsonString);
            jsoncons::json_stream_cursor cursor(is);
            
            // Reject empty streams or non-object root JSON tokens immediately.
            if (cursor.done() || cursor.current().event_type() != jsoncons::staj_event_type::begin_object) {
                return std::unexpected(JsonMapperErrorCode::TypeMismatch);
            }

            return mapContainerStreaming(cursor, schema);
        // Translate jsoncons serialization syntax errors into InvalidJson expected error.
        } catch (const jsoncons::ser_error& e) {
            std::cerr << "JSON Parse Error: " << e.what() << std::endl;
            return std::unexpected(JsonMapperErrorCode::InvalidJson);
        }
    }

private:
    /**
     * @brief Streaming helper parsing a JSON object cursor into child NamedObjects per schema.
     * @details Iterates key/value pairs from begin_object until end_object or stream end, matches keys
     * against schema.getFields(), instantiates strongly typed NamedObject children based on codec type,
     * attaches children via setParent(), and verifies all schema fields were matched.
     * @task TSK-20261113-003, TSK-20260303-002
     * @feature FE-0010, FE-0100, FE-0020
     * @compliance Documents internal helper parameter/return contracts, error containment, and branch logic (CS-0090.1..CS-0090.9).
     * @param[in,out] cursor
     *   Justification: Streaming StAX cursor positioned at begin_object event.
     *   Usage: Advanced via cursor.next() across keys and primitive values until end_object.
     *   Consumed: Cursor stream state is advanced through the JSON object tokens.
     *   Prerequisites: cursor.current().event_type() == jsoncons::staj_event_type::begin_object.
     * @param[in] schema
     *   Justification: ContainerDef defining permitted field keys and expected codec types.
     *   Usage: Queried for container name and searched linearly per JSON key.
     *   Consumed: Read-only reference; not modified or stored.
     *   Prerequisites: All schema fields must hold non-null getCodec() pointers.
     * @return Expected holding populated root NamedObject tree or JsonMapperErrorCode failure.
     *   Justification: Returns parsed NamedObject subtree or precise schema error without throwing for schema errors.
     *   Usage: Returned directly by toNamedObject().
     *   Consumed: Shared ownership transferred on success; trivial enum returned on error.
     *   Prerequisites: Valid cursor at begin_object and valid schema.
     *   Limitations: Bounded by finite JSON token stream; unknown keys terminate early with ExtraField.
     *   Lifecycle: Heap-allocated root and children managed via shared_ptr.
     *   Modification / Deletion: Caller owns returned hierarchy on success.
     *   Specific Values: Root NamedObject on success; InvalidJson, ExtraField, TypeMismatch, or MissingField on error.
     *   Error Behavior: Malformed token sequence throws jsoncons::ser_error (caught by caller) or returns unexpected error code.
     * @pre cursor points at begin_object and every field in schema has a non-null codec.
     * @post When returning value, every field in schema has a matching child attached to root.
     * @throws jsoncons::ser_error if cursor.next() encounters malformed JSON syntax mid-object.
     * @containment
     *   Approach: Returns std::unexpected immediately on structural, key, or type mismatches so invalid children
     *   are never attached; root tree is discarded cleanly on early error return.
     *   Limitations: Loop termination relies on finite JSON stream length (cursor.done() / end_object) without explicit token counter;
     *   duplicate keys could increment matchedFields without explicit duplicate check.
     */
    static std::expected<std::shared_ptr<quasar::named::NamedObject>, JsonMapperErrorCode> mapContainerStreaming(
        jsoncons::json_stream_cursor& cursor,
        const ContainerDef& schema
    ) {
        // Create root container node named after the schema and advance past begin_object.
        std::shared_ptr<quasar::named::NamedObject> root = quasar::named::NamedObject::create(schema.getName());
        cursor.next(); // Consume begin_object

        size_t matchedFields = 0;

        // Iterate JSON key/value pairs until end_object or finite stream exhaustion.
        while (!cursor.done() && cursor.current().event_type() != jsoncons::staj_event_type::end_object) {
            // Require every member entry to begin with a JSON key event.
            if (cursor.current().event_type() != jsoncons::staj_event_type::key) {
                return std::unexpected(JsonMapperErrorCode::InvalidJson);
            }

            std::string key = cursor.current().get<std::string>();
            cursor.next(); // Consume key

            // Find matching field in schema (bounded by schema.getFields().size()).
            std::shared_ptr<FieldDef> foundField = nullptr;
            // Scan schema field list sequentially for matching key name.
            for (const std::shared_ptr<FieldDef>& field : schema.getFields()) {
                // Stop scanning on first exact key name match.
                if (field->getName() == key) {
                    foundField = field;
                    break;
                }
            }

            // Reject JSON properties absent from ContainerDef schema.
            if (!foundField) {
                return std::unexpected(JsonMapperErrorCode::ExtraField);
            }

            // Record matched field count and inspect current JSON token against codec category.
            matchedFields++;
            const jsoncons::staj_event& ev = cursor.current();
            std::shared_ptr<quasar::named::NamedObject> child = nullptr;
            std::string expectedType = foundField->getCodec()->getCodecType();

            // [CS-0010.44] Strict Type Checking against Codec Type.
            // Dispatch Integer codec fields to signed/unsigned 64-bit NamedInteger instances.
            if (expectedType == "Integer") {
                // Construct int64 NamedInteger when JSON token is signed integer.
                if (ev.event_type() == jsoncons::staj_event_type::int64_value) {
                    child = quasar::named::NamedInteger<int64_t>::create(key, ev.get<int64_t>());
                // Construct uint64 NamedInteger when JSON token is unsigned integer.
                } else if (ev.event_type() == jsoncons::staj_event_type::uint64_value) {
                    child = quasar::named::NamedInteger<uint64_t>::create(key, ev.get<uint64_t>());
                // Reject non-integer JSON tokens for Integer codec fields.
                } else {
                    return std::unexpected(JsonMapperErrorCode::TypeMismatch);
                }
            // Dispatch FloatingPoint codec fields from JSON double or integral numeric tokens.
            } else if (expectedType == "FloatingPoint") {
                // Construct double NamedFloatingPoint directly for JSON double tokens.
                if (ev.event_type() == jsoncons::staj_event_type::double_value) {
                    child = quasar::named::NamedFloatingPoint<double>::create(key, ev.get<double>());
                // Widen integer JSON literals to double for FloatingPoint codec fields.
                } else if (ev.event_type() == jsoncons::staj_event_type::int64_value) {
                    child = quasar::named::NamedFloatingPoint<double>::create(key, static_cast<double>(ev.get<int64_t>()));
                // Reject non-numeric JSON tokens for FloatingPoint fields.
                } else {
                    return std::unexpected(JsonMapperErrorCode::TypeMismatch);
                }
            // Dispatch String codec fields from JSON string literals.
            } else if (expectedType == "String") {
                // Construct NamedString when token is a JSON string value.
                if (ev.event_type() == jsoncons::staj_event_type::string_value) {
                    child = quasar::named::NamedString::create(key, ev.get<std::string>());
                // Reject non-string JSON tokens for String codec fields.
                } else {
                    return std::unexpected(JsonMapperErrorCode::TypeMismatch);
                }
            // Dispatch Boolean codec fields from JSON true/false literals.
            } else if (expectedType == "Boolean") {
                // Construct NamedBoolean when token is a JSON boolean value.
                if (ev.event_type() == jsoncons::staj_event_type::bool_value) {
                    child = quasar::named::NamedBoolean::create(key, ev.get<bool>());
                // Reject non-boolean JSON tokens for Boolean codec fields.
                } else {
                    return std::unexpected(JsonMapperErrorCode::TypeMismatch);
                }
            // No additional codec type branches; unsupported types leave child == nullptr for rejection below.
            }

            // Attach instantiated child node to root, or reject unsupported codec categories.
            if (child != nullptr) {
                // Ensure proper tree attachment
                child->setParent(root);
            // Fail with TypeMismatch when codec category was unrecognized and produced no child.
            } else {
                 return std::unexpected(JsonMapperErrorCode::TypeMismatch);
            }
            
            cursor.next(); // Consume value
        }

        // Verify every schema field was populated by the JSON object.
        if (matchedFields < schema.getFields().size()) {
            return std::unexpected(JsonMapperErrorCode::MissingField);
        }

        return root;
    }
};

} // namespace datacodec

#endif // DATACODEC_JSONMAPPER_HPP
