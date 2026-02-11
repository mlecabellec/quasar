#include "resoem/FoEHandler.hpp"
#include <algorithm>
#include <cstring>
#include <thread>

namespace resoem {

FoEHandler::FoEHandler(MailboxHandler &mailbox) : mailbox_(mailbox) {}

Result<> FoEHandler::write_file(SlaveInfo &slave, std::string_view filename,
                               uint32_t password, std::span<const byte> data,
                               std::chrono::microseconds timeout) {
  std::vector<byte> mbx_buffer(slave.mbx_out_length);
  auto *foe_header = reinterpret_cast<foe::Header *>(mbx_buffer.data());

  foe_header->opcode = foe::WRQ;
  foe_header->reserved = 0;
  foe_header->password = password;
  size_t name_len = std::min(filename.size(), mbx_buffer.size() - sizeof(foe::Header));
  std::memcpy(mbx_buffer.data() + sizeof(foe::Header), filename.data(), name_len);
  
  int wkc = mailbox_.write(slave, mailbox::FOE, std::span{mbx_buffer.data(), sizeof(foe::Header) + name_len}, timeout);
  if (wkc <= 0) return std::unexpected(ECError::MailboxError);

  auto wait_ack = [&](uint32_t pkt) -> Result<> {
    auto start = std::chrono::steady_clock::now();
    std::vector<byte> resp_buf(slave.mbx_in_length);
    while (std::chrono::steady_clock::now() - start < timeout) {
      size_t len;
      mailbox::Type type;
      if (mailbox_.read(slave, type, resp_buf, len, timeout) > 0 && type == mailbox::FOE) {
        auto *h = reinterpret_cast<foe::Header *>(resp_buf.data());
        if (h->opcode == foe::ACK && h->packet_no == pkt) return {};
        if (h->opcode == foe::ERR) return std::unexpected(ECError::FoEError);
        if (h->opcode == foe::BUSY) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); continue; }
      }
    }
    return std::unexpected(ECError::Timeout);
  };

  if (auto res = wait_ack(0); !res) return res;

  uint32_t pkt_no = 1;
  size_t sent = 0;
  size_t max_payload = slave.mbx_out_length - sizeof(foe::Header);
  while (sent < data.size() || (sent == data.size() && data.size() > 0 && data.size() % max_payload == 0)) {
    size_t chunk = std::min(data.size() - sent, max_payload);
    foe_header->opcode = foe::DATA;
    foe_header->packet_no = pkt_no;
    std::memcpy(mbx_buffer.data() + sizeof(foe::Header), data.data() + sent, chunk);
    if (mailbox_.write(slave, mailbox::FOE, std::span{mbx_buffer.data(), sizeof(foe::Header) + chunk}, timeout) <= 0)
       return std::unexpected(ECError::MailboxError);
    if (auto res = wait_ack(pkt_no); !res) return res;
    sent += chunk; pkt_no++;
    if (chunk < max_payload) break;
  }
  return {};
}

Result<std::vector<byte>> FoEHandler::read_file(SlaveInfo &slave, std::string_view filename,
                               uint32_t password, std::chrono::microseconds timeout) {
  std::vector<byte> buffer;
  std::vector<byte> mbx_buffer(slave.mbx_out_length);
  auto *foe_header = reinterpret_cast<foe::Header *>(mbx_buffer.data());

  foe_header->opcode = foe::RRQ;
  foe_header->reserved = 0;
  foe_header->password = password;
  size_t name_len = std::min(filename.size(), mbx_buffer.size() - sizeof(foe::Header));
  std::memcpy(mbx_buffer.data() + sizeof(foe::Header), filename.data(), name_len);
  
  if (mailbox_.write(slave, mailbox::FOE, std::span{mbx_buffer.data(), sizeof(foe::Header) + name_len}, timeout) <= 0)
    return std::unexpected(ECError::MailboxError);

  uint32_t exp_pkt = 1;
  std::vector<byte> resp_buf(slave.mbx_in_length);
  size_t max_payload = slave.mbx_in_length - sizeof(foe::Header);

  while (true) {
    size_t len;
    mailbox::Type type;
    auto start = std::chrono::steady_clock::now();
    bool recv = false;
    while (std::chrono::steady_clock::now() - start < timeout) {
      if (mailbox_.read(slave, type, resp_buf, len, timeout) > 0 && type == mailbox::FOE) {
        auto *h = reinterpret_cast<foe::Header *>(resp_buf.data());
        if (h->opcode == foe::DATA && h->packet_no == exp_pkt) {
          size_t chunk = len - sizeof(foe::Header);
          buffer.insert(buffer.end(), resp_buf.begin() + sizeof(foe::Header), resp_buf.begin() + sizeof(foe::Header) + chunk);
          foe_header->opcode = foe::ACK; foe_header->packet_no = exp_pkt;
          mailbox_.write(slave, mailbox::FOE, std::span{mbx_buffer.data(), sizeof(foe::Header)}, timeout);
          if (chunk < max_payload) return buffer;
          exp_pkt++; recv = true; break;
        } else if (h->opcode == foe::ERR) return std::unexpected(ECError::FoEError);
      }
    }
    if (!recv) return std::unexpected(ECError::Timeout);
  }
}

} // namespace resoem
