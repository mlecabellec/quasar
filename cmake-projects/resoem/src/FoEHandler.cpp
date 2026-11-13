#include "resoem/FoEHandler.hpp"
#include <algorithm>
#include <cstring>
#include <functional>
#include <thread>

namespace resoem {

namespace {
constexpr uint32_t MAX_FOE_WAIT_ITERATIONS = 10'000'000U;
constexpr int FOE_MUTEX_TIMEOUT_MS = 200;
} // namespace

/// @brief Construct FoEHandler wrapping low-level MailboxHandler.
FoEHandler::FoEHandler(MailboxHandler &mailbox) : mailbox_(mailbox) {}

/// @brief Write a file to a slave via File over EtherCAT (FoE).
Result<> FoEHandler::write_file(SlaveInfo &slave, std::string_view filename,
                                uint32_t password, std::span<const byte> data,
                                std::chrono::microseconds timeout) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{FOE_MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(ECError::Timeout);
  }
  // [FE-0040.4.2.1] Implement file download state machine.
  std::vector<byte> mbx_buffer{};
  mbx_buffer.resize(slave.mbx_out_length);
  foe::Header *foe_header = reinterpret_cast<foe::Header *>(mbx_buffer.data());

  // 1. Send Write Request (WRQ).
  foe_header->opcode = foe::WRQ;
  foe_header->reserved = 0;
  foe_header->set_password(password);

  // The filename is placed immediately after the 6-byte FoE header.
  size_t name_len =
      std::min(filename.size(), mbx_buffer.size() - sizeof(foe::Header));
  std::memcpy(mbx_buffer.data() + sizeof(foe::Header), filename.data(),
              name_len);

  int wkc = mailbox_.write(
      slave, mailbox::FOE,
      std::span{mbx_buffer.data(), sizeof(foe::Header) + name_len}, timeout);
  if (wkc <= 0)
    return std::unexpected(ECError::MailboxError);

  // Helper to wait for an ACK for a specific packet number.
  std::function<Result<>(uint32_t)> wait_ack = [&](uint32_t pkt) -> Result<> {
    std::chrono::steady_clock::time_point start =
        std::chrono::steady_clock::now();
    std::vector<byte> resp_buf{};
    resp_buf.resize(slave.mbx_in_length);
    for (uint32_t iter = 0; iter < MAX_FOE_WAIT_ITERATIONS; ++iter) {
      if ((std::chrono::steady_clock::now() - start) >= timeout) {
        return std::unexpected(ECError::Timeout);
      }
      size_t len;
      mailbox::Type type;
      if (mailbox_.read(slave, type, resp_buf, len, timeout) > 0 &&
          type == mailbox::FOE) {
        foe::Header *h = reinterpret_cast<foe::Header *>(resp_buf.data());
        if (h->opcode == foe::ACK && h->packet_no() == pkt)
          return {};
        if (h->opcode == foe::ERR)
          return std::unexpected(ECError::FoEError);
        // Handle BUSY response with bounded backoff wait.
        if (h->opcode == foe::BUSY) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
          continue;
        }
      }
    }
    return std::unexpected(ECError::Timeout);
  };

  // Wait for ACK of the WRQ (packet 0).
  if (Result<> res = wait_ack(0); res.has_value() == false)
    return res;

  // 2. Send data packets.
  uint32_t pkt_no = 1;
  size_t sent = 0;
  size_t max_payload = slave.mbx_out_length - sizeof(foe::Header);

  // Continue sending while there is data, or if the last packet was exactly
  // max_payload (a zero-length packet is needed to terminate if the total size
  // is a multiple of max_payload).
  for (uint32_t pkt_guard = 0; pkt_guard < MAX_FOE_WAIT_ITERATIONS; ++pkt_guard) {
    if ((sent < data.size() || (sent == data.size() && data.size() > 0 &&
                                data.size() % max_payload == 0)) == false) {
      break;
    }
    size_t chunk = std::min(data.size() - sent, max_payload);
    foe_header->opcode = foe::DATA;
    foe_header->set_packet_no(pkt_no);

    std::memcpy(mbx_buffer.data() + sizeof(foe::Header), data.data() + sent,
                chunk);

    if (mailbox_.write(
            slave, mailbox::FOE,
            std::span{mbx_buffer.data(), sizeof(foe::Header) + chunk},
            timeout) <= 0)
      return std::unexpected(ECError::MailboxError);

    // Each DATA packet must be acknowledged by the slave.
    if (Result<> res = wait_ack(pkt_no); res.has_value() == false)
      return res;

    sent += chunk;
    pkt_no++;

    // If we sent a packet smaller than max_payload, the transfer is complete.
    if (chunk < max_payload)
      break;
  }
  return {};
}

/// @brief Read a file from a slave via File over EtherCAT (FoE).
Result<std::vector<byte>>
FoEHandler::read_file(SlaveInfo &slave, std::string_view filename,
                      uint32_t password, std::chrono::microseconds timeout) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{FOE_MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(ECError::Timeout);
  }
  // [FE-0040.4.2.1] Implement file upload state machine.
  std::vector<byte> buffer;
  std::vector<byte> mbx_buffer{};
  mbx_buffer.resize(slave.mbx_out_length);
  foe::Header *foe_header = reinterpret_cast<foe::Header *>(mbx_buffer.data());

  // 1. Send Read Request (RRQ).
  foe_header->opcode = foe::RRQ;
  foe_header->reserved = 0;
  foe_header->set_password(password);
  size_t name_len =
      std::min(filename.size(), mbx_buffer.size() - sizeof(foe::Header));
  std::memcpy(mbx_buffer.data() + sizeof(foe::Header), filename.data(),
              name_len);

  if (mailbox_.write(
          slave, mailbox::FOE,
          std::span{mbx_buffer.data(), sizeof(foe::Header) + name_len},
          timeout) <= 0)
    return std::unexpected(ECError::MailboxError);

  uint32_t exp_pkt = 1;
  std::vector<byte> resp_buf{};
  resp_buf.resize(slave.mbx_in_length);
  size_t max_payload = slave.mbx_in_length - sizeof(foe::Header);

  // 2. Receive data packets.
  for (uint32_t loop_guard = 0; loop_guard < MAX_FOE_WAIT_ITERATIONS; ++loop_guard) {
    size_t len;
    mailbox::Type type;
    std::chrono::steady_clock::time_point start =
        std::chrono::steady_clock::now();
    bool recv = false;
    for (uint32_t poll_iter = 0; poll_iter < MAX_FOE_WAIT_ITERATIONS; ++poll_iter) {
      if ((std::chrono::steady_clock::now() - start) >= timeout) {
        break;
      }
      if (mailbox_.read(slave, type, resp_buf, len, timeout) > 0 &&
          type == mailbox::FOE) {
        foe::Header *h = reinterpret_cast<foe::Header *>(resp_buf.data());
        if (h->opcode == foe::DATA && h->packet_no() == exp_pkt) {
          // Found the expected data packet.
          size_t chunk = len - sizeof(foe::Header);
          buffer.insert(buffer.end(), resp_buf.begin() + sizeof(foe::Header),
                        resp_buf.begin() + sizeof(foe::Header) + chunk);

          // Send ACK for the received packet.
          foe_header->opcode = foe::ACK;
          foe_header->set_packet_no(exp_pkt);
          mailbox_.write(slave, mailbox::FOE,
                         std::span{mbx_buffer.data(), sizeof(foe::Header)},
                         timeout);

          // If chunk < max_payload, it's the last packet.
          if (chunk < max_payload)
            return buffer;

          exp_pkt++;
          recv = true;
          break;
        } else if (h->opcode == foe::ERR) {
          return std::unexpected(ECError::FoEError);
        }
      }
    }
    if (recv == false)
      return std::unexpected(ECError::Timeout);
  }
  return std::unexpected(ECError::Timeout);
}

} // namespace resoem
