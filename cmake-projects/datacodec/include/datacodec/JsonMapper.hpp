#ifndef DATACODEC_JSONMAPPER_HPP
#define DATACODEC_JSONMAPPER_HPP

#include "datacodec/Schema.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/NamedInteger.hpp"
#include "quasar/named/NamedFloatingPoint.hpp"
#include "quasar/named/NamedString.hpp"
#include "quasar/named/NamedBoolean.hpp"
#include <jsoncons/json_cursor.hpp>
#include <expected>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

namespace datacodec {

/**
 * @enum JsonMapperErrorCode
 * @brief Error codes for JSON to NamedObject mapping.
 */
enum class JsonMapperErrorCode {
    Success = 0,
    InvalidJson,
    SchemaMismatch,
    MissingField,
    TypeMismatch,
    ExtraField
};

/**
 * @class JsonMapper
 * @brief High-performance, streaming utility for schema-driven JSON-to-NamedObject mapping.
 */
class JsonMapper {
public:
    /**
     * @brief Maps a JSON string to a NamedObject hierarchy based on a schema using streaming.
     */
    [[nodiscard]] static std::expected<std::shared_ptr<quasar::named::NamedObject>, JsonMapperErrorCode> toNamedObject(
        const std::string& jsonString,
        const ContainerDef& schema
    ) {
        try {
            std::istringstream is(jsonString);
            jsoncons::json_stream_cursor cursor(is);
            
            if (cursor.done() || cursor.current().event_type() != jsoncons::staj_event_type::begin_object) {
                return std::unexpected(JsonMapperErrorCode::TypeMismatch);
            }

            return mapContainerStreaming(cursor, schema);
        } catch (const jsoncons::ser_error& e) {
            std::cerr << "JSON Parse Error: " << e.what() << std::endl;
            return std::unexpected(JsonMapperErrorCode::InvalidJson);
        }
    }

private:
    /**
     * @brief Recursive helper to map a JSON object using a streaming cursor.
     */
    static std::expected<std::shared_ptr<quasar::named::NamedObject>, JsonMapperErrorCode> mapContainerStreaming(
        jsoncons::json_stream_cursor& cursor,
        const ContainerDef& schema
    ) {
        std::shared_ptr<quasar::named::NamedObject> root = quasar::named::NamedObject::create(schema.getName());
        cursor.next(); // Consume begin_object

        size_t matchedFields = 0;

        while (!cursor.done() && cursor.current().event_type() != jsoncons::staj_event_type::end_object) {
            if (cursor.current().event_type() != jsoncons::staj_event_type::key) {
                return std::unexpected(JsonMapperErrorCode::InvalidJson);
            }

            std::string key = cursor.current().get<std::string>();
            cursor.next(); // Consume key

            // Find matching field in schema
            std::shared_ptr<FieldDef> foundField = nullptr;
            for (const std::shared_ptr<FieldDef>& field : schema.getFields()) {
                if (field->getName() == key) {
                    foundField = field;
                    break;
                }
            }

            if (!foundField) {
                return std::unexpected(JsonMapperErrorCode::ExtraField);
            }

            matchedFields++;
            const jsoncons::staj_event& ev = cursor.current();
            std::shared_ptr<quasar::named::NamedObject> child = nullptr;
            std::string expectedType = foundField->getCodec()->getCodecType();

            // [CS-0010.44] Strict Type Checking against Codec Type.
            if (expectedType == "Integer") {
                if (ev.event_type() == jsoncons::staj_event_type::int64_value) {
                    child = quasar::named::NamedInteger<int64_t>::create(key, ev.get<int64_t>());
                } else if (ev.event_type() == jsoncons::staj_event_type::uint64_value) {
                    child = quasar::named::NamedInteger<uint64_t>::create(key, ev.get<uint64_t>());
                } else {
                    return std::unexpected(JsonMapperErrorCode::TypeMismatch);
                }
            } else if (expectedType == "FloatingPoint") {
                if (ev.event_type() == jsoncons::staj_event_type::double_value) {
                    child = quasar::named::NamedFloatingPoint<double>::create(key, ev.get<double>());
                } else if (ev.event_type() == jsoncons::staj_event_type::int64_value) {
                    child = quasar::named::NamedFloatingPoint<double>::create(key, static_cast<double>(ev.get<int64_t>()));
                } else {
                    return std::unexpected(JsonMapperErrorCode::TypeMismatch);
                }
            } else if (expectedType == "String") {
                if (ev.event_type() == jsoncons::staj_event_type::string_value) {
                    child = quasar::named::NamedString::create(key, ev.get<std::string>());
                } else {
                    return std::unexpected(JsonMapperErrorCode::TypeMismatch);
                }
            } else if (expectedType == "Boolean") {
                if (ev.event_type() == jsoncons::staj_event_type::bool_value) {
                    child = quasar::named::NamedBoolean::create(key, ev.get<bool>());
                } else {
                    return std::unexpected(JsonMapperErrorCode::TypeMismatch);
                }
            }

            if (child) {
                // Ensure proper tree attachment
                child->setParent(root);
            } else {
                 return std::unexpected(JsonMapperErrorCode::TypeMismatch);
            }
            
            cursor.next(); // Consume value
        }

        if (matchedFields < schema.getFields().size()) {
            return std::unexpected(JsonMapperErrorCode::MissingField);
        }

        return root;
    }
};

} // namespace datacodec

#endif // DATACODEC_JSONMAPPER_HPP
