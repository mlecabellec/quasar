#include "resoem/Diagnostics.hpp"
#include <map>

namespace resoem {

std::string_view sdo_abort_to_string(uint32_t abort_code) {
  // Static map of standard SDO abort codes as defined in the EtherCAT
  // specification.
  static const std::map<uint32_t, std::string_view> sdo_error_list = {
      {0x00000000, "No error"},
      {0x05030000, "Toggle bit not changed"},
      {0x05040000, "SDO protocol timeout"},
      {0x05040001, "Client/Server command specifier not valid or unknown"},
      {0x05040005, "Out of memory"},
      {0x06010000, "Unsupported access to an object"},
      {0x06010001, "Attempt to read to a write only object"},
      {0x06010002, "Attempt to write to a read only object"},
      {0x06010003,
       "Subindex can not be written, SI0 must be 0 for write access"},
      {0x06010004,
       "SDO Complete access not supported for variable length objects"},
      {0x06010005, "Object length exceeds mailbox size"},
      {0x06010006, "Object mapped to RxPDO, SDO download blocked"},
      {0x06020000, "The object does not exist in the object directory"},
      {0x06040041, "The object can not be mapped into the PDO"},
      {0x06040042, "The number and length of the objects to be mapped would "
                   "exceed the PDO length"},
      {0x06040043, "General parameter incompatibility reason"},
      {0x06040047, "General internal incompatibility in the device"},
      {0x06060000, "Access failed due to a hardware error"},
      {0x06070010,
       "Data type does not match, length of service parameter does not match"},
      {0x06070012,
       "Data type does not match, length of service parameter too high"},
      {0x06070013,
       "Data type does not match, length of service parameter too low"},
      {0x06090011, "Subindex does not exist"},
      {0x06090030, "Value range of parameter exceeded (only for write access)"},
      {0x06090031, "Value of parameter written too high"},
      {0x06090032, "Value of parameter written too low"},
      {0x06090036, "Maximum value is less than minimum value"},
      {0x08000000, "General error"},
      {0x08000020, "Data cannot be transferred or stored to the application"},
      {0x08000021, "Data cannot be transferred or stored to the application "
                   "because of local control"},
      {0x08000022, "Data cannot be transferred or stored to the application "
                   "because of the present device state"},
      {0x08000023, "Object dictionary dynamic generation fails or no object "
                   "dictionary is present"},
  };

  // Perform lookup in the error list.
  std::map<uint32_t, std::string_view>::const_iterator it =
      sdo_error_list.find(abort_code);
  if (it != sdo_error_list.end()) {
    return it->second;
  }
  return "Unknown SDO abort code";
}

std::string_view al_status_code_to_string(uint16_t status_code) {
  // Static map of Application Layer (AL) Status codes from register 0x0134.
  static const std::map<uint16_t, std::string_view> al_status_list = {
      {0x0000, "No error"},
      {0x0001, "Unspecified error"},
      {0x0002, "No memory"},
      {0x0003, "Invalid device setup"},
      {0x0004, "Invalid revision"},
      {0x0006, "SII/EEPROM information does not match firmware"},
      {0x0007, "Firmware update not successful. Old firmware still running"},
      {0x000E, "License error"},
      {0x0011, "Invalid requested state change"},
      {0x0012, "Unknown requested state"},
      {0x0013, "Bootstrap not supported"},
      {0x0014, "No valid firmware"},
      {0x0015, "Invalid mailbox configuration"},
      {0x0016, "Invalid mailbox configuration"},
      {0x0017, "Invalid sync manager configuration"},
      {0x0018, "No valid inputs available"},
      {0x0019, "No valid outputs"},
      {0x001A, "Synchronization error"},
      {0x001B, "Sync manager watchdog"},
      {0x001C, "Invalid sync Manager types"},
      {0x001D, "Invalid output configuration"},
      {0x001E, "Invalid input configuration"},
      {0x001F, "Invalid watchdog configuration"},
      {0x0020, "Slave needs cold start"},
      {0x0021, "Slave needs INIT"},
      {0x0022, "Slave needs PREOP"},
      {0x0023, "Slave needs SAFEOP"},
      {0x0024, "Invalid input mapping"},
      {0x0025, "Invalid output mapping"},
      {0x0026, "Inconsistent settings"},
      {0x0027, "Freerun not supported"},
      {0x0028, "Synchronisation not supported"},
      {0x0029, "Freerun needs 3buffer mode"},
      {0x002A, "Background watchdog"},
      {0x002B, "No valid Inputs and Outputs"},
      {0x002C, "Fatal sync error"},
      {0x002D, "No sync error"},
      {0x002E, "Invalid input FMMU configuration"},
      {0x0030, "Invalid DC SYNC configuration"},
      {0x0031, "Invalid DC latch configuration"},
      {0x0032, "PLL error"},
      {0x0033, "DC sync IO error"},
      {0x0034, "DC sync timeout error"},
      {0x0035, "DC invalid sync cycle time"},
      {0x0036, "DC invalid sync0 cycle time"},
      {0x0037, "DC invalid sync1 cycle time"},
      {0x0041, "MBX_AOE"},
      {0x0042, "MBX_EOE"},
      {0x0043, "MBX_COE"},
      {0x0044, "MBX_FOE"},
      {0x0045, "MBX_SOE"},
      {0x004F, "MBX_VOE"},
      {0x0050, "EEPROM no access"},
      {0x0051, "EEPROM error"},
      {0x0052, "External hardware not ready"},
      {0x0060, "Slave restarted locally"},
      {0x0061, "Device identification value updated"},
      {0x0070, "Detected Module Ident List does not match"},
      {0x0080, "Supply voltage too low"},
      {0x0081, "Supply voltage too high"},
      {0x0082, "Temperature too low"},
      {0x0083, "Temperature too high"},
      {0x00f0, "Application controller available"},
  };

  // Perform lookup in the AL status list.
  std::map<uint16_t, std::string_view>::const_iterator it =
      al_status_list.find(status_code);
  if (it != al_status_list.end()) {
    return it->second;
  }
  return "Unknown AL Status code";
}

} // namespace resoem
