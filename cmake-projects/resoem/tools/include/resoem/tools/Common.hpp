#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

namespace resoem::tools {

/**
 * @brief Helper to format a byte buffer as a hex string.
 */
inline std::string format_hex(const uint8_t* data, size_t len) {
    static const char* const lut = "0123456789ABCDEF";
    std::string out;
    out.reserve(len * 3);
    for (size_t i = 0; i < len; ++i) {
        out.push_back(lut[data[i] >> 4]);
        out.push_back(lut[data[i] & 15]);
        if (i + 1 < len) out.push_back(' ');
    }
    return out;
}

/**
 * @brief Helper to parse a hex string into a byte vector.
 */
inline std::vector<uint8_t> parse_hex(const std::string& hex) {
    std::vector<uint8_t> out;
    for (size_t i = 0; i < hex.length(); ) {
        if (std::isspace(hex[i])) {
            i++;
            continue;
        }
        if (i + 1 < hex.length()) {
            out.push_back(static_cast<uint8_t>(std::stoul(hex.substr(i, 2), nullptr, 16)));
            i += 2;
        } else {
            break;
        }
    }
    return out;
}

} // namespace resoem::tools
