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
  if (verbose_) {
    std::cout << "[MBX] Writing " << data.size() << " bytes to slave 0x" 
              << std::hex << slave.configured_address << " type=" << (int)type << std::dec << "..." << std::endl;
  }

  // Check if data fits into the slave's configured output mailbox.
  if (data.size() + sizeof(mailbox::Header) > slave.mbx_out_length) {
    if (verbose_) std::cerr << "  [MBX ERROR] Data too large for mailbox out (" << slave.mbx_out_length << " B)" << std::endl;
    return -1; // Data too large
  }

  // 1. Wait for the output mailbox to be empty.
  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();
  for (uint64_t i = 0; i < 1000000; ++i) {
    if (is_mailbox_empty(slave))
      break;
    if (std::chrono::steady_clock::now() - start > timeout) {
      if (verbose_) std::cerr << "  [MBX ERROR] Timeout waiting for mailbox out to be empty." << std::endl;
      return 0; // Timeout waiting for slave to clear mailbox
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    if (i == 999999)
      throw std::runtime_error("Hard limit exceeded in MailboxHandler::write");
  }

  // 2. Prepare the Mailbox datagram.
  std::vector<byte> mbx_buffer(slave.mbx_out_length, static_cast<byte>(0));
  mailbox::Header *header =
      reinterpret_cast<mailbox::Header *>(mbx_buffer.data());
  header->length = static_cast<uint16_t>(data.size());
  header->address = 0x0000;
  header->priority = 0x00;

  slave.mbx_cnt = (slave.mbx_cnt + 1) % 8;
  if (slave.mbx_cnt == 0)
    slave.mbx_cnt = 1;

  header->type = mailbox::set_type_cnt(type, slave.mbx_cnt);

  std::memcpy(mbx_buffer.data() + sizeof(mailbox::Header), data.data(),
              data.size());

  // 3. Write the entire buffer to the SyncManager physical address.
  int wkc = send_receive(cmds::FPWR, slave.configured_address,
                      slave.mbx_out_offset, mbx_buffer);
  if (verbose_ && wkc <= 0) std::cerr << "  [MBX ERROR] Write failed (WKC=" << wkc << ")" << std::endl;
  return wkc;
}

int MailboxHandler::read(SlaveInfo &slave, mailbox::Type &type,
                         std::span<byte> data, size_t &actual_len,
                         std::chrono::microseconds timeout) {
  if (verbose_) {
    std::cout << "[MBX] Reading from slave 0x" << std::hex << slave.configured_address << std::dec << "..." << std::endl;
  }

  // 1. Wait for the input mailbox to be full (meaning the slave has placed a
  // response).
  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();
  for (uint64_t i = 0; i < 1000000; ++i) {
    if (is_mailbox_full(slave))
      break;
    if (std::chrono::steady_clock::now() - start > timeout) {
      if (verbose_) std::cerr << "  [MBX ERROR] Timeout waiting for response." << std::endl;
      return 0; // Timeout waiting for response
    }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    if (i == 999999)
      throw std::runtime_error("Hard limit exceeded in MailboxHandler::read");
  }

  // 2. Read the response from the SyncManager input buffer.
  std::vector<byte> mbx_buffer(slave.mbx_in_length + sizeof(mailbox::Header));
  int wkc = send_receive(cmds::FPRD, slave.configured_address,
                         slave.mbx_in_offset, mbx_buffer);

  if (wkc <= 0) {
    if (verbose_) std::cerr << "  [MBX ERROR] Read failed (WKC=" << wkc << ")" << std::endl;
    return wkc;
  }

  // 3. Parse the mailbox header to determine protocol and payload length.
  mailbox::Header *header =
      reinterpret_cast<mailbox::Header *>(mbx_buffer.data());
  uint16_t len = header->length;
  type = static_cast<mailbox::Type>(header->type & 0x0F);

  if (verbose_) {
      std::cout << "  [MBX] Received " << len << " bytes, type=" << (int)type << ", cnt=" << (int)(header->type >> 4) << std::endl;
  }

  // Clamp the copied length to the user-provided buffer size.
  actual_len = std::min(static_cast<size_t>(len), data.size());
  std::memcpy(data.data(), mbx_buffer.data() + sizeof(mailbox::Header),
              actual_len);

  return wkc;
}

int MailboxHandler::send_receive(uint8_t cmd, uint16_t addr, uint16_t offset,
                                 std::span<byte> data) {
  // Simple retry loop to handle intermittent network issues.
  for (int attempt = 0; attempt <= retries_; ++attempt) {
    FrameBuilder builder;
    uint8_t idx = current_idx_++;
    builder.add_datagram(cmd, idx, addr, offset, data);
    std::span<const byte> frame = builder.build();

    socket_.send(frame);

    std::vector<byte> rx_buffer(1500);
    size_t received = socket_.receive(rx_buffer);
    if (received == 0) {
      // If no response, wait a bit and retry.
      if (attempt < retries_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        continue;
      }
      return -1; // Timeout after all retries
    }

    size_t wkc_offset = 14 + 2 + 10 + data.size();
    if (received < wkc_offset + 2)
      continue; // Malformed response

    uint16_t wkc;
    std::memcpy(&wkc, rx_buffer.data() + wkc_offset, 2);

    if (wkc > 0) {
      // Success: copy received data back to caller.
      std::memcpy(data.data(), rx_buffer.data() + 14 + 2 + 10, data.size());
      return wkc;
    }

    // If WKC is 0, the slave might be busy. Wait and retry.
    if (attempt < retries_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  return 0; // Failed (Working Counter was 0)
}

bool MailboxHandler::is_mailbox_empty(const SlaveInfo &slave) {
  // In EtherCAT, the Master can only write to a mailbox if it's 'empty'.
  // The 'Full' bit (bit 3 of SM status register) indicates if the SLAVE has
  // data for the Master. For Output SyncManagers, it means the slave hasn't
  // read the previous message yet.
  int wkc;
  uint8_t status = read_register_fprd<uint8_t>(
      slave.configured_address, regs::SM0 + regs::SM_STATUS_OFFSET, wkc);
  return (wkc > 0) && !(status & regs::sm_status::MBX_FULL);
}

bool MailboxHandler::is_mailbox_full(const SlaveInfo &slave) {
  // For Input SyncManagers, the 'Full' bit indicates that the slave has placed
  // a message in the mailbox for the Master to read.
  int wkc;
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
