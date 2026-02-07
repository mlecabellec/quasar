#pragma once

#include "resoem/common.hpp"
#include <cstring>
#include <vector>

namespace resoem {

// EtherCAT Header (2 bytes)
struct EtherCATHeader {
  uint16_t length : 11;
  uint16_t reserved : 1;
  uint16_t type : 4;
};

// Raw layout of EtherCAT datagram header (10 bytes)
struct alignas(1) DatagramHeader {
  uint8_t command;
  uint8_t index;
  uint16_t address;
  uint16_t offset;
  uint16_t length : 11;
  uint16_t reserved : 3;
  uint16_t round_trip : 1;
  uint16_t last : 1;
  uint16_t interrupt;
};

// Helper for building frames
class FrameBuilder {
public:
  FrameBuilder();

  // Reset buffer to empty state
  void reset();

  // Add a datagram to the frame
  // cmd: Command (e.g., APRD, BRD)
  // idx: Index (for matching response)
  // addr: Address (ADP)
  // off: Offset (ADO)
  // data: Payload
  void add_datagram(uint8_t cmd, uint8_t idx, uint16_t addr, uint16_t off,
                    std::span<const byte> data);

  // Finalize the frame and return the buffer ready to send
  std::span<const byte> build();

  // Get the internal buffer
  const std::vector<byte> &buffer() const { return buffer_; }

private:
  std::vector<byte> buffer_;
};

} // namespace resoem
