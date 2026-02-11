#pragma once

#include <cstdint>
#include <string_view>

namespace resoem {

/**
 * Get a human-readable string for an SDO abort code.
 */
std::string_view sdo_abort_to_string(uint32_t abort_code);

/**
 * Get a human-readable string for an AL Status code.
 */
std::string_view al_status_code_to_string(uint16_t status_code);

} // namespace resoem
