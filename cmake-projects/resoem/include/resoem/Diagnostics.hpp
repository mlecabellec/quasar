/**
 * @file Diagnostics.hpp
 * @brief Diagnostic utilities for EtherCAT error codes.
 */

#pragma once

#include <cstdint>
#include <string_view>

namespace resoem {

/**
 * @brief Get a human-readable string for an SDO abort code.
 * @param abort_code The 32-bit SDO abort code.
 * @return A string view containing the error description.
 */
std::string_view sdo_abort_to_string(uint32_t abort_code);

/**
 * @brief Get a human-readable string for an AL Status code.
 * @param status_code The 16-bit AL status code from register 0x0134.
 * @return A string view containing the error description.
 */
std::string_view al_status_code_to_string(uint16_t status_code);

} // namespace resoem
