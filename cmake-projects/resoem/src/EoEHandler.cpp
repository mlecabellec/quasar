#include "resoem/EoEHandler.hpp"
#include <algorithm>
#include <cstring>

namespace resoem {

namespace {
constexpr uint32_t MAX_EOE_RX_FRAGMENTS = 10'000'000U;
constexpr int EOE_MUTEX_TIMEOUT_MS = 200;
} // namespace

/// @brief Construct EoEHandler wrapping low-level MailboxHandler.
EoEHandler::EoEHandler(MailboxHandler &mailbox) : mailbox_(mailbox) {}

/// @brief Fragment and transmit a standard Ethernet frame via EoE.
Result<> EoEHandler::send_frame(SlaveInfo &slave, std::span<const byte> frame,
                                std::chrono::microseconds timeout) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{EOE_MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(ECError::Timeout);
  }
  // [FE-0040.4.3.1] Implement Ethernet frame fragmentation.
  // Calculate maximum payload size based on the slave's mailbox capacity.
  size_t max_payload = slave.mbx_out_length - sizeof(eoe::Header);
  uint16_t total_size = static_cast<uint16_t>(frame.size());
  size_t sent = 0;
  uint8_t frag = 0;
  std::vector<byte> mbx{};
  mbx.resize(slave.mbx_out_length);

  // Tunnel the Ethernet frame through one or more EoE fragments.
  while (sent < frame.size()) {
    size_t chunk = std::min(frame.size() - sent, max_payload);
    bool last = (sent + chunk >= frame.size());

    eoe::Header *h = reinterpret_cast<eoe::Header *>(mbx.data());
    // info1 contains: Fragment number, Last fragment flag, and EoE Type
    // (FRAME_DATA).
    h->info1 = eoe::make_info1(eoe::FRAME_DATA, frag, last, false);
    // info2 contains: Total frame size (for first fragment) or Offset (for
    // subsequent ones).
    h->info2 = eoe::make_info2(
        false, (frag == 0) ? total_size : static_cast<uint16_t>(sent));

    // Copy a chunk of the Ethernet frame into the mailbox buffer.
    std::memcpy(mbx.data() + sizeof(eoe::Header), frame.data() + sent, chunk);

    if (mailbox_.write(slave, mailbox::EOE,
                       std::span{mbx.data(), sizeof(eoe::Header) + chunk},
                       timeout) <= 0)
      return std::unexpected(ECError::MailboxError);

    sent += chunk;
    frag = (frag + 1) & 0x3F; // Cyclic fragment number [0..63].
  }
  return {};
}

/// @brief Receive and reassemble an Ethernet frame via EoE with explicit completion check.
Result<std::vector<byte>>
EoEHandler::receive_frame(SlaveInfo &slave, std::chrono::microseconds timeout) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{EOE_MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(ECError::Timeout);
  }
  // [FE-0040.4.3.1] Implement Ethernet frame reassembly.
  std::vector<byte> frame;
  uint8_t exp_frag = 0;
  std::vector<byte> mbx{};
  mbx.resize(slave.mbx_in_length);
  bool completed = false;

  // Keep reading mailbox fragments until the 'last' fragment of an Ethernet
  // frame is reached.
  for (uint32_t loop_guard = 0; loop_guard < MAX_EOE_RX_FRAGMENTS; ++loop_guard) {
    size_t len;
    mailbox::Type type;
    if (mailbox_.read(slave, type, mbx, len, timeout) <= 0)
      return std::unexpected(ECError::Timeout);

    if (type != mailbox::EOE || len < sizeof(eoe::Header))
      continue; // Not an EoE message or too small.

    eoe::Header *h = reinterpret_cast<eoe::Header *>(mbx.data());
    uint8_t e_type = h->info1 & 0x0F;
    uint8_t frag = (h->info1 >> 8) & 0x3F;
    bool last = (h->info1 & 0x4000) != 0;

    // Verify that we are receiving a frame data fragment and it matches the
    // expected sequence.
    if (e_type != eoe::FRAME_DATA || frag != exp_frag)
      return std::unexpected(ECError::ProtocolError);

    // Append fragment payload to the frame being reassembled.
    frame.insert(frame.end(), mbx.begin() + sizeof(eoe::Header),
                 mbx.begin() + len);

    if (last == true) {
      completed = true;
      break; // Full Ethernet frame received.
    }

    exp_frag = (exp_frag + 1) & 0x3F;
  }
  if (completed == false) {
    return std::unexpected(ECError::Timeout);
  }
  return frame;
}

} // namespace resoem
