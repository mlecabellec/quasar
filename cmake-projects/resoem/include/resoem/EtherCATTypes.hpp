#pragma once

#include <cstdint>

namespace resoem {

// EtherCAT registers
namespace regs {
constexpr uint16_t TYPE = 0x0000;
constexpr uint16_t REV = 0x0001;
constexpr uint16_t BUILD = 0x0002;
constexpr uint16_t FMMUS = 0x0004;
constexpr uint16_t SM = 0x0005;
constexpr uint16_t PORTS = 0x0007;
constexpr uint16_t ESC_FEATURES = 0x0008;

constexpr uint16_t CONFIG_STATION_ALIAS = 0x0012;
constexpr uint16_t CONFIG_STATION_ADDR = 0x0010;

constexpr uint16_t DL_PORT = 0x0101;
constexpr uint16_t DL_CONTROL = 0x0100;
constexpr uint16_t DL_STATUS = 0x0110;
constexpr uint16_t DL_ALIAS = 0x0103;

constexpr uint16_t IRQ_MASK = 0x0200;
constexpr uint16_t RX_ERR = 0x0300;

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

constexpr uint16_t DC_SYNC_ACT = 0x0981;
constexpr uint16_t DC_RECEIVE_TIME_PORT0 = 0x0900;
constexpr uint16_t DC_RECEIVE_TIME_PORT1 = 0x0904;
constexpr uint16_t DC_RECEIVE_TIME_PORT2 = 0x0908;
constexpr uint16_t DC_RECEIVE_TIME_PORT3 = 0x090C;
constexpr uint16_t DC_SYS_TIME = 0x0910;
constexpr uint16_t DC_SYS_TIME_OFFSET = 0x0920;
constexpr uint16_t DC_SYS_TIME_DELAY = 0x0928;
constexpr uint16_t DC_SYS_TIME_DIFF = 0x092C;
constexpr uint16_t DC_SPEED_CNT = 0x0930;
constexpr uint16_t DC_TIME_FILT = 0x0934;

constexpr uint16_t DC_CYCLIC_UNIT_CTRL = 0x0980;
constexpr uint16_t DC_SYNC_PULSE_LEN = 0x0982;
constexpr uint16_t DC_SYNC_START_TIME = 0x0990;
constexpr uint16_t DC_SYNC0_CYCLE_TIME = 0x09A0;
constexpr uint16_t DC_SYNC1_CYCLE_TIME = 0x09A4;

constexpr uint16_t SM0 = 0x0800; // SM0
constexpr uint16_t SM1 = 0x0808; // SM1
constexpr uint16_t SM2 = 0x0810; // SM2
constexpr uint16_t SM3 = 0x0818; // SM3
constexpr uint16_t SM4 = 0x0820; // SM4
constexpr uint16_t SM5 = 0x0828; // SM5
constexpr uint16_t SM6 = 0x0830; // SM6
constexpr uint16_t SM7 = 0x0838; // SM7
constexpr uint16_t SM8 = 0x0840; // SM8
constexpr uint16_t SM9 = 0x0848; // SM9
constexpr uint16_t SM10 = 0x0850; // SM10
constexpr uint16_t SM11 = 0x0858; // SM11
constexpr uint16_t SM12 = 0x0860; // SM12
constexpr uint16_t SM13 = 0x0868; // SM13
constexpr uint16_t SM14 = 0x0870; // SM14
constexpr uint16_t SM15 = 0x0878; // SM15
constexpr uint8_t SM_STATUS_OFFSET = 5;

namespace sm_status {
constexpr uint8_t MBX_FULL = 0x08; // bit 3
}

constexpr uint16_t AL_CONTROL = 0x0120;
constexpr uint16_t AL_STATUS = 0x0130;
constexpr uint16_t AL_STATUS_CODE = 0x0134;

namespace al_status {
constexpr uint16_t STATE_MASK = 0x000F;
constexpr uint16_t ERROR_BIT = 0x0010;
} // namespace al_status

constexpr uint16_t PDI_CONTROL = 0x0140;
constexpr uint16_t PDI_CONFIG = 0x0150; // ?

// EEPROM (SII) Access
constexpr uint16_t EEPROM_CONFIG = 0x0500;
constexpr uint16_t EEPROM_PDI_ACCESS_STATE = 0x0501;
constexpr uint16_t EEPROM_CONTROL = 0x0502;
constexpr uint16_t EEPROM_ADDRESS = 0x0504;
constexpr uint16_t EEPROM_DATA = 0x0508;

// Bitmask for EEPROM_CONFIG (usually register 0x0502 lower byte or similar)
// No, SOEM uses 0x0500 for EEPCFG
constexpr uint16_t REG_EEPCFG = 0x0500;
constexpr uint16_t ESTAT_R64 = 0x0040; // 8-byte read bit
} // namespace regs

// Commands
namespace cmds {
constexpr uint8_t APRD = 0x01; // Auto Increment Physical Read
constexpr uint8_t APWR = 0x02; // Auto Increment Physical Write
constexpr uint8_t APRW = 0x03; // Auto Increment Physical ReadWrite
constexpr uint8_t FPRD = 0x04; // Configured Address Physical Read
constexpr uint8_t FPWR = 0x05; // Configured Address Physical Write
constexpr uint8_t FPRW = 0x06; // Configured Address Physical ReadWrite
constexpr uint8_t BRD = 0x07;  // Broadcast Read
constexpr uint8_t BWR = 0x08;  // Broadcast Write
constexpr uint8_t BRW = 0x09;  // Broadcast ReadWrite
constexpr uint8_t LRD = 0x0A;  // Logical Read
constexpr uint8_t LWR = 0x0B;  // Logical Write
constexpr uint8_t LRW = 0x0C;  // Logical ReadWrite
constexpr uint8_t ARMW = 0x0D; // Auto Increment Read Multiple Write
constexpr uint8_t FRMW = 0x0E; // Configured Address Read Multiple Write
} // namespace cmds

// AL States
namespace states {
constexpr uint16_t INIT = 0x01;
constexpr uint16_t PRE_OP = 0x02;
constexpr uint16_t BOOT = 0x03;
constexpr uint16_t SAFE_OP = 0x04;
constexpr uint16_t OP = 0x08;
constexpr uint16_t ERROR = 0x10;
constexpr uint16_t ACK = 0x10; // Used in control register
} // namespace states

// EEPROM Control
namespace eeprom {
constexpr uint16_t CMD_NOP = 0x0000;
constexpr uint16_t CMD_READ = 0x0100;
constexpr uint16_t CMD_WRITE = 0x0200;
constexpr uint16_t CMD_RELOAD = 0x0300; // or 0x0400?

constexpr uint16_t ERROR_MASK = 0x7800;
constexpr uint16_t BUSY = 0x8000;

// Categories
constexpr uint16_t CAT_STRINGS = 10;
constexpr uint16_t CAT_GENERAL = 30;
constexpr uint16_t CAT_FMMU = 40;
constexpr uint16_t CAT_SYNC_MANAGER = 41;
constexpr uint16_t CAT_PDO_TX = 50;
constexpr uint16_t CAT_PDO_RX = 51;
} // namespace eeprom

namespace mailbox {
struct Header {
  uint16_t length;
  uint16_t address;
  uint8_t priority;
  uint8_t type; // bits 0-3: type, bits 4-6: counter, bit 7: reserved
} __attribute__((packed));

enum Type : uint8_t {
  ERR = 0x00,
  AOE = 0x01,
  EOE = 0x02,
  COE = 0x03,
  FOE = 0x04,
  SOE = 0x05,
  VOE = 0x0f
};

static inline uint8_t set_type_cnt(Type type, uint8_t cnt) {
  return static_cast<uint8_t>(type) | ((cnt & 0x07) << 4);
}
} // namespace mailbox

namespace coe {
enum Service : uint16_t {
  SDO_REQUEST = 0x02,
  SDO_RESPONSE = 0x03,
};

enum SDOCommand : uint8_t {
  SDO_DOWNLOAD_EXP_REQ = 0x23, // 1-4 bytes
  SDO_DOWNLOAD_INIT = 0x21,    // Normal/Segmented initiation
  SDO_DOWNLOAD_INIT_CA = 0x21 | 0x01,
  SDO_DOWNLOAD_RESP = 0x60,

  SDO_UPLOAD_REQ = 0x40,
  SDO_UPLOAD_REQ_CA = 0x40 | 0x01,
  SDO_UPLOAD_EXP_RESP = 0x43,
  SDO_UPLOAD_NORM_RESP = 0x41,

  SDO_SEG_UP_REQ = 0x60,    // Segmented upload request
  SDO_SEG_DOWN_RESP = 0x20, // Response to segmented download segment

  SDO_ABORT = 0x80
};

struct SDOHeader {
  uint16_t service; // bits 0-8: number, bits 12-15: service
  uint8_t command;
  uint16_t index;
  uint8_t subindex;
} __attribute__((packed));
} // namespace coe

namespace foe {
enum Opcode : uint8_t {
  RRQ = 1,
  WRQ = 2,
  DATA = 3,
  ACK = 4,
  ERR = 5,
  BUSY = 6
};

struct Header {
  uint8_t opcode;
  uint8_t reserved;
  union {
    uint32_t password;
    uint32_t packet_no;
    uint32_t error_code;
  };
} __attribute__((packed));

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

namespace eoe {
enum Type : uint8_t {
  INIT_REQ = 0,
  INIT_RESP = 1,
  SET_IP_REQ = 2,
  SET_IP_RESP = 3,
  GET_IP_REQ = 4,
  GET_IP_RESP = 5,
  FRAME_DATA = 15
};

struct Header {
  uint16_t info1; 
  uint16_t info2;
} __attribute__((packed));

// Helpers
static inline uint16_t make_info1(Type type, uint8_t frag_no, bool last, bool time_req) {
  return (static_cast<uint8_t>(type) & 0x0F) |
         ((frag_no & 0x3F) << 8) |
         (last ? 0x4000 : 0) |
         (time_req ? 0x8000 : 0);
}

static inline uint16_t make_info2(bool time_app, uint16_t frame_size_or_offset) {
  return (time_app ? 0x0001 : 0) |
         ((frame_size_or_offset & 0x0FFF) << 4);
}

} // namespace eoe

} // namespace resoem
