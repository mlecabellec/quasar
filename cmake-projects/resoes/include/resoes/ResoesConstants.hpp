/**
 * @file ResoesConstants.hpp
 * @brief Symbolic ETG.1000 / ETG.1020 constants, ESC register offsets, and safety bounds for resoes.
 * @feature [TSK-20261113-001] [CS-0040]
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace resoes::constants {

// =============================================================================
// Safety & Concurrency Limits [CS-0010.37] [CS-0040.2]
// =============================================================================

/// @brief Size of simulated ESC dual-port address space in bytes (8 KiB).
constexpr size_t ESC_MEMORY_SIZE_BYTES = 8192U;
/// @brief Hard iteration bound for cyclic mailbox / state machine loops.
constexpr size_t MAX_STATE_LOOP_ITERATIONS = 64U;
/// @brief Hard iteration bound for Object Dictionary / PDO scans.
constexpr size_t MAX_OD_ITERATIONS = 4096U;
/// @brief Hard iteration bound for segmented SDO payload transfers.
constexpr size_t MAX_SDO_SEGMENTS = 1024U;
/// @brief Timed mutex lock acquisition timeout in milliseconds [CS-0010.46].
constexpr int MUTEX_TIMEOUT_MS = 100;
/// @brief Bits per byte helper constant.
constexpr size_t BITS_PER_BYTE = 8U;
/// @brief Bits per 64-bit fast PDO packing word.
constexpr size_t BITS_PER_U64 = 64U;

// =============================================================================
// ESC Register Map (ETG.1000.4)
// =============================================================================

constexpr uint16_t ESCREG_ADDRESS = 0x0010U;      ///< Configured station address
constexpr uint16_t ESCREG_DLSTATUS = 0x0110U;     ///< Data Link status
constexpr uint16_t ESCREG_ALCONTROL = 0x0120U;    ///< AL Control register
constexpr uint16_t ESCREG_ALSTATUS = 0x0130U;     ///< AL Status register
constexpr uint16_t ESCREG_ALERROR = 0x0134U;      ///< AL Status Code register
constexpr uint16_t ESCREG_ALEVENT_MASK = 0x0204U; ///< AL Event mask (PDI)
constexpr uint16_t ESCREG_ALEVENT = 0x0220U;      ///< AL Event request (PDI)

constexpr uint16_t ESCREG_WD_DIV = 0x0400U;     ///< Watchdog divider
constexpr uint16_t ESCREG_WD_PDI = 0x0410U;     ///< PDI watchdog time
constexpr uint16_t ESCREG_WD_PDATA = 0x0420U;   ///< Process data watchdog time
constexpr uint16_t ESCREG_WDSTATUS = 0x0440U;   ///< Watchdog status process data

constexpr uint16_t ESCREG_EEPCFG = 0x0500U;     ///< EEPROM config register
constexpr uint16_t ESCREG_EECONTSTAT = 0x0502U; ///< EEPROM control/status
constexpr uint16_t ESCREG_EEADDR = 0x0504U;     ///< EEPROM word address
constexpr uint16_t ESCREG_EEDATA = 0x0508U;     ///< EEPROM data buffer (8 bytes)

constexpr uint16_t ESCREG_SM0 = 0x0800U;        ///< SyncManager 0 base
constexpr uint16_t ESCREG_SM_STRIDE = 0x0008U;  ///< SyncManager register stride
constexpr uint16_t ESCREG_SM0_STATUS = 0x0805U; ///< SM0 status byte
constexpr uint16_t ESCREG_SM0_ACTIVATE = 0x0806U; ///< SM0 activate byte
constexpr uint16_t ESCREG_SM0_PDI_CTRL = 0x0807U; ///< SM0 PDI control byte
constexpr uint16_t ESCREG_SM1_STATUS = 0x080DU; ///< SM1 status byte
constexpr uint16_t ESCREG_SM1_ACTIVATE = 0x080EU; ///< SM1 activate byte
constexpr uint16_t ESCREG_SM1_PDI_CTRL = 0x080FU; ///< SM1 PDI control byte
constexpr uint16_t ESCREG_SM2_STATUS = 0x0815U; ///< SM2 status byte
constexpr uint16_t ESCREG_SM2_ACTIVATE = 0x0816U; ///< SM2 activate byte
constexpr uint16_t ESCREG_SM2_PDI_CTRL = 0x0817U; ///< SM2 PDI control byte
constexpr uint16_t ESCREG_SM3_STATUS = 0x081DU; ///< SM3 status byte
constexpr uint16_t ESCREG_SM3_ACTIVATE = 0x081EU; ///< SM3 activate byte
constexpr uint16_t ESCREG_SM3_PDI_CTRL = 0x081FU; ///< SM3 PDI control byte

constexpr uint16_t ESCREG_CYCLIC_UNIT_CTRL = 0x0980U; ///< DC activation ctrl
constexpr uint16_t ESCREG_SYNC_ACT = 0x0981U;         ///< DC SYNC activation
constexpr uint16_t ESCREG_SYNC0_CYCLE = 0x09A0U;      ///< DC SYNC0 cycle time
constexpr uint16_t ESCREG_SYNC1_CYCLE = 0x09A4U;      ///< DC SYNC1 cycle time

// =============================================================================
// AL States, Event Bits & Status Error Codes (ETG.1000.6)
// =============================================================================

constexpr uint16_t AL_STATE_NONE = 0x0000U;   ///< Uninitialized state
constexpr uint16_t AL_STATE_INIT = 0x0001U;   ///< Init state
constexpr uint16_t AL_STATE_PREOP = 0x0002U;  ///< Pre-Operational state
constexpr uint16_t AL_STATE_BOOT = 0x0003U;   ///< Bootstrap state
constexpr uint16_t AL_STATE_SAFEOP = 0x0004U; ///< Safe-Operational state
constexpr uint16_t AL_STATE_OP = 0x0008U;     ///< Operational state
constexpr uint16_t AL_STATE_MASK = 0x000FU;   ///< Mask for AL state bits
constexpr uint16_t AL_STATE_ERROR_BIT = 0x0010U; ///< Error indicator bit
constexpr uint16_t AL_STATE_ACK_BIT = 0x0010U;   ///< AL Control Error ACK bit

constexpr uint16_t ESC_DL_EVENT_SM0_FULL = 0x0100U;   ///< AL Event SM0 written
constexpr uint16_t ESC_DL_EVENT_SM1_READ = 0x0200U;   ///< AL Event SM1 read
constexpr uint16_t ESC_DL_EVENT_SM2_EVENT = 0x0400U;  ///< AL Event SM2 output
constexpr uint16_t ESC_DL_EVENT_SM3_EVENT = 0x0800U;  ///< AL Event SM3 input
constexpr uint16_t ESC_AL_EVENT_AL_CONTROL = 0x0001U; ///< AL Control changed
constexpr uint16_t ESC_AL_EVENT_DC_SYNC0 = 0x0004U;   ///< DC SYNC0 event
constexpr uint16_t ESC_AL_EVENT_DC_SYNC1 = 0x0008U;   ///< DC SYNC1 event
constexpr uint16_t ESC_AL_EVENT_SM_CHANGE = 0x0010U;  ///< SyncManager activated
constexpr uint16_t ESC_AL_EVENT_EEP_CMD = 0x0020U;    ///< EEPROM emulation req

constexpr uint16_t ALERR_NONE = 0x0000U;                   ///< No error
constexpr uint16_t ALERR_UNSPECIFIED = 0x0001U;            ///< Unspecified error
constexpr uint16_t ALERR_NO_MEMORY = 0x0002U;              ///< No memory
constexpr uint16_t ALERR_INVALID_STATE_CHANGE = 0x0011U;   ///< Invalid transition
constexpr uint16_t ALERR_UNKNOWN_STATE = 0x0012U;          ///< Unknown AL state
constexpr uint16_t ALERR_BOOT_NOT_SUPPORTED = 0x0013U;     ///< Boot unsupported
constexpr uint16_t ALERR_INVALID_MAILBOX_CFG_BOOT = 0x0015U;///< Bad boot MBX cfg
constexpr uint16_t ALERR_INVALID_MAILBOX_CFG_PREOP = 0x0016U;///< Bad preop MBX cfg
constexpr uint16_t ALERR_INVALID_SM_CFG = 0x0017U;         ///< Bad SM config
constexpr uint16_t ALERR_NO_VALID_INPUTS = 0x0018U;        ///< No valid inputs
constexpr uint16_t ALERR_NO_VALID_OUTPUTS = 0x0019U;       ///< No valid outputs
constexpr uint16_t ALERR_SYNC_ERROR = 0x001AU;             ///< Sync error
// Watchdog and SyncManager validation codes
constexpr uint16_t ALERR_WATCHDOG = 0x001BU;               ///< SM watchdog expired
constexpr uint16_t ALERR_INVALID_SM_TYPE = 0x001CU;        ///< Invalid SM type
constexpr uint16_t ALERR_INVALID_OUTPUT_CFG = 0x001DU;     ///< Bad SM2 config
constexpr uint16_t ALERR_INVALID_INPUT_CFG = 0x001EU;      ///< Bad SM3 config
constexpr uint16_t ALERR_INVALID_WD_CFG = 0x001FU;         ///< Invalid WD config
constexpr uint16_t ALERR_DC_INVALID_SYNC_CFG = 0x0030U;    ///< Bad DC sync cfg
constexpr uint16_t ALERR_DC_INVALID_LATCH_CFG = 0x0031U;   ///< Bad DC latch cfg
constexpr uint16_t ALERR_DC_PLLC_ERROR = 0x0032U;          ///< PLL sync error
constexpr uint16_t ALERR_DC_SYNC_IO_ERROR = 0x0033U;       ///< Sync IO error
constexpr uint16_t ALERR_DC_SYNC_TIMEOUT = 0x0034U;        ///< Sync timeout
constexpr uint16_t ALERR_EEPROM_NO_ACCESS = 0x0050U;       ///< EEPROM inaccessible

// =============================================================================
// SyncManager, Mailbox & Default Slave Layout Constants
// =============================================================================

constexpr size_t SM_COUNT = 4U;                 ///< Standard ESC SyncManagers 0..3
constexpr uint8_t SM_INDEX_MBX_OUT = 0U;        ///< SM0: Master->Slave Mailbox
constexpr uint8_t SM_INDEX_MBX_IN = 1U;         ///< SM1: Slave->Master Mailbox
constexpr uint8_t SM_INDEX_PDO_OUT = 2U;        ///< SM2: Process Data Outputs
constexpr uint8_t SM_INDEX_PDO_IN = 3U;         ///< SM3: Process Data Inputs

constexpr uint8_t SM_ACTIVATE_ENABLE = 0x01U;   ///< SM activate enable bit
constexpr uint8_t SM_ACTIVATE_REPEAT = 0x02U;   ///< SM repeat request bit
constexpr uint8_t SM_PDI_CTRL_DEACTIVATE = 0x01U; ///< SM PDI disable bit
constexpr uint8_t SM_STATUS_MBX_FULL = 0x08U;   ///< Mailbox full status flag
constexpr uint8_t SM_STATUS_REPEAT_ACK = 0x02U; ///< Repeat ACK status bit
constexpr uint8_t SM_CTRL_DIR_WRITE = 0x04U;    ///< ECAT writes, PDI reads
constexpr uint8_t SM_CTRL_DIR_READ = 0x00U;     ///< ECAT reads, PDI writes
constexpr uint8_t SM_CTRL_MODE_MAILBOX = 0x02U; ///< One-buffer mailbox mode
constexpr uint8_t SM_CTRL_MODE_MASK = 0x03U;    ///< Operation mode mask
constexpr uint8_t SM_CTRL_DIR_MASK = 0x0CU;     ///< Direction field mask

constexpr uint16_t DEFAULT_MBX_SIZE = 128U;      ///< Default mailbox buffer bytes
constexpr uint16_t DEFAULT_MBX_BOOT_SIZE = 128U; ///< Default BOOT mailbox bytes
constexpr size_t DEFAULT_MBX_BUFFERS = 3U;       ///< Default mailbox slot count
constexpr uint16_t DEFAULT_MBX0_ADDR = 0x1000U;  ///< Default SM0 start address
constexpr uint16_t DEFAULT_MBX1_ADDR = 0x1080U;  ///< Default SM1 start address
constexpr uint16_t DEFAULT_SM2_ADDR = 0x1100U;   ///< Default SM2 start address
constexpr uint16_t DEFAULT_SM3_ADDR = 0x1180U;   ///< Default SM3 start address
constexpr size_t MBX_HEADER_SIZE = 6U;           ///< ETG.1000 mailbox header bytes
constexpr uint8_t MBX_MAX_COUNTER = 7U;          ///< Valid mailbox counter max

// Mailbox Protocol Types (ETG.1000.6 Table 29)
constexpr uint8_t MBX_TYPE_ERROR = 0x00U; ///< Mailbox error reply
constexpr uint8_t MBX_TYPE_AOE = 0x01U;   ///< ADS over EtherCAT
constexpr uint8_t MBX_TYPE_EOE = 0x02U;   ///< Ethernet over EtherCAT
constexpr uint8_t MBX_TYPE_COE = 0x03U;   ///< CANopen over EtherCAT
constexpr uint8_t MBX_TYPE_FOE = 0x04U;   ///< File over EtherCAT
constexpr uint8_t MBX_TYPE_SOE = 0x05U;   ///< Servo over EtherCAT

// Mailbox Error Codes
constexpr uint16_t MBXERR_SYNTAX = 0x0001U;           ///< Syntax error
constexpr uint16_t MBXERR_UNSUPPORTED = 0x0002U;      ///< Unsupported protocol
constexpr uint16_t MBXERR_INVALID_CHANNEL = 0x0003U;  ///< Invalid channel
constexpr uint16_t MBXERR_SERVICE_NOT_SUPPORTED = 0x0004U; ///< Bad service
constexpr uint16_t MBXERR_INVALID_HEADER = 0x0005U;   ///< Invalid header
constexpr uint16_t MBXERR_SIZE_TOO_SHORT = 0x0006U;   ///< Packet too short
constexpr uint16_t MBXERR_NO_MORE_MEMORY = 0x0007U;   ///< Buffer exhausted
constexpr uint16_t MBXERR_INVALID_SIZE = 0x0008U;     ///< Invalid size

// =============================================================================
// CoE / SDO / SDO Information Constants
// =============================================================================

constexpr uint8_t COE_SERVICE_EMERGENCY = 0x01U;   ///< CoE Emergency
constexpr uint8_t COE_SERVICE_SDOREQ = 0x02U;      ///< SDO Request
constexpr uint8_t COE_SERVICE_SDORES = 0x03U;      ///< SDO Response
constexpr uint8_t COE_SERVICE_TXPDO = 0x04U;       ///< TxPDO over MBX
constexpr uint8_t COE_SERVICE_RXPDO = 0x05U;       ///< RxPDO over MBX
constexpr uint8_t COE_SERVICE_SDOINFO = 0x08U;     ///< SDO Information

constexpr uint8_t COE_HEADER_SIZE = 2U;            ///< CoE header size
constexpr uint8_t SDO_HEADER_SIZE = 8U;            ///< SDO CS + Index + Sub + Size
constexpr uint8_t SDO_INFO_HEADER_SIZE = 6U;       ///< SDO Info header bytes
constexpr size_t SDO_EXPEDITED_MAX_BYTES = 4U;     ///< Expedited payload limit

// SDO Client->Server Command Specifier (CCS in bits 7..5)
constexpr uint8_t SDO_CCS_DOWNLOAD_SEG_REQ = 0x00U;  ///< Segmented download
constexpr uint8_t SDO_CCS_DOWNLOAD_INIT_REQ = 0x20U; ///< Initiate download
constexpr uint8_t SDO_CCS_UPLOAD_INIT_REQ = 0x40U;   ///< Initiate upload
constexpr uint8_t SDO_CCS_UPLOAD_SEG_REQ = 0x60U;    ///< Segmented upload
constexpr uint8_t SDO_CCS_ABORT_REQ = 0x80U;         ///< Abort transfer
constexpr uint8_t SDO_CCS_MASK = 0xE0U;              ///< CCS mask

// SDO Server->Client Command Specifier (SCS in bits 7..5)
constexpr uint8_t SDO_SCS_UPLOAD_SEG_RES = 0x00U;    ///< Segmented upload resp
constexpr uint8_t SDO_SCS_DOWNLOAD_SEG_RES = 0x20U;  ///< Segmented download ack
constexpr uint8_t SDO_SCS_UPLOAD_INIT_RES = 0x40U;   ///< Initiate upload resp
constexpr uint8_t SDO_SCS_DOWNLOAD_INIT_RES = 0x60U; ///< Initiate download resp

constexpr uint8_t SDO_FLAG_SIZE_IND = 0x01U;         ///< Size indicator bit
constexpr uint8_t SDO_FLAG_EXPEDITED = 0x02U;        ///< Expedited transfer bit
constexpr uint8_t SDO_FLAG_COMPLETE_ACCESS = 0x10U;  ///< Complete Access bit
constexpr uint8_t SDO_FLAG_TOGGLE = 0x10U;           ///< Segment toggle bit
constexpr uint8_t SDO_FLAG_MORE_FOLLOWS_LAST = 0x01U;///< Segmented last chunk
constexpr uint8_t SDO_SIZE_SHIFT = 2U;               ///< Size field bit shift

// SDO Information Opcodes
constexpr uint8_t SDOINFO_OP_GET_ODLIST_REQ = 0x01U; ///< Get OD list req
constexpr uint8_t SDOINFO_OP_GET_ODLIST_RES = 0x02U; ///< Get OD list resp
constexpr uint8_t SDOINFO_OP_GET_OD_REQ = 0x03U;     ///< Get OD req
constexpr uint8_t SDOINFO_OP_GET_OD_RES = 0x04U;     ///< Get OD resp
constexpr uint8_t SDOINFO_OP_GET_ED_REQ = 0x05U;     ///< Get Entry desc req
constexpr uint8_t SDOINFO_OP_GET_ED_RES = 0x06U;     ///< Get Entry desc resp
constexpr uint8_t SDOINFO_OP_ERROR_RES = 0x07U;      ///< SDO Info error
constexpr uint8_t SDOINFO_INCOMPLETE_BIT = 0x80U;    ///< Incomplete flag

// Standard Object Dictionary Indexes
constexpr uint16_t OD_IDX_SM2_ASSIGN = 0x1C12U;      ///< SM2 RxPDO assign
constexpr uint16_t OD_IDX_SM3_ASSIGN = 0x1C13U;      ///< SM3 TxPDO assign
constexpr uint16_t OD_IDX_RXPDO_MAP_MIN = 0x1600U;   ///< RxPDO map start
constexpr uint16_t OD_IDX_RXPDO_MAP_MAX = 0x17FFU;   ///< RxPDO map end
constexpr uint16_t OD_IDX_TXPDO_MAP_MIN = 0x1A00U;   ///< TxPDO map start
constexpr uint16_t OD_IDX_TXPDO_MAP_MAX = 0x1BFFU;   ///< TxPDO map end

// SDO Abort Codes (CiA 301 / ETG.1000.6)
constexpr uint32_t SDO_ABORT_TOGGLE = 0x05030000U;
constexpr uint32_t SDO_ABORT_GENERAL = 0x05040000U;
constexpr uint32_t SDO_ABORT_CMD_INVALID = 0x05040001U;         ///< Invalid command specifier
constexpr uint32_t SDO_ABORT_UNSUPPORTED = 0x06010000U;         ///< Unsupported access
constexpr uint32_t SDO_ABORT_WRITE_ONLY = 0x06010001U;          ///< Write-only object
constexpr uint32_t SDO_ABORT_READ_ONLY = 0x06010002U;           ///< Read-only object
constexpr uint32_t SDO_ABORT_SUBINDEX0_NOT_ZERO = 0x06010003U;  ///< Subindex 0 non-zero
constexpr uint32_t SDO_ABORT_NO_CA = 0x06010004U;               ///< Complete Access unsupported
constexpr uint32_t SDO_ABORT_OBJ_NOT_EXIST = 0x06020000U;       ///< Object missing in OD
constexpr uint32_t SDO_ABORT_TYPE_MISMATCH = 0x06070010U;       ///< Data type mismatch
constexpr uint32_t SDO_ABORT_DATA_TOO_LONG = 0x06070012U;       ///< Data parameter too high
constexpr uint32_t SDO_ABORT_DATA_TOO_SHORT = 0x06070013U;      ///< Data parameter too short
constexpr uint32_t SDO_ABORT_SUBINDEX_NOT_EXIST = 0x06090011U;  ///< Subindex missing
constexpr uint32_t SDO_ABORT_VALUE_RANGE = 0x06090030U;         ///< Value range exceeded
// Generic parameter & AL state abort codes
constexpr uint32_t SDO_ABORT_GENERAL_PARAM = 0x08000000U;       ///< General parameter error
constexpr uint32_t SDO_ABORT_WRONG_STATE = 0x08000021U;         ///< Illegal AL state for access

// =============================================================================
// SII EEPROM Emulation Constants (0x0500..0x0515)
// =============================================================================

constexpr uint16_t EEP_CMD_IDLE = 0x0000U;       ///< Idle command
constexpr uint16_t EEP_CMD_READ = 0x0100U;       ///< Read 4 or 8 bytes
constexpr uint16_t EEP_CMD_WRITE = 0x0200U;      ///< Write 2 bytes (1 word)
constexpr uint16_t EEP_CMD_RELOAD = 0x0400U;     ///< Reload configuration
constexpr uint16_t EEP_CMD_MASK = 0x0700U;       ///< Command mask in 0x0502
constexpr uint16_t EEP_STAT_READBYTES_8 = 0x0040U;///< 8-byte read capability bit
constexpr uint16_t EEP_STAT_ERROR_MASK = 0x7800U; ///< Error bits mask
constexpr uint16_t EEP_STAT_ACK_ERR = 0x2000U;   ///< Acknowledge/missing error
constexpr uint16_t EEP_STAT_WRITE_ERR = 0x4000U; ///< Write/CRC error
constexpr uint16_t EEP_STAT_BUSY = 0x8000U;      ///< EEPROM busy bit
constexpr uint8_t EEP_READ_SIZE_4 = 4U;          ///< 4-byte read granularity
constexpr uint8_t EEP_READ_SIZE_8 = 8U;          ///< 8-byte read granularity
constexpr size_t EEP_WORD_SIZE_BYTES = 2U;       ///< SII word address step (2B)
constexpr size_t EEP_DEFAULT_SIZE_BYTES = 2048U; ///< 16 Kbit default SII size

} // namespace resoes::constants
