/**
 * @file EniConstants.hpp
 * @brief Symbolic constants and limitations for ETG.2100 ENI management.
 * @details Contributes to [CS-0040] Strategic Constant Management and [FE-0040] Resoem Master.
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace resoem::eni {

/** Maximum number of slaves supported in an ENI file. [CS-0040.2] */
inline constexpr std::size_t MAX_ENI_SLAVES = 4096;

/** Maximum number of InitCmd elements per slave or master. [CS-0040.2] */
inline constexpr std::size_t MAX_INIT_CMDS_PER_SLAVE = 2048;

/** Maximum number of PDO entries per PDO. [CS-0040.2] */
inline constexpr std::size_t MAX_PDO_ENTRIES_PER_PDO = 256;

/** Maximum number of cyclic frame commands in an ENI file. [CS-0040.2] */
inline constexpr std::size_t MAX_CYCLIC_CMDS = 1024;

/** Maximum number of variables in process image. [CS-0040.2] */
inline constexpr std::size_t MAX_PROCESS_IMAGE_VARIABLES = 65536;

/** Default number of retries for an InitCmd. [CS-0040.2] */
inline constexpr std::uint32_t DEFAULT_INIT_CMD_RETRIES = 3;

/** Default timeout for an InitCmd in milliseconds. [CS-0040.2] */
inline constexpr std::uint32_t DEFAULT_INIT_CMD_TIMEOUT_MS = 5000;

/** Default timeout for an ESM state transition in milliseconds. [CS-0040.2] */
inline constexpr std::uint32_t DEFAULT_TRANSITION_TIMEOUT_MS = 3000;

/** Default cycle time in microseconds. [CS-0040.2] */
inline constexpr std::uint32_t DEFAULT_CYCLE_TIME_US = 1000;

/** Maximum XML document size accepted for parsing (bytes). [CS-0040.2] */
inline constexpr std::size_t MAX_XML_FILE_SIZE_BYTES = 100 * 1024 * 1024;

/** Number of ports per ESC chip. [CS-0040.2] */
inline constexpr std::size_t MAX_ESC_PORTS = 4;

/** EtherCAT EtherType constant (0x88A4). [CS-0040.2] */
inline constexpr std::uint16_t ETHERCAT_ETHERTYPE_VAL = 0x88A4;

/**
 * @brief EtherCAT Datagram Command Codes matching ETG.1000 / ETG.2100.
 */
enum class DatagramCmd : std::uint8_t {
  NOP = 0,
  APRD = 1,
  APWR = 2,
  APRW = 3,
  FPRD = 4,
  FPWR = 5,
  FPRW = 6,
  BRD = 7,
  BWR = 8,
  BRW = 9,
  LRD = 10,
  LWR = 11,
  LRW = 12,
  ARMW = 13,
  FRMW = 14
};

/**
 * @brief ESM Transition bitflags.
 */
enum class EsmTransition : std::uint16_t {
  NONE = 0x0000,
  IP = 0x0001, ///< Init to Pre-Operational
  PS = 0x0002, ///< Pre-Operational to Safe-Operational
  PI = 0x0004, ///< Pre-Operational to Init
  SP = 0x0008, ///< Safe-Operational to Pre-Operational
  SO = 0x0010, ///< Safe-Operational to Operational
  SI = 0x0020, ///< Safe-Operational to Init
  OS = 0x0040, ///< Operational to Safe-Operational
  OP = 0x0080, ///< Operational to Pre-Operational
  OI = 0x0100, ///< Operational to Init
  IB = 0x0200, ///< Init to Bootstrap
  BI = 0x0400, ///< Bootstrap to Init
  II = 0x0800, ///< Init to Init
  PP = 0x1000, ///< Pre-Operational to Pre-Operational
  SS = 0x2000  ///< Safe-Operational to Safe-Operational
};

/**
 * @brief Bitwise OR for EsmTransition flags.
 */
[[nodiscard]] constexpr EsmTransition operator|(EsmTransition a, EsmTransition b) noexcept {
  return static_cast<EsmTransition>(static_cast<std::uint16_t>(a) | static_cast<std::uint16_t>(b));
}

/**
 * @brief Bitwise AND for EsmTransition flags.
 */
[[nodiscard]] constexpr EsmTransition operator&(EsmTransition a, EsmTransition b) noexcept {
  return static_cast<EsmTransition>(static_cast<std::uint16_t>(a) & static_cast<std::uint16_t>(b));
}

/**
 * @brief Bitwise XOR for EsmTransition flags. [CS-0020.64]
 */
[[nodiscard]] constexpr EsmTransition operator^(EsmTransition a, EsmTransition b) noexcept {
  return static_cast<EsmTransition>(static_cast<std::uint16_t>(a) ^ static_cast<std::uint16_t>(b));
}

/**
 * @brief Bitwise NOT for EsmTransition flags. [CS-0020.64]
 */
[[nodiscard]] constexpr EsmTransition operator~(EsmTransition a) noexcept {
  return static_cast<EsmTransition>(~static_cast<std::uint16_t>(a));
}

/**
 * @brief Compound bitwise OR for EsmTransition flags.
 */
constexpr EsmTransition &operator|=(EsmTransition &a, EsmTransition b) noexcept {
  a = a | b;
  return a;
}

/**
 * @brief Compound bitwise AND for EsmTransition flags.
 */
constexpr EsmTransition &operator&=(EsmTransition &a, EsmTransition b) noexcept {
  a = a & b;
  return a;
}

/**
 * @brief Compound bitwise XOR for EsmTransition flags.
 */
constexpr EsmTransition &operator^=(EsmTransition &a, EsmTransition b) noexcept {
  a = a ^ b;
  return a;
}

} // namespace resoem::eni
