#include "resoem/MailboxHandler.hpp"
#include "resoem/EtherCATFrame.hpp"
#include <algorithm>
#include <cstring>
#include <thread>

namespace resoem {

MailboxHandler::MailboxHandler(RawSocket &socket) : socket_(socket) {}

int MailboxHandler::write(SlaveInfo &slave, mailbox::Type type,
                          std::span<const byte> data,
                          std::chrono::microseconds timeout) {
  if (data.size() + sizeof(mailbox::Header) > slave.mbx_out_length) {
    return -1; // Data too large
  }

  // 1. Wait for mailbox to be empty (SM status)
  auto start = std::chrono::steady_clock::now();
  while (!is_mailbox_empty(slave)) {
    if (std::chrono::steady_clock::now() - start > timeout) {
      return 0; // Timeout
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }

  // 2. Prepare Mailbox buffer
  std::vector<byte> mbx_buffer(slave.mbx_out_length, static_cast<byte>(0));
  mailbox::Header *header =
      reinterpret_cast<mailbox::Header *>(mbx_buffer.data());
  header->length = static_cast<uint16_t>(data.size());
  header->address = 0x0000;
  header->priority = 0x00;

  // Toggle counter
  slave.mbx_cnt = (slave.mbx_cnt + 1) % 8;
  if (slave.mbx_cnt == 0)
    slave.mbx_cnt = 1;

  header->type = mailbox::set_type_cnt(type, slave.mbx_cnt);

  std::memcpy(mbx_buffer.data() + sizeof(mailbox::Header), data.data(),
              data.size());

  // 3. Write to SM (FPWR)
  return send_receive(cmds::FPWR, slave.configured_address,
                      slave.mbx_out_offset, mbx_buffer);
}

int MailboxHandler::read(SlaveInfo &slave, mailbox::Type &type,
                         std::span<byte> data, size_t &actual_len,
                         std::chrono::microseconds timeout) {
  // 1. Wait for mailbox to be full (SM status)
  auto start = std::chrono::steady_clock::now();
  while (!is_mailbox_full(slave)) {
    if (std::chrono::steady_clock::now() - start > timeout) {
      return 0; // Timeout
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }

  // 2. Read from SM (FPRD)
  std::vector<byte> mbx_buffer(slave.mbx_in_length + sizeof(mailbox::Header));
  int wkc = send_receive(cmds::FPRD, slave.configured_address,
                         slave.mbx_in_offset, mbx_buffer);

  if (wkc <= 0)
    return wkc;

  // 3. Parse header
  mailbox::Header *header =
      reinterpret_cast<mailbox::Header *>(mbx_buffer.data());
  uint16_t len = header->length;
  type = static_cast<mailbox::Type>(header->type & 0x0F);

  actual_len = std::min(static_cast<size_t>(len), data.size());
  std::memcpy(data.data(), mbx_buffer.data() + sizeof(mailbox::Header),
              actual_len);

  return wkc;
}

int MailboxHandler::send_receive(uint8_t cmd, uint16_t addr, uint16_t offset,
                                 std::span<byte> data) {
  FrameBuilder builder;
  uint8_t idx = current_idx_++;
  builder.add_datagram(cmd, idx, addr, offset, data);
  auto frame = builder.build();

  socket_.send(frame);

  std::vector<byte> rx_buffer(1500);
  size_t received = socket_.receive(rx_buffer);
  if (received == 0)
    return -1;

  size_t wkc_offset = 14 + 2 + 10 + data.size();
  if (received < wkc_offset + 2)
    return -2;

  uint16_t wkc;
  std::memcpy(&wkc, rx_buffer.data() + wkc_offset, 2);

  if (wkc > 0) {
    std::memcpy(data.data(), rx_buffer.data() + 14 + 2 + 10, data.size());
  }

  return wkc;
}

bool MailboxHandler::is_mailbox_empty(const SlaveInfo &slave) {
  // For MbxOut (usually SM0), Status bit 3 (0x08) is set if mailbox is FULL.
  // We want it EMPTY (0) to write.
  int wkc;
  // Assuming SM0 is used for MbxOut if configured
  uint8_t status = read_register_fprd<uint8_t>(
      slave.configured_address, regs::SM0 + regs::SM_STATUS_OFFSET, wkc);
  return (wkc > 0) && !(status & regs::sm_status::MBX_FULL);
}

bool MailboxHandler::is_mailbox_full(const SlaveInfo &slave) {
  // For MbxIn (usually SM1), Status bit 3 (0x08) is set if mailbox is FULL
  // (ready to read).
  int wkc;
  // Assuming SM1 is used for MbxIn if configured
  uint8_t status = read_register_fprd<uint8_t>(
      slave.configured_address, regs::SM1 + regs::SM_STATUS_OFFSET, wkc);
  return (wkc > 0) && (status & regs::sm_status::MBX_FULL);
}

template <typename T>
T MailboxHandler::read_register_fprd(uint16_t configured_addr, uint16_t reg,
                                     int &wkc) {
  T val{};
  std::span<byte> buf(reinterpret_cast<byte *>(&val), sizeof(T));
  wkc = send_receive(cmds::FPRD, configured_addr, reg, buf);
  return val;
}

} // namespace resoem
