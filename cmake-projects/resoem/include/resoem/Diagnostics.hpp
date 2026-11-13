/**
 * @file Diagnostics.hpp
 * @brief Diagnostic utilities for EtherCAT error codes.
 */

#pragma once

#include <cstdint>
#include <string_view>

namespace resoem {

/// @brief Get a human-readable string for an SDO abort code.
std::string_view sdo_abort_to_string(uint32_t abort_code);

/// @brief Get a human-readable string for an AL Status code (register 0x0134).
std::string_view al_status_code_to_string(uint16_t status_code);

} // namespace resoem
