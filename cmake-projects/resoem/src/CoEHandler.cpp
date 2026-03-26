#include "resoem/CoEHandler.hpp"
#include "resoem/Diagnostics.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>

namespace resoem {

CoEHandler::CoEHandler(MailboxHandler &mailbox) : mailbox_(mailbox) {}

Result<> CoEHandler::sdo_write(SlaveInfo &slave, uint16_t index,
                               uint8_t subindex, std::span<const byte> data,
                               bool complete_access,
                               std::chrono::microseconds timeout) {
  if (verbose_) {
    std::cout << "[CoE] SDO Write to slave 0x" << std::hex << slave.configured_address 
              << " index 0x" << index << ":" << (int)subindex 
              << " size=" << std::dec << data.size() << " B" << std::endl;
  }

  // [FE-0040.4.1.1] Support SDO Write for expedited and normal transfers.
  std::vector<byte> req_buf;
  // Standard CANopen over EtherCAT header (2 bytes).
  // Service 2 = SDO Request.
  uint16_t canopen_header = (coe::SDO_REQUEST << 12);

  // Use Expedited Transfer if data fits in 4 bytes and Complete Access is not
  // requested.
  if (data.size() <= 4 && !complete_access) {
    if (verbose_) std::cout << "  - Using Expedited Transfer" << std::endl;
    req_buf.resize(2 + sizeof(coe::SDOHeader) + 4);
    std::memcpy(req_buf.data(), &canopen_header, 2);

    coe::SDOHeader *sdo =
        reinterpret_cast<coe::SDOHeader *>(req_buf.data() + 2);
    sdo->index = index;
    sdo->subindex = subindex;
    // Command byte: expedited download request.
    // Bits 2-3 indicate how many bytes in the 4-byte payload are NOT used.
    sdo->command = coe::SDO_DOWNLOAD_EXP_REQ | ((4 - data.size()) << 2) | 0x01;

    // Copy payload (1-4 bytes).
    std::memcpy(reinterpret_cast<byte *>(sdo) + sizeof(coe::SDOHeader),
                data.data(), data.size());

    int wkc = mailbox_.write(slave, mailbox::COE, req_buf, timeout);
    if (wkc <= 0)
      return std::unexpected(ECError::MailboxError);

    // Wait for the response from the slave.
    mailbox::Type rx_type;
    std::vector<byte> resp_buf(slave.mbx_in_length);
    size_t actual_len;
    wkc = mailbox_.read(slave, rx_type, resp_buf, actual_len, timeout);

    if (wkc <= 0)
      return std::unexpected(ECError::MailboxError);
    if (rx_type != mailbox::COE || actual_len < 3)
      return std::unexpected(ECError::InvalidResponse);

    // Check if the slave aborted the transfer.
    if (resp_buf[2] == coe::SDO_ABORT) {
      uint32_t abort_code;
      std::memcpy(&abort_code, resp_buf.data() + 3, 4);
      handle_sdo_abort(slave.configured_address, index, subindex, abort_code);
      return std::unexpected(ECError::SDOAbort);
    }
    if (resp_buf[2] != coe::SDO_DOWNLOAD_RESP)
      return std::unexpected(ECError::InvalidResponse);

  } else {
    if (verbose_) std::cout << "  - Using Normal/Segmented Transfer" << std::endl;
    // Perform Normal or Segmented Download Initiation.
    req_buf.resize(2 + sizeof(coe::SDOHeader) + 4);
    std::memcpy(req_buf.data(), &canopen_header, 2);

    coe::SDOHeader *sdo =
        reinterpret_cast<coe::SDOHeader *>(req_buf.data() + 2);
    sdo->index = index;
    sdo->subindex = complete_access ? 1 : subindex;
    sdo->command = coe::SDO_DOWNLOAD_INIT;

    // Total size of the object to be downloaded.
    uint32_t total_size = static_cast<uint32_t>(data.size());
    std::memcpy(reinterpret_cast<byte *>(sdo) + sizeof(coe::SDOHeader),
                &total_size, 4);

    // In 'Normal' download, some data can be sent in the initiation packet if
    // there's room.
    size_t max_first_segment = slave.mbx_out_length - 12;
    size_t first_segment_size = std::min(data.size(), max_first_segment);

    if (first_segment_size > 0) {
      req_buf.insert(req_buf.end(), data.data(),
                     data.data() + first_segment_size);
    }

    int wkc = mailbox_.write(slave, mailbox::COE, req_buf, timeout);
    if (wkc <= 0)
      return std::unexpected(ECError::MailboxError);

    // Receive initiation response.
    mailbox::Type rx_type;
    std::vector<byte> resp_buf(slave.mbx_in_length);
    size_t actual_len;
    wkc = mailbox_.read(slave, rx_type, resp_buf, actual_len, timeout);

    if (wkc <= 0)
      return std::unexpected(ECError::MailboxError);
    if (rx_type != mailbox::COE || actual_len < 3)
      return std::unexpected(ECError::InvalidResponse);

    if (resp_buf[2] == coe::SDO_ABORT) {
      uint32_t abort_code;
      std::memcpy(&abort_code, resp_buf.data() + 3, 4);
      handle_sdo_abort(slave.configured_address, index, subindex, abort_code);
      return std::unexpected(ECError::SDOAbort);
    }

    // [FE-0040.4.1.2] Implement segmented SDO transfers for large data objects.
    // Send remaining data in segments.
    size_t sent = first_segment_size;
    uint8_t toggle = 0x00;
    int seg_count = 0;
    while (sent < data.size()) {
      // Max segment size depends on the slave's mailbox capacity.
      size_t max_seg = slave.mbx_out_length -
                       3; // 2 bytes CoE header + 1 byte Segment control
      size_t seg_size = std::min(data.size() - sent, max_seg);
      bool last_seg = (sent + seg_size == data.size());

      if (verbose_) std::cout << "    - Sending segment " << ++seg_count << " (" << seg_size << " B)" << std::endl;

      std::vector<byte> seg_buf(2 + 1 + seg_size);
      std::memcpy(seg_buf.data(), &canopen_header, 2);

      // Segment command: [Toggle bit | n bytes | last segment bit].
      uint8_t cmd = toggle;
      if (last_seg) {
        cmd |= 0x01; // Last segment
        // Bits 1-3 indicate how many bytes in the segment are NOT used.
        if (seg_size < max_seg)
          cmd |= ((max_seg - seg_size) << 1);
      }
      seg_buf[2] = cmd;
      std::memcpy(seg_buf.data() + 3, data.data() + sent, seg_size);

      wkc = mailbox_.write(slave, mailbox::COE, seg_buf, timeout);
      if (wkc <= 0)
        return std::unexpected(ECError::MailboxError);

      wkc = mailbox_.read(slave, rx_type, resp_buf, actual_len, timeout);
      if (wkc <= 0)
        return std::unexpected(ECError::MailboxError);

      if (resp_buf[2] == coe::SDO_ABORT) {
        uint32_t abort_code;
        std::memcpy(&abort_code, resp_buf.data() + 3, 4);
        handle_sdo_abort(slave.configured_address, index, subindex, abort_code);
        return std::unexpected(ECError::SDOAbort);
      }
      sent += seg_size;
      toggle ^= 0x10; // Flip toggle bit for the next segment.
    }
  }
  if (verbose_) std::cout << "  - SDO Write Success" << std::endl;
  return {};
}

Result<size_t> CoEHandler::sdo_read(SlaveInfo &slave, uint16_t index,
                                    uint8_t subindex, std::span<byte> data,
                                    bool complete_access,
                                    std::chrono::microseconds timeout) {
  if (verbose_) {
    std::cout << "[CoE] SDO Read from slave 0x" << std::hex << slave.configured_address 
              << " index 0x" << index << ":" << (int)subindex << std::dec << "..." << std::endl;
  }

  // [FE-0040.4.1.1] Support SDO Read for expedited and normal transfers.
  std::vector<byte> req_buf(sizeof(uint16_t) + sizeof(coe::SDOHeader));
  uint16_t canopen_header = (coe::SDO_REQUEST << 12);
  std::memcpy(req_buf.data(), &canopen_header, 2);

  coe::SDOHeader *sdo = reinterpret_cast<coe::SDOHeader *>(req_buf.data() + 2);
  sdo->command = complete_access ? coe::SDO_UPLOAD_REQ_CA : coe::SDO_UPLOAD_REQ;
  sdo->index = index;
  sdo->subindex = (complete_access && subindex > 1) ? 1 : subindex;

  // Send the SDO upload request.
  int wkc = mailbox_.write(slave, mailbox::COE, req_buf, timeout);
  if (wkc <= 0)
    return std::unexpected(ECError::MailboxError);

  mailbox::Type rx_type;
  std::vector<byte> resp_buf(slave.mbx_in_length);
  size_t actual_len;
  wkc = mailbox_.read(slave, rx_type, resp_buf, actual_len, timeout);

  if (wkc <= 0)
    return std::unexpected(ECError::MailboxError);
  if (rx_type != mailbox::COE || actual_len < 3)
    return std::unexpected(ECError::InvalidResponse);

  const coe::SDOHeader *resp_sdo =
      reinterpret_cast<const coe::SDOHeader *>(resp_buf.data() + 2);
  if (resp_sdo->command == coe::SDO_ABORT) {
    uint32_t abort_code;
    std::memcpy(&abort_code, resp_buf.data() + 2 + sizeof(coe::SDOHeader), 4);
    handle_sdo_abort(slave.configured_address, index, subindex, abort_code);
    return std::unexpected(ECError::SDOAbort);
  }

  size_t actual_size = 0;
  // If bit 1 of command is set, it's an expedited response (1-4 bytes).
  if (resp_sdo->command & 0x02) {
    uint8_t size_ind = (resp_sdo->command >> 2) & 0x03;
    size_t len = 4 - ((resp_sdo->command & 0x01) ? size_ind : 0);
    if (verbose_) std::cout << "  - Received Expedited response (" << len << " B)" << std::endl;
    actual_size = std::min(len, data.size());
    std::memcpy(data.data(), resp_buf.data() + 2 + sizeof(coe::SDOHeader),
                actual_size);
  } else {
    // Segmented or normal upload. The initiation response contains the total
    // size.
    uint32_t total_size;
    std::memcpy(&total_size, resp_buf.data() + 2 + sizeof(coe::SDOHeader), 4);
    if (verbose_) std::cout << "  - Starting Segmented upload (Total size: " << total_size << " B)" << std::endl;
    
    if (total_size > data.size()) {
      if (verbose_) std::cerr << "    [CoE ERROR] Buffer too small (Provided: " << data.size() << " B)" << std::endl;
      return std::unexpected(ECError::ProtocolError);
    }

    size_t received = 0;
    // Some data might already be present in the initiation response.
    size_t initial_data_size = actual_len - 10;
    if (initial_data_size > 0) {
      size_t to_copy =
          std::min(initial_data_size, static_cast<size_t>(total_size));
      if (verbose_) std::cout << "    - Initial chunk: " << to_copy << " B" << std::endl;
      std::memcpy(data.data(), resp_buf.data() + 10, to_copy);
      received = to_copy;
    }

    // [FE-0040.4.1.2] Implement segmented SDO transfers for large data objects.
    // Request segments until the 'last segment' bit is received.
    uint8_t toggle = 0x00;
    int seg_count = 0;
    while (received < total_size) {
      std::vector<byte> seg_req(3);
      uint16_t can_req = (coe::SDO_REQUEST << 12);
      std::memcpy(seg_req.data(), &can_req, 2);
      seg_req[2] = coe::SDO_SEG_UP_REQ | toggle;

      wkc = mailbox_.write(slave, mailbox::COE, seg_req, timeout);
      if (wkc <= 0)
        return std::unexpected(ECError::MailboxError);

      wkc = mailbox_.read(slave, rx_type, resp_buf, actual_len, timeout);
      if (wkc <= 0)
        return std::unexpected(ECError::MailboxError);

      if (resp_buf[2] == coe::SDO_ABORT)
        return std::unexpected(ECError::SDOAbort);

      size_t seg_size = actual_len - 3;
      bool last_seg = (resp_buf[2] & 0x01) != 0;
      if (last_seg) {
        // Calculate how many bytes in the last segment are padding.
        uint8_t bytes_not_valid = (resp_buf[2] >> 1) & 0x07;
        if (seg_size >= bytes_not_valid)
          seg_size -= bytes_not_valid;
      }
      if (verbose_) std::cout << "    - Received segment " << ++seg_count << " (" << seg_size << " B)" << (last_seg ? " [LAST]" : "") << std::endl;
      
      std::memcpy(data.data() + received, resp_buf.data() + 3, seg_size);
      received += seg_size;
      if (last_seg)
        break;
      toggle ^= 0x10;
    }
    actual_size = received;
  }
  if (verbose_) std::cout << "  - SDO Read Success (" << actual_size << " B)" << std::endl;
  return actual_size;
}

Result<std::vector<uint16_t>>
CoEHandler::read_od_list(SlaveInfo &slave, std::chrono::microseconds timeout) {
  // [FE-0040.4.1.3] Provide Object Dictionary (OD) browsing capabilities (Read OD List).
  std::vector<uint16_t> indexes;
// ...
// Actually I'll do them one by one to avoid large blocks

  std::vector<byte> req_buf(8);
  // CANopen Service 1 = SDO Information.
  uint16_t canopen_header = (0x01 << 12);
  std::memcpy(req_buf.data(), &canopen_header, 2);
  req_buf[2] = 0x01;         // SDO Information Service: Get OD List Request.
  uint16_t list_type = 0x01; // 0x01 = All objects.
  std::memcpy(req_buf.data() + 6, &list_type, 2);

  int wkc = mailbox_.write(slave, mailbox::COE, req_buf, timeout);
  if (wkc <= 0)
    return std::unexpected(ECError::MailboxError);

  bool more = true;
  while (more) {
    mailbox::Type rx_type;
    std::vector<byte> resp_buf(slave.mbx_in_length);
    size_t actual_len;
    wkc = mailbox_.read(slave, rx_type, resp_buf, actual_len, timeout);
    if (wkc <= 0)
      return std::unexpected(ECError::MailboxError);
    // 0x07 is the SDO Info Error response.
    if (resp_buf[2] == 0x07)
      return std::unexpected(ECError::ProtocolError);

    // fragments > 0 means more packets follow.
    uint16_t fragments;
    std::memcpy(&fragments, resp_buf.data() + 4, 2);
    more = (fragments > 0);

    // Each object index is 2 bytes.
    size_t n = (actual_len - 6) / 2;
    for (size_t i = 0; i < n; ++i) {
      uint16_t idx;
      std::memcpy(&idx, resp_buf.data() + 6 + i * 2, 2);
      if (idx != 0)
        indexes.push_back(idx);
    }
  }
  return indexes;
}

Result<CoEHandler::ODEntry>
CoEHandler::read_od_description(SlaveInfo &slave, uint16_t index,
                                std::chrono::microseconds timeout) {
  // [FE-0040.4.1.3] Provide Object Dictionary (OD) browsing capabilities (Read OD Description).
  std::vector<byte> req_buf(8);
  uint16_t canopen_header = (0x01 << 12);
  std::memcpy(req_buf.data(), &canopen_header, 2);
  req_buf[2] = 0x03; // SDO Information Service: Get Object Description Request.
  std::memcpy(req_buf.data() + 6, &index, 2);

  int wkc = mailbox_.write(slave, mailbox::COE, req_buf, timeout);
  if (wkc <= 0)
    return std::unexpected(ECError::MailboxError);

  mailbox::Type rx_type;
  std::vector<byte> resp_buf(slave.mbx_in_length);
  size_t actual_len;
  wkc = mailbox_.read(slave, rx_type, resp_buf, actual_len, timeout);

  // 0x04 is the Get Object Description Response.
  if (wkc <= 0 || resp_buf[2] != 0x04)
    return std::unexpected(ECError::ProtocolError);

  ODEntry entry;
  entry.index = index;
  // Parse object details: data type, max subindex, and object code.
  std::memcpy(&entry.datatype, resp_buf.data() + 8, 2);
  entry.max_subindex = resp_buf[10];
  entry.object_code = resp_buf[11];
  // The object name follows the fixed fields.
  entry.name.assign(reinterpret_cast<char *>(resp_buf.data() + 12),
                    actual_len - 12);
  return entry;
}

void CoEHandler::handle_sdo_abort(uint16_t slave_addr, uint16_t index,
                                  uint8_t subindex, uint32_t abort_code) {
  std::cerr << "SDO Abort at slave 0x" << std::hex << slave_addr << " index 0x"
            << index << ":" << (int)subindex << " code 0x" << abort_code << ": "
            << sdo_abort_to_string(abort_code) << std::dec << std::endl;
}

} // namespace resoem
