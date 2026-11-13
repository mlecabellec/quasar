# ObjectDictionary

## Overview
`resoes::ObjectDictionary` (`cmake-projects/resoes/include/resoes/ObjectDictionary.hpp`) provides thread-safe storage and lookup for CANopen over EtherCAT (CoE) objects (`0x1000`-`0xFFFF`), replacing SOES `SDOobjects[]` static globals and untyped `void*` data pointers.

## Key Types
- **`OdObjectType`**: `Var` (`0x0007`), `Array` (`0x0008`), `Record` (`0x0009`).
- **`OdDataType`**: Standard ETG.1000.6 scalar and string types (`Boolean`, `Integer8`..`Integer64`, `Unsigned8`..`Unsigned64`, `Real32`, `VisibleString`, `OctetString`).
- **`od_access`**: Access permission and PDO mappability bitflags (`READ_PREOP`, `READ_SAFEOP`, `READ_OP`, `WRITE_PREOP`, `WRITE_SAFEOP`, `WRITE_OP`, `RX_PDO_MAP`, `TX_PDO_MAP`, `READ_ALL`, `WRITE_ALL`, `READ_WRITE_ALL`).
- **`OdSubindexEntry`**: Stores `subindex`, `data_type`, `bit_length`, `access_flags`, `name`, and explicit little-endian `std::vector<byte>` payload storage.
