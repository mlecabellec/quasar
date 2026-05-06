/**
 * @file EtherCATFrame.hpp
 * @brief Structures and classes for building and parsing EtherCAT frames.
 */

#pragma once

#include "resoem/common.hpp"
#include <cstring>
#include <vector>

namespace resoem {

/**
 * @brief Standard EtherCAT Frame Header (2 bytes).
 * 
 * This header follows the Ethernet header and precedes the datagrams.
 */
struct EtherCATHeader {
  uint16_t length : 11;   ///< Total length of all datagrams
  uint16_t reserved : 1;  ///< Reserved, must be 0
  uint16_t type : 4;      ///< Protocol type (1 for EtherCAT)
};

/**
 * @brief EtherCAT Datagram Header (10 bytes).
 * 
 * Each datagram within a frame starts with this header.
 */
struct __attribute__((packed)) DatagramHeader {
  uint8_t command;      ///< Command type (APRD, BWR, etc.)
  uint8_t index;        ///< Working counter index
  uint16_t address;     ///< Slave address (Station or Auto-increment)
  uint16_t offset;      ///< Memory offset (ADO)
  uint16_t length : 11; ///< Data length
  uint16_t reserved : 3; ///< Reserved
  uint16_t round_trip : 1; ///< Round trip flag
  uint16_t last : 1;    ///< More datagrams flag (0=last, 1=more)
  uint16_t interrupt;   ///< Interrupt register
};

/**
 * @brief Helper class to construct EtherCAT frames containing one or more datagrams.
 * @details Contributes to [FE-0040.2] EtherCAT Frame Management.
 */
class FrameBuilder {
public:
  /**
   * @brief Construct a new Frame Builder object.
   * @details [FE-0040.2.1] Provides a FrameBuilder for constructing multi-datagram frames.
   */
  FrameBuilder();

  /**
   * @brief Reset the internal buffer to start building a new frame.
   */
  void reset();

  /**
   * @brief Add a datagram with a 16-bit address and offset.
   * @details [FE-0040.2.2] Support standard EtherCAT commands (APRD, APWR, etc.).
   * 
   * @param cmd Command type (e.g., cmds::APRD).
   * @param idx Datagram index for matching responses.
   * @param addr Station or auto-increment address.
   * @param off Memory offset (ADO).
   * @param data Payload data.
   */
  void add_datagram(uint8_t cmd, uint8_t idx, uint16_t addr, uint16_t off,
                    std::span<const byte> data);

  /**
   * @brief Add a datagram with a 32-bit logical address.
   * @details [FE-0040.2.4] Support 32-bit logical addressing for process data exchange.
   * 
   * @param cmd Logical command type (e.g., cmds::LRD, cmds::LWR).
   * @param idx Datagram index.
   * @param address 32-bit logical address.
   * @param data Payload data.
   */
  void add_datagram_logical(uint8_t cmd, uint8_t idx, uint32_t address,
                            std::span<const byte> data);

  /**
   * @brief Finalize the frame, calculating headers and fixing 'More' bits.
   * @details [FE-0040.2.3] Implement automatic "More" bit handling and frame padding to minimum size.
   * 
   * @return A span pointing to the complete frame data in the internal buffer.
   */
  std::span<const byte> build();

  /**
   * @brief Get the internal buffer.
   * @return Constant reference to the raw byte buffer.
   */
  const std::vector<byte> &buffer() const { return buffer_; }

private:
  std::vector<byte> buffer_; ///< Internal storage for the frame being built
};

} // namespace resoem
