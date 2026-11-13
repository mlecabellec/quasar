# Diagnostics

## Overview
`resoem::Diagnostics` (`cmake-projects/resoem/include/resoem/Diagnostics.hpp`) provides human-readable decoding helpers for EtherCAT protocol diagnostic codes.

## Functions
- **`sdo_abort_to_string(uint32_t abort_code)`**: Decodes ETG.1000 CoE SDO abort codes into `std::string_view`.
- **`al_status_code_to_string(uint16_t status_code)`**: Decodes EtherCAT Application Layer (AL) status codes (register `0x0134`) into `std::string_view`.
