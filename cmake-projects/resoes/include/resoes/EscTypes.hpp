/**
 * @file EscTypes.hpp
 * @brief Clean C++23 ESC, Mailbox, CoE, and Slave configuration structs with wire size checks.
 * @feature [TSK-20261113-001] [CS-0020.74] [CS-0060.6]
 */

#pragma once

#include "resoes/ResoesConstants.hpp"
#include <cstdint>

namespace resoes {

/**
 * @brief EtherCAT Application Layer states matching ETG.1000.6 encoding.
 * @feature [TSK-20261113-001]
 */
enum class AlState : uint16_t {
  None = constants::AL_STATE_NONE,
  Init = constants::AL_STATE_INIT,
  PreOp = constants::AL_STATE_PREOP,
  Boot = constants::AL_STATE_BOOT,
  SafeOp = constants::AL_STATE_SAFEOP,
  Op = constants::AL_STATE_OP
};

/**
 * @brief SyncManager ESC register block (`0x0800 + idx * 8`), wire-compatible layout.
 * @feature [TSK-20261113-001] [CS-0020.74]
 */
#pragma pack(push, 1)
struct SyncManagerRegisters {
  uint16_t physical_start = 0U; ///< Physical DPRAM start offset
  uint16_t length = 0U;         ///< Configured length in bytes
  uint8_t control = 0U;         ///< Control byte (mode, direction, interrupts)
  uint8_t status = 0U;          ///< Status byte (read/written flags)
  uint8_t activate = 0U;        ///< Activate byte (bit 0 = enable, bit 1 = repeat)
  uint8_t pdi_control = 0U;     ///< PDI control byte (bit 0 = deactivate)
};
#pragma pack(pop)
static_assert(sizeof(SyncManagerRegisters) == 8U,
              "SyncManagerRegisters wire size must be 8 bytes");

/**
 * @brief ETG.1000.6 standard 6-byte Mailbox Header (explicit fields, zero unions).
 * @feature [TSK-20261113-001] [CS-0020.74] [CS-0060.6]
 */
#pragma pack(push, 1)
struct MailboxHeader {
  uint16_t length = 0U;    ///< Payload byte length excluding 6-byte header
  uint16_t address = 0U;   ///< Station address or priority address
  uint8_t channel = 0U;    ///< Channel/priority flags (lower 6 bits + 2 priority)
  uint8_t type_cnt = 0U;   ///< Bits 0..3: mailbox type, Bits 4..6: counter

  /// @brief Extract mailbox protocol type (bits 0..3).
  [[nodiscard]] constexpr uint8_t type() const noexcept {
    return static_cast<uint8_t>(type_cnt & 0x0FU);
  }

  /// @brief Extract mailbox sequence counter (bits 4..6).
  [[nodiscard]] constexpr uint8_t counter() const noexcept {
    return static_cast<uint8_t>((type_cnt >> 4U) & 0x07U);
  }

  /// @brief Encode mailbox protocol type and counter without unions.
  constexpr void set_type_and_counter(uint8_t proto_type,
                                      uint8_t seq_cnt) noexcept {
    uint8_t masked_type = static_cast<uint8_t>(proto_type & 0x0FU);
    uint8_t masked_cnt = static_cast<uint8_t>((seq_cnt & 0x07U) << 4U);
    type_cnt = static_cast<uint8_t>(masked_type | masked_cnt);
  }
};
#pragma pack(pop)
static_assert(sizeof(MailboxHeader) == constants::MBX_HEADER_SIZE,
              "MailboxHeader wire size must be 6 bytes");

/**
 * @brief CoE 2-byte header (number + SDO/PDO/Emergency service opcode).
 * @feature [TSK-20261113-001] [CS-0020.74]
 */
#pragma pack(push, 1)
struct CoeHeader {
  uint16_t number_service = 0U; ///< Bits 0..8 number, Bits 12..15 CoE service

  /// @brief Extract CoE service code (bits 12..15).
  [[nodiscard]] constexpr uint8_t service() const noexcept {
    return static_cast<uint8_t>((number_service >> 12U) & 0x0FU);
  }

  /// @brief Extract CoE 9-bit number field (bits 0..8).
  [[nodiscard]] constexpr uint16_t number() const noexcept {
    return static_cast<uint16_t>(number_service & 0x01FFU);
  }

  /// @brief Encode CoE service and number fields cleanly.
  constexpr void set(uint8_t coe_service, uint16_t coe_number) noexcept {
    uint16_t svc_bits =
        static_cast<uint16_t>(static_cast<uint16_t>(coe_service & 0x0FU) << 12U);
    uint16_t num_bits = static_cast<uint16_t>(coe_number & 0x01FFU);
    number_service = static_cast<uint16_t>(svc_bits | num_bits);
  }
};
#pragma pack(pop)
static_assert(sizeof(CoeHeader) == constants::COE_HEADER_SIZE,
              "CoeHeader wire size must be 2 bytes");

/**
 * @brief CoE SDO 8-byte request/response header (zero C unions).
 * @feature [TSK-20261113-001] [CS-0020.74] [CS-0060.6]
 */
#pragma pack(push, 1)
struct SdoHeader {
  uint8_t command = 0U;    ///< SDO command specifier & transfer flags
  uint16_t index = 0U;     ///< Object Dictionary index
  uint8_t subindex = 0U;   ///< Object Dictionary subindex
  uint32_t size_or_data = 0U; ///< Total size, abort code, or expedited payload
};
#pragma pack(pop)
static_assert(sizeof(SdoHeader) == constants::SDO_HEADER_SIZE,
              "SdoHeader wire size must be 8 bytes");

/**
 * @brief CoE SDO Information 6-byte header (zero C unions).
 * @feature [TSK-20261113-001] [CS-0020.74]
 */
#pragma pack(push, 1)
struct SdoInfoHeader {
  uint8_t opcode = 0U;          ///< SDO Info opcode + bit 7 incomplete flag
  uint8_t reserved = 0U;        ///< Reserved byte (0)
  uint16_t fragments_left = 0U; ///< Remaining SDO Info fragments
  uint16_t info_arg = 0U;       ///< ListType / Index / First fragment word
};
#pragma pack(pop)
static_assert(sizeof(SdoInfoHeader) == constants::SDO_INFO_HEADER_SIZE,
              "SdoInfoHeader wire size must be 6 bytes");

/**
 * @brief CoE SDO Information Entry Description header (6 bytes payload prefix).
 * @feature [TSK-20261113-001] [CS-0020.74]
 */
#pragma pack(push, 1)
struct SdoEntryDescWire {
  uint16_t index = 0U;       ///< Object index
  uint8_t subindex = 0U;     ///< Object subindex
  uint8_t value_info = 0U;   ///< Value info flags
  uint16_t data_type = 0U;   ///< ETG/CANopen data type code
  uint16_t bit_length = 0U;  ///< Subindex bit length
  uint16_t obj_access = 0U;  ///< ETG access rights bitmask
};
#pragma pack(pop)
static_assert(sizeof(SdoEntryDescWire) == 10U,
              "SdoEntryDescWire wire size must be 10 bytes");

/**
 * @brief Runtime SubDevice configuration replacing compile-time SOES `options.h` macros.
 * @feature [TSK-20261113-001]
 */
struct SlaveConfig {
  uint16_t mbx_size = constants::DEFAULT_MBX_SIZE;          ///< Standard MBX length
  uint16_t mbx_boot_size = constants::DEFAULT_MBX_BOOT_SIZE;///< BOOT MBX length
  uint16_t mbx0_address = constants::DEFAULT_MBX0_ADDR;     ///< SM0 DPRAM offset
  uint16_t mbx1_address = constants::DEFAULT_MBX1_ADDR;     ///< SM1 DPRAM offset
  uint16_t mbx0_boot_address = constants::DEFAULT_MBX0_ADDR;///< BOOT SM0 offset
  uint16_t mbx1_boot_address = constants::DEFAULT_MBX1_ADDR;///< BOOT SM1 offset
  uint16_t sm2_address = constants::DEFAULT_SM2_ADDR;       ///< SM2 RxPDO offset
  uint16_t sm3_address = constants::DEFAULT_SM3_ADDR;       ///< SM3 TxPDO offset
  uint16_t max_rx_pdo_bytes = 256U;                         ///< Max SM2 bytes
  uint16_t max_tx_pdo_bytes = 256U;                         ///< Max SM3 bytes
  // Mailbox slot count and protocol feature toggles
  uint8_t mailbox_slots = static_cast<uint8_t>(constants::DEFAULT_MBX_BUFFERS); ///< Pool size
  bool boot_supported = false;                              ///< BOOT state allowed
  bool use_foe = false;                                     ///< FoE server flag
  bool use_eoe = false;                                     ///< EoE server flag
  uint8_t eeprom_read_size = constants::EEP_READ_SIZE_8;    ///< SII read width (4/8)
};

} // namespace resoes
