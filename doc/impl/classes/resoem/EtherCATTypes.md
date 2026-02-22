# EtherCATTypes

## [IMPL-CLASSES-001] Description
This documentation covers the shared constants, enumerations, and namespace definitions found in `EtherCATTypes.hpp`. These definitions provide the semantic building blocks for the EtherCAT protocol, including register addresses, command codes, state values, and EEPROM categories.

## [IMPL-CLASSES-002] Namespaces & Constants

### Namespace `regs`
Defines standard EtherCAT Controller (ESC) register addresses.
- `TYPE` (0x0000), `REV`, `BUILD`
- `FMMUS`, `SM`
- `PORTS`, `ESC_FEATURES`
- `CONFIG_STATION_ADDR` (0x0010), `CONFIG_STATION_ALIAS`
- `DL_CONTROL`, `DL_STATUS`
- `IRQ_MASK`, `RX_ERR`
- `AL_CONTROL`, `AL_STATUS`
- `DC_SYS_TIME`, `DC_SYNC_ACT`
- `EEPROM_CONTROL`, `EEPROM_DATA`

### Namespace `cmds`
Defines EtherCAT datagram command codes.
- `APRD` (0x01): Auto Increment Physical Read
- `APWR` (0x02): Auto Increment Physical Write
- `FPRD` (0x04): Configured Address Physical Read
- `FPWR` (0x05): Configured Address Physical Write
- `BRD` (0x07): Broadcast Read
- `BWR` (0x08): Broadcast Write
- `LRD` (0x0A): Logical Read
- `LWR` (0x0B): Logical Write
- `LRW` (0x0C): Logical ReadWrite

### Namespace `states`
Defines EtherCAT Application Layer (AL) states.
- `INIT` (0x01)
- `PRE_OP` (0x02)
- `BOOT` (0x03)
- `SAFE_OP` (0x04)
- `OP` (0x08)
- `ERROR` (0x10)

### Namespace `eeprom`
Defines EEPROM (SII) control constants and categories.
- `CMD_READ` (0x0100), `CMD_WRITE`
- `BUSY` (0x8000)
- `CAT_STRINGS` (10), `CAT_GENERAL` (30), `CAT_FMMU`, `CAT_SYNC_MANAGER`

### Namespace `mailbox`
Defines Mailbox types.
- `ERR`, `AOE`, `EOE`, `COE`, `FOE`, `SOE`, `VOE`

### Namespace `coe`
Defines CANopen over EtherCAT constants.
- `SDO_DOWNLOAD_EXP_REQ` (0x23), `SDO_DOWNLOAD_INIT` (0x21), `SDO_DOWNLOAD_INIT_CA`
- `SDO_DOWNLOAD_RESP` (0x60)
- `SDO_UPLOAD_REQ` (0x40), `SDO_UPLOAD_REQ_CA`, `SDO_UPLOAD_EXP_RESP`, `SDO_UPLOAD_NORM_RESP`
- `SDO_SEG_UP_REQ` (0x60), `SDO_SEG_DOWN_RESP` (0x20)
- `SDO_ABORT` (0x80)

### Namespace `foe`
Defines File over EtherCAT opcodes and headers.
- `RRQ`, `WRQ`, `DATA`, `ACK`, `ERR`, `BUSY`

### Namespace `eoe`
Defines Ethernet over EtherCAT message types and headers.
- `INIT_REQ`, `INIT_RESP`, `SET_IP_REQ`, `FRAME_DATA`

## [IMPL-CLASSES-003] Attributes
- Constants only.

## [IMPL-CLASSES-004] Relations
- Used by `Enumerator`, `MailboxHandler`, `CoEHandler`.

## [IMPL-CLASSES-005] Dependencies
- None.

## [IMPL-CLASSES-006] Tests
- Implicitly used in all `resoem` tests.

## [IMPL-CLASSES-007] Examples
- Usage:
  ```cpp
  uint16_t cmd = cmds::APRD;
  uint16_t reg = regs::AL_STATUS;
  ```

## [IMPL-CLASSES-008] Class Diagram
*(Not applicable for namespace collection)*
