/**
 * @file SoEHandler.hpp
 * @brief Servo over EtherCAT (SoE) protocol implementation.
 */

#pragma once

#include "resoem/MailboxHandler.hpp"
#include "resoem/common.hpp"
#include <chrono>
#include <cstdint>
#include <span>
#include <vector>

namespace resoem {

/**
 * @brief SoE Operation Codes
 */
enum SoEOpCode : uint8_t {
  SOE_READREQ = 0x01,
  SOE_READRES = 0x02,
  SOE_WRITEREQ = 0x03,
  SOE_WRITERES = 0x04,
  SOE_NOTIFICATION = 0x05,
  SOE_EMERGENCY = 0x06
};

/**
 * @brief SoE Element Flags
 */
enum SoEElementFlags : uint8_t {
  SOE_DATASTATE = 0x01,
  SOE_NAME = 0x02,
  SOE_ATTRIBUTE = 0x04,
  SOE_UNIT = 0x08,
  SOE_MIN = 0x10,
  SOE_MAX = 0x20,
  SOE_VALUE = 0x40,
  SOE_DEFAULT = 0x80
};

/**
 * @brief SoE Errors
 */
enum class SoEError : uint16_t {
  NoError = 0x0000,
  InvalidIDN = 0x1001,
  InvalidData = 0x1002,
  DataTooHigh = 0x1003,
  DataTooLow = 0x1004,
  InvalidElement = 0x1005,
  NoData = 0x1006,
  InvalidOpMode = 0x1007
};

/**
 * @brief Handles SoE IDN (Identification Number) read and write operations.
 */
class SoEHandler {
public:
  /**
   * @brief Construct a new SoE Handler object.
   * @param mailbox The low-level mailbox handler to use.
   */
  explicit SoEHandler(MailboxHandler &mailbox);

  /**
   * @brief Read an IDN from a slave.
   *
   * @param slave The target slave.
   * @param drive_no Drive number (0-7).
   * @param element_flags Flags indicating which element to read (Value, Name,
   * etc.).
   * @param idn The IDN to read.
   * @param data Buffer to store the read data.
   * @param timeout Maximum time to wait.
   * @return Result<size_t> Number of bytes read or error.
   */
  Result<size_t>
  read(SlaveInfo &slave, uint8_t drive_no, uint8_t element_flags, uint16_t idn,
       std::span<byte> data,
       std::chrono::microseconds timeout = std::chrono::seconds(2));

  /**
   * @brief Write an IDN to a slave.
   *
   * @param slave The target slave.
   * @param drive_no Drive number (0-7).
   * @param element_flags Flags indicating which element to write.
   * @param idn The IDN to write.
   * @param data Data to write.
   * @param timeout Maximum time to wait.
   * @return Result<> Success or error.
   */
  Result<> write(SlaveInfo &slave, uint8_t drive_no, uint8_t element_flags,
                 uint16_t idn, std::span<const byte> data,
                 std::chrono::microseconds timeout = std::chrono::seconds(2));

private:
  MailboxHandler &mailbox_;

#pragma pack(push, 1)
  struct SoEHeader {
    uint8_t op_code : 3;
    uint8_t incomplete : 1;
    uint8_t error : 1;
    uint8_t drive_no : 3;
    uint8_t element_flags;
    union {
      uint16_t idn;
      uint16_t fragments_left;
    };
  };
#pragma pack(pop)
};

} // namespace resoem
