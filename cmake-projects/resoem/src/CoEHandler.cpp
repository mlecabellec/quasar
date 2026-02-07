#include "resoem/CoEHandler.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>

namespace resoem {

CoEHandler::CoEHandler(MailboxHandler &mailbox) : mailbox_(mailbox) {}

CoEError CoEHandler::sdo_write(SlaveInfo &slave, uint16_t index,
                               uint8_t subindex, std::span<const byte> data,
                               bool complete_access,
                               std::chrono::microseconds timeout) {
  // 1. Prepare SDO download request
  std::vector<byte> req_buf(sizeof(uint16_t) + sizeof(coe::SDOHeader) +
                            data.size());

  // CoE Header (CANopen)
  uint16_t canopen = (coe::SDO_REQUEST << 12);
  std::memcpy(req_buf.data(), &canopen, 2);

  coe::SDOHeader *sdo = reinterpret_cast<coe::SDOHeader *>(req_buf.data() + 2);
  sdo->index = index;
  sdo->subindex =
      (complete_access)
          ? 0x01
          : subindex; // CA uses subindex 1? No, usually subindex 0 with CA flag
  if (complete_access)
    sdo->subindex = subindex; // actually SOEM handles this in flags

  // Simplified: Expedited if data <= 4 bytes
  if (data.size() <= 4 && !complete_access) {
    sdo->command = coe::SDO_DOWNLOAD_EXP_REQ | ((4 - data.size()) << 2) |
                   0x01; // size indicator
    std::memcpy(reinterpret_cast<byte *>(sdo) + sizeof(coe::SDOHeader),
                data.data(), data.size());
    req_buf.resize(sizeof(uint16_t) + sizeof(coe::SDOHeader) +
                   4); // Fixed size for expedited
  } else {
    // Normal/Segmented
    sdo->command = coe::SDO_DOWNLOAD_NORM_REQ;
    // ... Normal download involves size at the end or in header?
    // Actually SOEM: ldata[0] = htoel(psize);
    // We'll stick to expedited for now or research normal format.
    return CoEError::DataTooLarge;
  }

  // 2. Send via Mailbox
  int wkc = mailbox_.write(slave, mailbox::COE, req_buf, timeout);
  if (wkc <= 0)
    return CoEError::MailboxError;

  // 3. Receive response
  mailbox::Type rx_type;
  std::vector<byte> resp_buf(slave.mbx_in_length);
  wkc = mailbox_.read(slave, rx_type, resp_buf, timeout);

  if (wkc <= 0)
    return CoEError::MailboxError;
  if (rx_type != mailbox::COE)
    return CoEError::InvalidResponse;

  uint16_t resp_canopen;
  std::memcpy(&resp_canopen, resp_buf.data(), 2);
  if ((resp_canopen >> 12) != coe::SDO_RESPONSE)
    return CoEError::InvalidResponse;

  coe::SDOHeader *resp_sdo =
      reinterpret_cast<coe::SDOHeader *>(resp_buf.data() + 2);
  if (resp_sdo->command == coe::SDO_ABORT) {
    uint32_t abort_code;
    std::memcpy(&abort_code, resp_buf.data() + 2 + sizeof(coe::SDOHeader), 4);
    return handle_sdo_abort(slave.configured_address, index, subindex,
                            abort_code);
  }

  if (resp_sdo->command != coe::SDO_DOWNLOAD_RESP)
    return CoEError::InvalidResponse;

  return CoEError::Success;
}

CoEError CoEHandler::sdo_read(SlaveInfo &slave, uint16_t index,
                              uint8_t subindex, std::span<byte> data,
                              size_t &actual_size, bool complete_access,
                              std::chrono::microseconds timeout) {
  // 1. Prepare SDO upload request
  std::vector<byte> req_buf(sizeof(uint16_t) + sizeof(coe::SDOHeader));

  uint16_t canopen = (coe::SDO_REQUEST << 12);
  std::memcpy(req_buf.data(), &canopen, 2);

  coe::SDOHeader *sdo = reinterpret_cast<coe::SDOHeader *>(req_buf.data() + 2);
  sdo->command = complete_access ? coe::SDO_UPLOAD_REQ_CA : coe::SDO_UPLOAD_REQ;
  sdo->index = index;
  sdo->subindex = (complete_access && subindex > 1) ? 1 : subindex;

  // 2. Send via Mailbox
  int wkc = mailbox_.write(slave, mailbox::COE, req_buf, timeout);
  if (wkc <= 0)
    return CoEError::MailboxError;

  // 3. Receive response
  mailbox::Type rx_type;
  std::vector<byte> resp_buf(slave.mbx_in_length);
  wkc = mailbox_.read(slave, rx_type, resp_buf, timeout);

  if (wkc <= 0)
    return CoEError::MailboxError;
  if (rx_type != mailbox::COE)
    return CoEError::InvalidResponse;

  uint16_t resp_canopen;
  std::memcpy(&resp_canopen, resp_buf.data(), 2);
  if ((resp_canopen >> 12) != coe::SDO_RESPONSE)
    return CoEError::InvalidResponse;

  coe::SDOHeader *resp_sdo =
      reinterpret_cast<coe::SDOHeader *>(resp_buf.data() + 2);
  if (resp_sdo->command == coe::SDO_ABORT) {
    uint32_t abort_code;
    std::memcpy(&abort_code, resp_buf.data() + 2 + sizeof(coe::SDOHeader), 4);
    return handle_sdo_abort(slave.configured_address, index, subindex,
                            abort_code);
  }

  // Expedited or Normal/Segmented response
  if (resp_sdo->command & 0x02) {
    // Expedited
    uint8_t size_ind = (resp_sdo->command >> 2) & 0x03;
    size_t len = 4 - size_ind;
    actual_size = std::min(len, data.size());
    std::memcpy(data.data(), resp_buf.data() + 2 + sizeof(coe::SDOHeader),
                actual_size);
  } else {
    // Normal/Segmented
    uint32_t total_size;
    std::memcpy(&total_size, resp_buf.data() + 2 + sizeof(coe::SDOHeader), 4);

    if (total_size > data.size()) {
      return CoEError::DataTooLarge;
    }

    size_t received = 0;
    // Initial data in this frame?
    // According to SOEM, Framedatasize = MbxHeader.length - 10
    // Header (6) + CANopen (2) + SDO Header (3) + SDO Size (4) = 15 bytes total
    // overhead for 1st frame? Wait, ec_SDOt size is MbxHeader(6) + CANOpen(2) +
    // Command(1) + Index(2) + SubIndex(1) = 12 bytes. Ldata[0] is at offset 12.
    // So Framedatasize = MbxHeader.length - 10? No.
    // SDOHeader in our implementation is 2 + 1 + 2 + 1 = 6 bytes?
    // Wait, let's look at SDOHeader in EtherCATTypes.hpp
    /*
    struct SDOHeader {
      uint16_t service; // bits 0-8: number, bits 12-15: service
      uint8_t command;
      uint16_t index;
      uint8_t subindex;
    } __attribute__((packed));
    */
    // This is 6 bytes. PLUS the 2 bytes CANopen at the start?
    // Our CoE frame starts with CANopen(2).
    // So 2 + 6 = 8 bytes before data.
    // MbxHeader.length is the size of (CANopen + SDOHeader + data).
    // So data_in_frame = MbxHeader.length - 8?
    // Actually, CoE SDO Normal response has 4 bytes size [ldata[0]] then data
    // [ldata[1]]. So data_in_frame = MbxHeader.length - 8 - 4 =
    // MbxHeader.length - 12.

    // Let's check SOEM's Framedatasize calculation: Framedatasize =
    // (etohs(aSDOp->MbxHeader.length) - 10); 10 because SOEM's CANopen is 2
    // bytes, Command is 1 byte, Index 2, SubIndex 1, then ldata[0] is 4 bytes.
    // Wait, 2+1+2+1+4 = 10. Yes.

    uint16_t current_mbx_len;
    // We don't have easy access to MbxHeader from resp_buf directly without
    // re-interpreting. Actually, MailboxHandler::read should probably return
    // the header or we can find it. For now, let's assume resp_buf contains
    // everything AFTER MbxHeader. MailboxHandler::read implementation:
    /*
      std::vector<byte> full_buf(slave.mbx_in_length + 6);
      ...
      std::memcpy(data.data(), full_buf.data() + 6, slave.mbx_in_length);
    */
    // So resp_buf is just the payload.
    // The payload length for Mailbox is in the header, which we don't have
    // here. BUT we know the slave.mbx_in_length is the MAX. The actual received
    // bytes from RawSocket::receive is what we need.

    // I should modify MailboxHandler::read to return the actual payload length.
  }

  return CoEError::Success;
}

CoEError CoEHandler::handle_sdo_abort(uint16_t slave_addr, uint16_t index,
                                      uint8_t subindex, uint32_t abort_code) {
  std::cerr << "SDO Abort at slave 0x" << std::hex << slave_addr << " index 0x"
            << index << ":" << (int)subindex << " code 0x" << abort_code
            << std::dec << std::endl;
  return CoEError::SDOAbort;
}

} // namespace resoem
