/**
 * @file EtherCATTypes.hpp
 * @brief EtherCAT register definitions, commands, and protocol headers.
 */

#pragma once

#include <cstdint>

namespace resoem {

/**
 * @brief EtherCAT slave controller (ESC) registers.
 */
namespace regs {
constexpr uint16_t TYPE = 0x0000;         ///< ESC Type
constexpr uint16_t REV = 0x0001;          ///< ESC Revision
constexpr uint16_t BUILD = 0x0002;        ///< ESC Build
constexpr uint16_t FMMUS = 0x0004;        ///< Number of FMMUs supported
constexpr uint16_t SM = 0x0005;           ///< Number of SyncManagers supported
constexpr uint16_t PORTS = 0x0007;        ///< Number of ports
constexpr uint16_t ESC_FEATURES = 0x0008; ///< ESC features supported

constexpr uint16_t CONFIG_STATION_ALIAS = 0x0012; ///< Configured Station Alias
constexpr uint16_t CONFIG_STATION_ADDR = 0x0010; ///< Configured Station Address

constexpr uint16_t DL_PORT = 0x0101;    ///< DL Port control
constexpr uint16_t DL_CONTROL = 0x0100; ///< DL Control
constexpr uint16_t DL_STATUS = 0x0110;  ///< DL Status
constexpr uint16_t DL_ALIAS = 0x0103;   ///< DL Alias

constexpr uint16_t IRQ_MASK = 0x0200; ///< Interrupt Mask
constexpr uint16_t RX_ERR = 0x0300;   ///< RX Error counter

// FMMU Registers
constexpr uint16_t FMMU0 = 0x0600;
constexpr uint16_t FMMU1 = 0x0610;
constexpr uint16_t FMMU2 = 0x0620;
constexpr uint16_t FMMU3 = 0x0630;
constexpr uint16_t FMMU4 = 0x0640;
constexpr uint16_t FMMU5 = 0x0650;
constexpr uint16_t FMMU6 = 0x0660;
constexpr uint16_t FMMU7 = 0x0670;
constexpr uint16_t FMMU8 = 0x0680;
constexpr uint16_t FMMU9 = 0x0690;
constexpr uint16_t FMMU10 = 0x06A0;
constexpr uint16_t FMMU11 = 0x06B0;
constexpr uint16_t FMMU12 = 0x06C0;
constexpr uint16_t FMMU13 = 0x06D0;
constexpr uint16_t FMMU14 = 0x06E0;
constexpr uint16_t FMMU15 = 0x06F0;

// Distributed Clocks (DC) Registers
constexpr uint16_t DC_SYNC_ACT = 0x0981;           ///< DC Sync activation
constexpr uint16_t DC_RECEIVE_TIME_PORT0 = 0x0900; ///< DC Receive time Port 0
constexpr uint16_t DC_RECEIVE_TIME_PORT1 = 0x0904; ///< DC Receive time Port 1
constexpr uint16_t DC_RECEIVE_TIME_PORT2 = 0x0908; ///< DC Receive time Port 2
constexpr uint16_t DC_RECEIVE_TIME_PORT3 = 0x090C; ///< DC Receive time Port 3
constexpr uint16_t DC_SYS_TIME = 0x0910;           ///< DC System time
constexpr uint16_t DC_SYS_TIME_OFFSET = 0x0920;    ///< DC System time offset
constexpr uint16_t DC_SYS_TIME_DELAY = 0x0928;     ///< DC System time delay
constexpr uint16_t DC_SYS_TIME_DIFF = 0x092C; ///< DC System time difference
constexpr uint16_t DC_SPEED_CNT = 0x0930;     ///< DC Speed counter
constexpr uint16_t DC_TIME_FILT = 0x0934;     ///< DC Time filter

constexpr uint16_t DC_CYCLIC_UNIT_CTRL = 0x0980; ///< DC Cyclic unit control
constexpr uint16_t DC_SYNC_PULSE_LEN = 0x0982;   ///< DC Sync pulse length
constexpr uint16_t DC_SYNC_START_TIME = 0x0990;  ///< DC Sync start time
constexpr uint16_t DC_SYNC0_CYCLE_TIME = 0x09A0; ///< DC Sync0 cycle time
constexpr uint16_t DC_SYNC1_CYCLE_TIME = 0x09A4; ///< DC Sync1 cycle time

// SyncManager (SM) Registers
constexpr uint16_t SM0 = 0x0800;
constexpr uint16_t SM1 = 0x0808;
constexpr uint16_t SM2 = 0x0810;
constexpr uint16_t SM3 = 0x0818;
constexpr uint16_t SM4 = 0x0820;
constexpr uint16_t SM5 = 0x0828;
constexpr uint16_t SM6 = 0x0830;
constexpr uint16_t SM7 = 0x0838;
constexpr uint16_t SM8 = 0x0840;
constexpr uint16_t SM9 = 0x0848;
constexpr uint16_t SM10 = 0x0850;
constexpr uint16_t SM11 = 0x0858;
constexpr uint16_t SM12 = 0x0860;
constexpr uint16_t SM13 = 0x0868;
constexpr uint16_t SM14 = 0x0870;
constexpr uint16_t SM15 = 0x0878;
constexpr uint8_t SM_STATUS_OFFSET = 5;

/**
 * @brief SyncManager status bits.
 */
namespace sm_status {
constexpr uint8_t MBX_FULL = 0x08; ///< Mailbox is full (bit 3)
}

constexpr uint16_t AL_CONTROL = 0x0120;     ///< Application Layer Control
constexpr uint16_t AL_STATUS = 0x0130;      ///< Application Layer Status
constexpr uint16_t AL_STATUS_CODE = 0x0134; ///< Application Layer Status Code

/**
 * @brief Application Layer status masks and bits.
 */
namespace al_status {
constexpr uint16_t STATE_MASK = 0x000F; ///< Mask for the state bits
constexpr uint16_t ERROR_BIT = 0x0010;  ///< Error bit (bit 4)
} // namespace al_status

constexpr uint16_t PDI_CONTROL = 0x0140; ///< Process Data Interface Control
constexpr uint16_t PDI_CONFIG =
    0x0150; ///< Process Data Interface Configuration

// ESC Error Counters
constexpr uint16_t RX_ERR_COUNT_PORT0 = 0x0300; ///< RX Error Counter Port 0
constexpr uint16_t RX_ERR_COUNT_PORT1 = 0x0302; ///< RX Error Counter Port 1
constexpr uint16_t RX_ERR_COUNT_PORT2 = 0x0304; ///< RX Error Counter Port 2
constexpr uint16_t RX_ERR_COUNT_PORT3 = 0x0306; ///< RX Error Counter Port 3
constexpr uint16_t FWD_RX_ERR_COUNT_PORT0 =
    0x0308; ///< Forwarded RX Error Counter Port 0
constexpr uint16_t FWD_RX_ERR_COUNT_PORT1 =
    0x0309; ///< Forwarded RX Error Counter Port 1
constexpr uint16_t FWD_RX_ERR_COUNT_PORT2 =
    0x030A; ///< Forwarded RX Error Counter Port 2
constexpr uint16_t FWD_RX_ERR_COUNT_PORT3 =
    0x030B; ///< Forwarded RX Error Counter Port 3
constexpr uint16_t PROC_UNIT_ERR_COUNT =
    0x030C;                                ///< Processing Unit Error Counter
constexpr uint16_t PDI_ERR_COUNT = 0x030D; ///< PDI Error Counter
constexpr uint16_t LOST_LINK_COUNT_PORT0 = 0x0310; ///< Lost Link Counter Port 0
constexpr uint16_t LOST_LINK_COUNT_PORT1 = 0x0311; ///< Lost Link Counter Port 1
constexpr uint16_t LOST_LINK_COUNT_PORT2 = 0x0312; ///< Lost Link Counter Port 2
constexpr uint16_t LOST_LINK_COUNT_PORT3 = 0x0313; ///< Lost Link Counter Port 3

// EEPROM (SII) Access Registers
constexpr uint16_t EEPROM_CONFIG = 0x0500; ///< EEPROM Configuration
constexpr uint16_t EEPROM_PDI_ACCESS_STATE =
    0x0501;                                 ///< EEPROM PDI Access State
constexpr uint16_t EEPROM_CONTROL = 0x0502; ///< EEPROM Control/Status
constexpr uint16_t EEPROM_ADDRESS = 0x0504; ///< EEPROM Address
constexpr uint16_t EEPROM_DATA = 0x0508;    ///< EEPROM Data

constexpr uint16_t REG_EEPCFG = 0x0500; ///< EEPROM Configuration register
constexpr uint16_t ESTAT_R64 = 0x0040;  ///< 8-byte read support bit
} // namespace regs

/**
 * @brief EtherCAT Commands.
 */
namespace cmds {
constexpr uint8_t APRD = 0x01; ///< Auto Increment Physical Read
constexpr uint8_t APWR = 0x02; ///< Auto Increment Physical Write
constexpr uint8_t APRW = 0x03; ///< Auto Increment Physical ReadWrite
constexpr uint8_t FPRD = 0x04; ///< Configured Address Physical Read
constexpr uint8_t FPWR = 0x05; ///< Configured Address Physical Write
constexpr uint8_t FPRW = 0x06; ///< Configured Address Physical ReadWrite
constexpr uint8_t BRD = 0x07;  ///< Broadcast Read
constexpr uint8_t BWR = 0x08;  ///< Broadcast Write
constexpr uint8_t BRW = 0x09;  ///< Broadcast ReadWrite
constexpr uint8_t LRD = 0x0A;  ///< Logical Read
constexpr uint8_t LWR = 0x0B;  ///< Logical Write
constexpr uint8_t LRW = 0x0C;  ///< Logical ReadWrite
constexpr uint8_t ARMW = 0x0D; ///< Auto Increment Read Multiple Write
constexpr uint8_t FRMW = 0x0E; ///< Configured Address Read Multiple Write
} // namespace cmds

/**
 * @brief Application Layer (AL) States.
 */
namespace states {
constexpr uint16_t INIT = 0x01;    ///< Init state
constexpr uint16_t PRE_OP = 0x02;  ///< Pre-Operational state
constexpr uint16_t BOOT = 0x03;    ///< Bootstrap state
constexpr uint16_t SAFE_OP = 0x04; ///< Safe-Operational state
constexpr uint16_t OP = 0x08;      ///< Operational state
constexpr uint16_t ERROR = 0x10;   ///< Error state
constexpr uint16_t ACK = 0x10;     ///< Acknowledge bit
} // namespace states

/**
 * @brief EEPROM (SII) Control commands and constants.
 */
namespace eeprom {
constexpr uint16_t CMD_NOP = 0x0000;    ///< No operation
constexpr uint16_t CMD_READ = 0x0100;   ///< Read command
constexpr uint16_t CMD_WRITE = 0x0200;  ///< Write command
constexpr uint16_t CMD_RELOAD = 0x0300; ///< Reload command

constexpr uint16_t ERROR_MASK = 0x7800; ///< Mask for error bits
constexpr uint16_t BUSY = 0x8000;       ///< Busy bit

// SII Categories
constexpr uint16_t CAT_STRINGS = 10;      ///< Strings category
constexpr uint16_t CAT_GENERAL = 30;      ///< General category
constexpr uint16_t CAT_FMMU = 40;         ///< FMMU category
constexpr uint16_t CAT_SYNC_MANAGER = 41; ///< SyncManager category
constexpr uint16_t CAT_PDO_TX = 50;       ///< TxPDO category
constexpr uint16_t CAT_PDO_RX = 51;       ///< RxPDO category
} // namespace eeprom

/**
 * @brief Mailbox protocol definitions.
 */
namespace mailbox {
/**
 * @brief Standard Mailbox Header.
 */
struct Header {
  uint16_t length;  ///< Length of the mailbox data
  uint16_t address; ///< Station address
  uint8_t priority; ///< Priority
  uint8_t type;     ///< Type and counter (bits 0-3: type, bits 4-6: counter)
} __attribute__((packed));

/**
 * @brief Mailbox protocol types.
 */
enum Type : uint8_t {
  ERR = 0x00, ///< Error
  AOE = 0x01, ///< ADS over EtherCAT
  EOE = 0x02, ///< Ethernet over EtherCAT
  COE = 0x03, ///< CANopen over EtherCAT
  FOE = 0x04, ///< File over EtherCAT
  SOE = 0x05, ///< Servo Profile over EtherCAT
  VOE = 0x0f  ///< Vendor specific over EtherCAT
};

/**
 * @brief Helper to set the type and counter in the mailbox header.
 * @param type Mailbox protocol type.
 * @param cnt Mailbox counter [0..7].
 * @return Combined type and counter byte.
 */
static inline uint8_t set_type_cnt(Type type, uint8_t cnt) {
  return static_cast<uint8_t>(type) | ((cnt & 0x07) << 4);
}
} // namespace mailbox

/**
 * @brief CANopen over EtherCAT (CoE) definitions.
 */
namespace coe {
/**
 * @brief CoE Service types.
 */
enum Service : uint16_t {
  SDO_REQUEST = 0x02,  ///< SDO Request
  SDO_RESPONSE = 0x03, ///< SDO Response
};

/**
 * @brief SDO Command specifiers.
 */
enum SDOCommand : uint8_t {
  SDO_DOWNLOAD_EXP_REQ = 0x23, ///< Expedited download request (1-4 bytes)
  SDO_DOWNLOAD_INIT = 0x21,    ///< Normal/Segmented download initiation
  SDO_DOWNLOAD_INIT_CA =
      0x21 | 0x01,          ///< Download initiation with Complete Access
  SDO_DOWNLOAD_RESP = 0x60, ///< Download response

  SDO_UPLOAD_REQ = 0x40,           ///< Upload request
  SDO_UPLOAD_REQ_CA = 0x40 | 0x01, ///< Upload request with Complete Access
  SDO_UPLOAD_EXP_RESP = 0x43,      ///< Expedited upload response
  SDO_UPLOAD_NORM_RESP = 0x41,     ///< Normal upload response

  SDO_SEG_UP_REQ = 0x60,    ///< Segmented upload request
  SDO_SEG_DOWN_RESP = 0x20, ///< Segmented download response

  SDO_ABORT = 0x80 ///< SDO Abort
};

/**
 * @brief SDO Protocol Header.
 */
struct SDOHeader {
  uint16_t service; ///< Service and object number
  uint8_t command;  ///< SDO command
  uint16_t index;   ///< Object index
  uint8_t subindex; ///< Object sub-index
} __attribute__((packed));
} // namespace coe

/**
 * @brief File over EtherCAT (FoE) definitions.
 */
namespace foe {
/**
 * @brief FoE Opcodes.
 */
enum Opcode : uint8_t {
  RRQ = 1,  ///< Read Request
  WRQ = 2,  ///< Write Request
  DATA = 3, ///< Data
  ACK = 4,  ///< Acknowledge
  ERR = 5,  ///< Error
  BUSY = 6  ///< Busy
};

/**
 * @brief FoE Protocol Header.
 */
struct Header {
  uint8_t opcode;   ///< FoE opcode
  uint8_t reserved; ///< Reserved
  union {
    uint32_t password;   ///< Password for RRQ/WRQ
    uint32_t packet_no;  ///< Packet number for DATA/ACK
    uint32_t error_code; ///< Error code for ERR
  };
} __attribute__((packed));

// FoE Error Codes
constexpr uint32_t ERR_NOT_FOUND = 0x8001;
constexpr uint32_t ERR_ACCESS_DENIED = 0x8002;
constexpr uint32_t ERR_DISK_FULL = 0x8003;
constexpr uint32_t ERR_ILLEGAL_OP = 0x8004;
constexpr uint32_t ERR_UNKNOWN_PACKET = 0x8005;
constexpr uint32_t ERR_ILLEGAL_FILE = 0x8006;
constexpr uint32_t ERR_FILE_EXISTS = 0x8007;
constexpr uint32_t ERR_NO_USER = 0x8008;
constexpr uint32_t ERR_BOOTSTRAP_ONLY = 0x8009;
constexpr uint32_t ERR_NOT_IN_BOOTSTRAP = 0x800A;
constexpr uint32_t ERR_NO_FILE = 0x800B;
constexpr uint32_t ERR_PROGRAM_ERROR = 0x800C;
} // namespace foe

/**
 * @brief Ethernet over EtherCAT (EoE) definitions.
 */
namespace eoe {
/**
 * @brief EoE Message Types.
 */
enum Type : uint8_t {
  INIT_REQ = 0,    ///< Initiate EoE Request
  INIT_RESP = 1,   ///< Initiate EoE Response
  SET_IP_REQ = 2,  ///< Set IP Parameter Request
  SET_IP_RESP = 3, ///< Set IP Parameter Response
  GET_IP_REQ = 4,  ///< Get IP Parameter Request
  GET_IP_RESP = 5, ///< Get IP Parameter Response
  FRAME_DATA = 15  ///< Ethernet Frame Data
};

/**
 * @brief EoE Protocol Header.
 */
struct Header {
  uint16_t info1; ///< FragNo, LastFragment, TimeRequest, Type
  uint16_t info2; ///< FrameSize / Offset, TimeAppended
} __attribute__((packed));

/**
 * @brief Helper to construct info1 for EoE header.
 */
static inline uint16_t make_info1(Type type, uint8_t frag_no, bool last,
                                  bool time_req) {
  return (static_cast<uint8_t>(type) & 0x0F) | ((frag_no & 0x3F) << 8) |
         (last ? 0x4000 : 0) | (time_req ? 0x8000 : 0);
}

/**
 * @brief Helper to construct info2 for EoE header.
 */
static inline uint16_t make_info2(bool time_app,
                                  uint16_t frame_size_or_offset) {
  return (time_app ? 0x0001 : 0) | ((frame_size_or_offset & 0x0FFF) << 4);
}

} // namespace eoe

} // namespace resoem
