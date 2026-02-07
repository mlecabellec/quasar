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

constexpr uint16_t DC_SYNC_ACT = 0x0981;
constexpr uint16_t DC_SYS_TIME = 0x0910;
constexpr uint16_t DC_SPEED_CNT = 0x0930;
constexpr uint16_t DC_TIME_FILT = 0x0934;

constexpr uint16_t SM0 = 0x0800; // SM0
constexpr uint16_t SM1 = 0x0808; // SM1
constexpr uint8_t SM_STATUS_OFFSET = 5;

namespace sm_status {
constexpr uint8_t MBX_FULL = 0x08; // bit 3
}

constexpr uint16_t AL_CONTROL = 0x0120;
constexpr uint16_t AL_STATUS = 0x0130;
constexpr uint16_t AL_STATUS_CODE = 0x0134;

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

} // namespace resoem
