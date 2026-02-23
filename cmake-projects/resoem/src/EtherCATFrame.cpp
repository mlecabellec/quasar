#include "resoem/EtherCATFrame.hpp"
#include <algorithm>

namespace resoem {

FrameBuilder::FrameBuilder() {
  // Pre-allocate buffer to avoid frequent reallocations. 
  // 1500 bytes is the standard Ethernet MTU.
  buffer_.reserve(1500);
  reset();
}

void FrameBuilder::reset() {
  buffer_.clear();
  
  // Reserve space for the 14-byte Ethernet header. 
  // We'll fill this in the build() method.
  buffer_.resize(ETHERNET_HEADER_SIZE);

  // Reserve space for the 2-byte EtherCAT header.
  // This header describes the total length of all datagrams.
  buffer_.resize(ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE);
}

void FrameBuilder::add_datagram(uint8_t cmd, uint8_t idx, uint16_t addr,
                                uint16_t off, std::span<const byte> data) {
  // [FE-0040.2.2] Support standard EtherCAT commands (APRD, APWR, etc.).
  size_t current_size = buffer_.size();
  size_t data_len = data.size();

  // Resize buffer to accommodate:
  // - Datagram header (10 bytes)
  // - Payload data (data_len bytes)
  // - Working Counter (WKC) (2 bytes)
  buffer_.resize(current_size + 10 + data_len + 2);

  byte *ptr = buffer_.data() + current_size;

  // Set command and datagram index.
  ptr[0] = cmd;
  ptr[1] = idx;
  
  // Write the slave address and memory offset (ADO).
  std::memcpy(ptr + 2, &addr, 2);
  std::memcpy(ptr + 4, &off, 2);

  // The length field in the datagram header is 11 bits.
  // The remaining 5 bits are used for Reserved (3), RoundTrip (1), and More (1).
  uint16_t len_field = static_cast<uint16_t>(data_len & 0x7FF);
  std::memcpy(ptr + 6, &len_field, 2);

  // Initialize interrupt register field to zero.
  std::memset(ptr + 8, 0, 2);

  // Copy the payload data if any.
  if (!data.empty()) {
    std::memcpy(ptr + 10, data.data(), data_len);
  }

  // Initialize the Working Counter (WKC) to 0. 
  // The slaves will increment this as they process the datagram.
  std::memset(ptr + 10 + data_len, 0, 2);
}

void FrameBuilder::add_datagram_logical(uint8_t cmd, uint8_t idx,
                                        uint32_t address,
                                        std::span<const byte> data) {
  // [FE-0040.2.4] Support 32-bit logical addressing for process data exchange.
  // For logical addressing, the 32-bit address is split across 
  // the 16-bit address and 16-bit offset fields.
  add_datagram(cmd, idx, static_cast<uint16_t>(address & 0xFFFF),
               static_cast<uint16_t>(address >> 16), data);
}

std::span<const byte> FrameBuilder::build() {
  // 1. Fill Ethernet Header
  // Set Destination MAC to Broadcast (FF:FF:FF:FF:FF:FF).
  std::memset(buffer_.data(), 0xFF, 6);

  // Source MAC is left as 00:00:00:00:00:00. 
  // Most Linux raw socket implementations will fill this automatically.
  
  // Set EtherType to 0x88A4 (EtherCAT). 
  // We write it in network byte order (Big Endian).
  buffer_[12] = 0x88;
  buffer_[13] = 0xA4;

  // 2. Fill EtherCAT Header
  // Calculate total length of all appended datagrams.
  uint16_t total_len = static_cast<uint16_t>(
      buffer_.size() - ETHERNET_HEADER_SIZE - ETHERCAT_HEADER_SIZE);

  // Type 1 indicates an EtherCAT frame (not to be confused with EtherType).
  uint16_t ecat_header = total_len & 0x7FF; 
  ecat_header |= (1 << 12);                 

  std::memcpy(buffer_.data() + 14, &ecat_header, 2);

  // 3. Fix up 'More' bits in datagram headers.
  // [FE-0040.2.3] Implement automatic "More" bit handling.
  // Each datagram (except the last) must have the 'More' bit set 
  // to notify the slaves that another datagram follows in the same frame.
  size_t offset = ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE;
  while (offset < buffer_.size()) {
    uint16_t len_field;
    std::memcpy(&len_field, buffer_.data() + offset + 6, 2);

    uint16_t data_len = len_field & 0x7FF;
    size_t wkc_offset = offset + 10 + data_len;
    size_t next_datagram_offset = wkc_offset + 2;

    // If there is data remaining in the buffer, set the 'M' bit of the current datagram.
    if (next_datagram_offset < buffer_.size()) {
      len_field |= (1 << 15); 
      std::memcpy(buffer_.data() + offset + 6, &len_field, 2);
    }

    offset = next_datagram_offset;
  }

  // 4. Pad the frame to minimum Ethernet size.
  // [FE-0040.2.3] Implement frame padding to minimum Ethernet size (64 bytes).
  // Standard Ethernet frames must be at least 64 bytes (including 4-byte CRC).
  // Thus, the payload (everything from Dest MAC to end of datagrams) must be 60 bytes.
  if (buffer_.size() < 60) {
    buffer_.resize(60, 0);
  }

  return std::span<const byte>(buffer_);
}

} // namespace resoem
