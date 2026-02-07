#include "resoem/EtherCATFrame.hpp"
#include <algorithm>

namespace resoem {

FrameBuilder::FrameBuilder() {
  // Reserve space for Ethernet header (14) + EtherCAT header (2) + some payload
  buffer_.reserve(1500);
  reset();
}

void FrameBuilder::reset() {
  buffer_.clear();
  // 14 bytes Ethernet header placeholder
  buffer_.resize(ETHERNET_HEADER_SIZE);

  // 2 bytes EtherCAT header placeholder
  buffer_.resize(ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE);
}

void FrameBuilder::add_datagram(uint8_t cmd, uint8_t idx, uint16_t addr,
                                uint16_t off, std::span<const byte> data) {
  size_t current_size = buffer_.size();
  size_t data_len = data.size();

  // Datagram header (10 bytes) + Data + WKC (2 bytes)
  buffer_.resize(current_size + 10 + data_len + 2);

  byte *ptr = buffer_.data() + current_size;

  // Command
  ptr[0] = cmd;
  // Index
  ptr[1] = idx;
  // Address (ADP)
  std::memcpy(ptr + 2, &addr, 2);
  // Offset (ADO)
  std::memcpy(ptr + 4, &off, 2);

  // Length (11 bits) + R + C + R + M
  // Length is strictly data length
  uint16_t len_field = static_cast<uint16_t>(data_len & 0x7FF);
  // Determine if there is a 'Next' datagram.
  // We update the PREVIOUS datagram's header 'M' bit if this is not the first
  // one. BUT here we are just appending. We assume 'M' (More) bit is 0 for now.
  // If we add another one later, we'd need to set the M bit of this one.

  // Wait, the logic is: if there are more datagrams following, set M=1.
  // Since we are building sequentially, we don't know if more will come.
  // Actually, we can just set the M bit of the *previous* datagram if
  // buffer_.size() > (ETH+ECAT).

  if (current_size > (ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE)) {
    // Find the previous datagram's length field.
    // It's at [prev_start + 6]
    // This is tricky without tracking offsets.
    // A simple FrameBuilder usually iterates or we just assume the user builds
    // in order. For simplicity: We will set the 'More' bit of the previous
    // datagram if there is one.

    // However, standard says: M bit is in the Length field.
    // Let's implement a smarter way: The build() method will fix up the
    // length/more bits if we tracked them. Or simpler: The user adds datagrams.
    // When adding a NEW one, we go back to the previous one and set its 'More'
    // bit.
  }

  std::memcpy(ptr + 6, &len_field, 2);

  // Interrupt (2 bytes)
  std::memset(ptr + 8, 0, 2);

  // Data
  if (!data.empty()) {
    std::memcpy(ptr + 10, data.data(), data_len);
  }

  // Working Counter (WKC) - initialized to 0
  std::memset(ptr + 10 + data_len, 0, 2);
}

std::span<const byte> FrameBuilder::build() {
  // 1. Fill Ethernet Header (Broadcast dest, src, EtherType)
  // Destination: Broadcast (FF:FF:FF:FF:FF:FF)
  std::memset(buffer_.data(), 0xFF, 6);

  // Source: We will leave it 0, the kernel RAW socket may fill it or we can
  // fill it if we possess the MAC. For now, let's leave it 00. IMPORTANT: Raw
  // socket on Linux might overwrite it or we should get it from socket.

  // EtherType (0x88A4) - Little Endian or Big Endian? Network byte order (Big
  // Endian).
  buffer_[12] = 0x88;
  buffer_[13] = 0xA4;

  // 2. Fill EtherCAT Header (Length | Reserved | Type)
  // Length = Total size of datagrams (header + data + WKC)
  uint16_t total_len = static_cast<uint16_t>(
      buffer_.size() - ETHERNET_HEADER_SIZE - ETHERCAT_HEADER_SIZE);

  uint16_t ecat_header = total_len & 0x7FF; // Length mask
  ecat_header |= (1 << 12);                 // Type = 1 (EtherCAT frames)

  std::memcpy(buffer_.data() + 14, &ecat_header, 2);

  // 3. Fix up 'More' bits.
  // Iterate through datagrams to set the 'M' bit if necessary.
  size_t offset = ETHERNET_HEADER_SIZE + ETHERCAT_HEADER_SIZE;
  while (offset < buffer_.size()) {
    // Datagram header starts at 'offset'
    // Length field at offset + 6
    uint16_t len_field;
    std::memcpy(&len_field, buffer_.data() + offset + 6, 2);

    uint16_t data_len = len_field & 0x7FF;
    size_t wkc_offset = offset + 10 + data_len;
    size_t next_datagram_offset = wkc_offset + 2;

    // Check if there is another datagram
    if (next_datagram_offset < buffer_.size()) {
      len_field |= (1 << 15); // Set 'More' bit
      std::memcpy(buffer_.data() + offset + 6, &len_field, 2);
    }

    offset = next_datagram_offset;
  }

  // 4. Pad to minimum Ethernet frame size (64 bytes including FCS, so 60 bytes
  // payload)
  if (buffer_.size() < 60) {
    buffer_.resize(60, 0);
  }

  return std::span<const byte>(buffer_);
}

} // namespace resoem
