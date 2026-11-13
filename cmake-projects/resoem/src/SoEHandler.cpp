#include "resoem/SoEHandler.hpp"
#include "resoem/EtherCATTypes.hpp"
#include <cstring>
#include <stdexcept>
#include <vector>

namespace resoem {

namespace {
constexpr uint64_t MAX_SOE_POLL_ITERATIONS = 1'000'000ULL;
constexpr int SOE_MUTEX_TIMEOUT_MS = 200;
} // namespace

/// @brief Construct SoEHandler wrapping low-level MailboxHandler.
SoEHandler::SoEHandler(MailboxHandler &mailbox) : mailbox_(mailbox) {}

/// @brief Read an SoE IDN parameter from a servo drive cleanly under mutex.
Result<size_t> SoEHandler::read(SlaveInfo &slave, uint8_t drive_no,
                                uint8_t element_flags, uint16_t idn,
                                std::span<byte> data,
                                std::chrono::microseconds timeout) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{SOE_MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(ECError::Timeout);
  }
  // Construct request
  std::vector<byte> tx_buffer{};
  tx_buffer.resize(sizeof(SoEHeader));
  SoEHeader *req = reinterpret_cast<SoEHeader *>(tx_buffer.data());
  req->op_code = SOE_READREQ;
  req->incomplete = 0;
  req->error = 0;
  req->drive_no = drive_no;
  req->element_flags = element_flags;
  req->set_idn(idn);

  // Send request using MailboxHandler::write
  if (mailbox_.write(slave, mailbox::Type::SOE, tx_buffer, timeout) <= 0) {
    return std::unexpected(ECError::MailboxError);
  }

  // Receive response using MailboxHandler::read
  std::vector<byte> rx_buffer{};
  rx_buffer.resize(slave.mbx_in_length);
  mailbox::Type recycled_type;
  size_t actual_len = 0;

  // Polling loop for response
  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();
  for (uint64_t i = 0; i < MAX_SOE_POLL_ITERATIONS; ++i) {
    if (std::chrono::steady_clock::now() - start >= timeout)
      break;
    int wkc =
        mailbox_.read(slave, recycled_type, rx_buffer, actual_len, timeout);
    if (wkc <= 0)
      continue;

    // Check if it's an SoE frame
    if (recycled_type != mailbox::Type::SOE)
      continue;

    if (actual_len < sizeof(SoEHeader))
      continue;

    SoEHeader *resp = reinterpret_cast<SoEHeader *>(rx_buffer.data());
    // Mailbox handler might have stripped the mailbox header, but here we
    // process the SoE payload
    if (resp->drive_no != drive_no || resp->element_flags != element_flags)
      continue;

    if (resp->op_code == SOE_READRES) {
      if (resp->error != 0U) {
        return std::unexpected(ECError::ProtocolError);
      }
      size_t payload_size = actual_len - sizeof(SoEHeader);
      if (payload_size > data.size()) {
        // Truncate or error? For now truncate
        payload_size = data.size();
      }
      std::memcpy(data.data(), rx_buffer.data() + sizeof(SoEHeader),
                  payload_size);
      return payload_size;
    }
    if (i == (MAX_SOE_POLL_ITERATIONS - 1U))
      throw std::runtime_error("Hard limit exceeded in SoEHandler::read");
  }

  return std::unexpected(ECError::Timeout);
}

/// @brief Write an SoE IDN parameter to a servo drive cleanly under mutex.
Result<> SoEHandler::write(SlaveInfo &slave, uint8_t drive_no,
                           uint8_t element_flags, uint16_t idn,
                           std::span<const byte> data,
                           std::chrono::microseconds timeout) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{SOE_MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(ECError::Timeout);
  }
  // Construct request
  std::vector<byte> tx_buffer{};
  tx_buffer.resize(sizeof(SoEHeader) + data.size());
  SoEHeader *req = reinterpret_cast<SoEHeader *>(tx_buffer.data());
  req->op_code = SOE_WRITEREQ;
  req->incomplete = 0; // Simplified: no fragmentation support yet
  req->error = 0;
  req->drive_no = drive_no;
  req->element_flags = element_flags;
  req->set_idn(idn);

  std::memcpy(tx_buffer.data() + sizeof(SoEHeader), data.data(), data.size());

  // Send request
  if (mailbox_.write(slave, mailbox::Type::SOE, tx_buffer, timeout) <= 0) {
    return std::unexpected(ECError::MailboxError);
  }

  // Receive response
  std::vector<byte> rx_buffer{};
  rx_buffer.resize(slave.mbx_in_length);
  mailbox::Type recycled_type;
  size_t actual_len = 0;

  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();
  for (uint64_t i = 0; i < MAX_SOE_POLL_ITERATIONS; ++i) {
    if (std::chrono::steady_clock::now() - start >= timeout)
      break;
    int wkc =
        mailbox_.read(slave, recycled_type, rx_buffer, actual_len, timeout);
    if (wkc <= 0)
      continue;

    if (recycled_type != mailbox::Type::SOE)
      continue;
    if (actual_len < sizeof(SoEHeader))
      continue;

    SoEHeader *resp = reinterpret_cast<SoEHeader *>(rx_buffer.data());
    if (resp->drive_no != drive_no || resp->element_flags != element_flags)
      continue;

    if (resp->op_code == SOE_WRITERES) {
      if (resp->error != 0U)
        return std::unexpected(ECError::ProtocolError);
      return {};
    }
    if (i == (MAX_SOE_POLL_ITERATIONS - 1U))
      throw std::runtime_error("Hard limit exceeded in SoEHandler::write");
  }

  return std::unexpected(ECError::Timeout);
}

} // namespace resoem
