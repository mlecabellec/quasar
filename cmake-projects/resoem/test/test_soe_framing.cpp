#include "resoem/SoEHandler.hpp"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

// Mock MailboxHandler or test framing logic directly?
// Since MailboxHandler depends on a Socket, we might need a mock socket or
// just verify the struct layout and basic logic if we can detach it.
// For now, let's verify the struct packing which is critical.

using namespace resoem;

void test_header_packing() {
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
  } __attribute__((packed));

  // 1 byte (bitfields) + 1 byte (flags) + 2 bytes (union) = 4 bytes
  static_assert(sizeof(SoEHeader) == 4, "SoEHeader must be 4 bytes");

  SoEHeader h;
  std::memset(&h, 0, sizeof(h));
  h.op_code = 0x01; // READREQ
  h.drive_no = 2;
  h.element_flags = 0x40; // Value
  h.idn = 123;

  // Byte 0: op_code(3) | incomplete(1) | error(1) | drive_no(3)
  // Layout typically LSB->MSB:
  // bits 0-2: op_code
  // bit 3: incomplete
  // bit 4: error
  // bits 5-7: drive_no

  // Let's verify byte 0
  uint8_t b0 = *(reinterpret_cast<uint8_t *>(&h));

  // Check components
  // op_code is 0x01 (binary 001) at bits 0-2 -> 0x01
  // drive_no is 2 (binary 010) at bits 5-7 -> 010xxxxx -> 0x40 ?
  // 2 << 5 = 64 = 0x40.
  // So expected b0 = 0x40 | 0x01 = 0x41.

  std::cout << "Encoded Byte 0: 0x" << std::hex << (int)b0 << std::dec
            << std::endl;

  assert((b0 & 0x07) == 0x01); // op_code
  assert((b0 >> 5) == 0x02);   // drive_no

  std::cout << "SoE Header Packing Test Passed." << std::endl;
}

int main() {
  test_header_packing();
  return 0;
}
