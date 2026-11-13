/**
 * @file CoeServer.cpp
 * @brief CoE SDO Expedited/Segmented/Complete-Access, SDO Info & Bit-Sliced PDO engine.
 * @feature [TSK-20261113-001]
 */

#include "resoes/CoeServer.hpp"
#include <algorithm>
#include <array>
#include <chrono>

namespace resoes {

namespace {
/// @brief Pack up to `bit_len` bits from `src_bytes` into `dst` at `dst_bit_offset`.
void copy_bits_to_buffer(std::span<byte> dst, uint32_t dst_bit_offset,
                         std::span<const byte> src_bytes,
                         uint16_t bit_len) noexcept {
  for (uint16_t b = 0U; b < bit_len; ++b) {
    size_t src_byte_idx = static_cast<size_t>(b) / constants::BITS_PER_BYTE;
    uint8_t src_bit_idx = static_cast<uint8_t>(b % constants::BITS_PER_BYTE);
    uint8_t bit_val = 0U;
    if (src_byte_idx < src_bytes.size()) {
      uint8_t s = std::to_integer<uint8_t>(src_bytes[src_byte_idx]);
      bit_val = static_cast<uint8_t>((s >> src_bit_idx) & 0x01U);
    }
    uint32_t target_bit = dst_bit_offset + static_cast<uint32_t>(b);
    size_t dst_byte_idx = static_cast<size_t>(target_bit) / constants::BITS_PER_BYTE;
    uint8_t dst_bit_shift =
        static_cast<uint8_t>(target_bit % constants::BITS_PER_BYTE);
    if (dst_byte_idx < dst.size()) {
      uint8_t cur = std::to_integer<uint8_t>(dst[dst_byte_idx]);
      uint8_t mask = static_cast<uint8_t>(1U << dst_bit_shift);
      if (bit_val != 0U) {
        cur = static_cast<uint8_t>(cur | mask);
      } else {
        cur = static_cast<uint8_t>(cur & static_cast<uint8_t>(~mask));
      }
      dst[dst_byte_idx] = static_cast<byte>(cur);
    }
  }
}

/// @brief Unpack `bit_len` bits from `src` at `src_bit_offset` into `dst_bytes`.
void extract_bits_from_buffer(std::span<const byte> src,
                              uint32_t src_bit_offset,
                              std::span<byte> dst_bytes,
                              uint16_t bit_len) noexcept {
  for (uint16_t b = 0U; b < bit_len; ++b) {
    uint32_t source_bit = src_bit_offset + static_cast<uint32_t>(b);
    size_t src_byte_idx = static_cast<size_t>(source_bit) / constants::BITS_PER_BYTE;
    uint8_t src_bit_shift =
        static_cast<uint8_t>(source_bit % constants::BITS_PER_BYTE);
    uint8_t bit_val = 0U;
    if (src_byte_idx < src.size()) {
      uint8_t s = std::to_integer<uint8_t>(src[src_byte_idx]);
      bit_val = static_cast<uint8_t>((s >> src_bit_shift) & 0x01U);
    }
    size_t dst_byte_idx = static_cast<size_t>(b) / constants::BITS_PER_BYTE;
    uint8_t dst_bit_shift = static_cast<uint8_t>(b % constants::BITS_PER_BYTE);
    if (dst_byte_idx < dst_bytes.size()) {
      uint8_t cur = std::to_integer<uint8_t>(dst_bytes[dst_byte_idx]);
      uint8_t mask = static_cast<uint8_t>(1U << dst_bit_shift);
      if (bit_val != 0U) {
        cur = static_cast<uint8_t>(cur | mask);
      } else {
        cur = static_cast<uint8_t>(cur & static_cast<uint8_t>(~mask));
      }
      dst_bytes[dst_byte_idx] = static_cast<byte>(cur);
    }
  }
}
} // namespace

/// @brief Construct CoeServer referencing ObjectDictionary & EscController.
CoeServer::CoeServer(ObjectDictionary &od, EscController &controller)
    : od_{od}, controller_{controller} {}

/// @brief Set optional SDO download observer hook.
void CoeServer::set_observer(ICoeObserver *observer) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == true) {
    observer_ = observer;
  }
}

/// @brief Return ETG.1000 mailbox type constant for CoE (`0x03`).
uint8_t CoeServer::protocol_type() const noexcept {
  return constants::MBX_TYPE_COE;
}

/// @brief Clear active segmented transfer state.
void CoeServer::reset() noexcept {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == true) {
    seg_ = SegmentedSession{};
  }
}

/// @brief Dispatch CoE Mailbox service frame cleanly (`SDOREQ`, `SDOINFO`).
uint16_t CoeServer::handle_request(const MailboxHeader &req_header,
                                   std::span<const byte> req_payload,
                                   std::vector<byte> &out_tx_payload,
                                   uint8_t &out_tx_type, bool &out_send_reply) {
  out_tx_type = constants::MBX_TYPE_COE;
  out_send_reply = false;
  if (req_payload.size() < constants::COE_HEADER_SIZE) {
    return constants::MBXERR_SIZE_TOO_SHORT;
  }
  uint16_t coe_word = read_le16(req_payload, 0U).value_or(0U);
  CoeHeader coe_hdr{};
  coe_hdr.number_service = coe_word;
  uint8_t service = coe_hdr.service();
  // Acquire mutex before touching segmented SDO session state.
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return constants::MBXERR_NO_MORE_MEMORY;
  }
  uint16_t max_payload = controller_.config().mbx_size;
  if (req_header.length > 0U) {
    Result<SyncManagerRegisters> sm1 =
        controller_.sync_manager(constants::SM_INDEX_MBX_IN);
    if (sm1.has_value() == true) {
      max_payload = static_cast<uint16_t>(
          sm1.value().length - constants::MBX_HEADER_SIZE);
    }
  }
  if (service == constants::COE_SERVICE_SDOREQ) {
    handle_sdo_request_locked(req_payload, max_payload, out_tx_payload);
    out_send_reply = (out_tx_payload.empty() == false);
    return 0U;
  } else if (service == constants::COE_SERVICE_SDOINFO) {
    handle_sdo_info_locked(req_payload, out_tx_payload);
    out_send_reply = (out_tx_payload.empty() == false);
    return 0U;
  } else {
    return constants::MBXERR_SERVICE_NOT_SUPPORTED;
  }
}

/// @brief Check read/write permissions against active AL state (`PreOp`/`SafeOp`/`Op`).
bool CoeServer::is_access_allowed(uint16_t access_flags, bool is_write) const {
  AlState st = controller_.snapshot().al_state;
  if (is_write == true) {
    if (st == AlState::PreOp) {
      return (access_flags & od_access::WRITE_PREOP) != 0U;
    } else if (st == AlState::SafeOp) {
      return (access_flags & od_access::WRITE_SAFEOP) != 0U;
    } else if (st == AlState::Op) {
      return (access_flags & od_access::WRITE_OP) != 0U;
    } else {
      return false;
    }
  } else {
    if (st == AlState::PreOp) {
      return (access_flags & od_access::READ_PREOP) != 0U;
    } else if (st == AlState::SafeOp) {
      return (access_flags & od_access::READ_SAFEOP) != 0U;
    } else if (st == AlState::Op) {
      return (access_flags & od_access::READ_OP) != 0U;
    } else {
      return false;
    }
  }
}

/// @brief Build a 10-byte CoE SDO Abort response frame.
void CoeServer::build_sdo_abort(uint16_t index, uint8_t subindex,
                                uint32_t abort_code,
                                std::vector<byte> &out_tx_payload) {
  out_tx_payload.assign(constants::COE_HEADER_SIZE + constants::SDO_HEADER_SIZE,
                        byte{0});
  CoeHeader coe_hdr{};
  coe_hdr.set(constants::COE_SERVICE_SDORES, 0U);
  (void)write_le16(out_tx_payload, 0U, coe_hdr.number_service);
  out_tx_payload[2U] = static_cast<byte>(constants::SDO_CCS_ABORT_REQ);
  (void)write_le16(out_tx_payload, 3U, index);
  out_tx_payload[5U] = static_cast<byte>(subindex);
  (void)write_le32(out_tx_payload, 6U, abort_code);
}

/// @brief Gather bytes for SDO Upload (single subindex or Complete Access).
uint32_t CoeServer::gather_upload_bytes(uint16_t index, uint8_t subindex,
                                        bool complete_access,
                                        std::vector<byte> &out_data) const {
  out_data.clear();
  Result<OdObjectEntry> obj_res = od_.get_object(index);
  if (obj_res.has_value() == false) {
    return constants::SDO_ABORT_OBJ_NOT_EXIST;
  }
  const OdObjectEntry &OBJ = obj_res.value();
  if (complete_access == false) {
    for (size_t i = 0U; i < OBJ.subindices.size(); ++i) {
      if (OBJ.subindices[i].subindex == subindex) {
        if (is_access_allowed(OBJ.subindices[i].access_flags, false) == false) {
          return constants::SDO_ABORT_WRITE_ONLY;
        }
        out_data = OBJ.subindices[i].data;
        return 0U;
      }
    }
    return constants::SDO_ABORT_SUBINDEX_NOT_EXIST;
  } else {
    // Complete Access supports starting at subindex 0 or 1 across all entries.
    if (subindex > 1U) {
      return constants::SDO_ABORT_SUBINDEX_NOT_EXIST;
    }
    for (size_t i = 0U; i < OBJ.subindices.size(); ++i) {
      if (OBJ.subindices[i].subindex >= subindex) {
        if (is_access_allowed(OBJ.subindices[i].access_flags, false) == false) {
          return constants::SDO_ABORT_WRITE_ONLY;
        }
        // Pad 1-byte subindex 0 to 2-byte boundary when following records exist.
        out_data.insert(out_data.end(), OBJ.subindices[i].data.begin(),
                        OBJ.subindices[i].data.end());
      }
    }
    return 0U;
  }
}

/// @brief Commit SDO Download bytes (single subindex or Complete Access).
uint32_t CoeServer::commit_download_bytes(uint16_t index, uint8_t subindex,
                                          bool complete_access,
                                          std::span<const byte> payload) {
  Result<OdObjectEntry> obj_res = od_.get_object(index);
  if (obj_res.has_value() == false) {
    return constants::SDO_ABORT_OBJ_NOT_EXIST;
  }
  const OdObjectEntry &OBJ = obj_res.value();
  if (complete_access == false) {
    for (size_t i = 0U; i < OBJ.subindices.size(); ++i) {
      if (OBJ.subindices[i].subindex == subindex) {
        if (is_access_allowed(OBJ.subindices[i].access_flags, true) == false) {
          return constants::SDO_ABORT_READ_ONLY;
        }
        if (observer_ != nullptr) {
          uint32_t pre_err = observer_->on_pre_sdo_write(index, subindex, payload);
          if (pre_err != 0U) {
            return pre_err;
          }
        }
        Result<> wres = od_.write_bytes(index, subindex, payload);
        if (wres.has_value() == false) {
          return constants::SDO_ABORT_TYPE_MISMATCH;
        }
        if (observer_ != nullptr) {
          observer_->on_post_sdo_write(index, subindex);
        }
        return 0U;
      }
    }
    return constants::SDO_ABORT_SUBINDEX_NOT_EXIST;
  } else {
    if (subindex > 1U) {
      return constants::SDO_ABORT_SUBINDEX_NOT_EXIST;
    }
    size_t cursor = 0U;
    for (size_t i = 0U; i < OBJ.subindices.size(); ++i) {
      if (OBJ.subindices[i].subindex >= subindex) {
        if (is_access_allowed(OBJ.subindices[i].access_flags, true) == false) {
          return constants::SDO_ABORT_READ_ONLY;
        }
        size_t need = OBJ.subindices[i].data.size();
        if ((cursor + need) > payload.size()) {
          return constants::SDO_ABORT_DATA_TOO_SHORT;
        }
        std::span<const byte> slice{payload.data() + cursor, need};
        Result<> wres = od_.write_bytes(index, OBJ.subindices[i].subindex, slice);
        if (wres.has_value() == false) {
          return constants::SDO_ABORT_VALUE_RANGE;
        }
        cursor += need;
      }
    }
    if (observer_ != nullptr) {
      observer_->on_post_sdo_write(index, subindex);
    }
    return 0U;
  }
}

/// @brief Dispatch SDO request by Command Specifier (CCS).
void CoeServer::handle_sdo_request_locked(std::span<const byte> req_payload,
                                          uint16_t max_mbx_payload,
                                          std::vector<byte> &out_tx_payload) {
  if (req_payload.size() <
      (constants::COE_HEADER_SIZE + constants::SDO_HEADER_SIZE)) {
    build_sdo_abort(0U, 0U, constants::SDO_ABORT_GENERAL, out_tx_payload);
    return;
  }
  SdoHeader hdr{};
  hdr.command = std::to_integer<uint8_t>(req_payload[2U]);
  hdr.index = read_le16(req_payload, 3U).value_or(0U);
  hdr.subindex = std::to_integer<uint8_t>(req_payload[5U]);
  hdr.size_or_data = read_le32(req_payload, 6U).value_or(0U);
  uint8_t ccs = static_cast<uint8_t>(hdr.command & constants::SDO_CCS_MASK);
  if (ccs == constants::SDO_CCS_UPLOAD_INIT_REQ) {
    handle_upload_init_locked(hdr, max_mbx_payload, out_tx_payload);
  } else if (ccs == constants::SDO_CCS_UPLOAD_SEG_REQ) {
    handle_upload_seg_locked(hdr, max_mbx_payload, out_tx_payload);
  } else if (ccs == constants::SDO_CCS_DOWNLOAD_INIT_REQ) {
    handle_download_init_locked(hdr, req_payload, out_tx_payload);
  } else if (ccs == constants::SDO_CCS_DOWNLOAD_SEG_REQ) {
    handle_download_seg_locked(hdr, req_payload, out_tx_payload);
  } else if (ccs == constants::SDO_CCS_ABORT_REQ) {
    // Master aborted transfer cleanly; discard active session.
    seg_ = SegmentedSession{};
    out_tx_payload.clear();
  } else {
    build_sdo_abort(hdr.index, hdr.subindex, constants::SDO_ABORT_CMD_INVALID,
                    out_tx_payload);
  }
}

/// @brief Handle Initiate SDO Upload (expedited <=4B or segmented normal >4B).
void CoeServer::handle_upload_init_locked(const SdoHeader &hdr,
                                          uint16_t max_mbx_payload,
                                          std::vector<byte> &out_tx_payload) {
  bool ca = (hdr.command & constants::SDO_FLAG_COMPLETE_ACCESS) != 0U;
  std::vector<byte> payload{};
  uint32_t err = gather_upload_bytes(hdr.index, hdr.subindex, ca, payload);
  if (err != 0U) {
    build_sdo_abort(hdr.index, hdr.subindex, err, out_tx_payload);
    return;
  }
  uint8_t ca_bit = (ca == true) ? constants::SDO_FLAG_COMPLETE_ACCESS : 0U;
  // Use Expedited transfer when payload is <= 4 bytes and not forced normal.
  if (payload.size() <= constants::SDO_EXPEDITED_MAX_BYTES) {
    out_tx_payload.assign(constants::COE_HEADER_SIZE + constants::SDO_HEADER_SIZE,
                          byte{0});
    CoeHeader coe_hdr{};
    coe_hdr.set(constants::COE_SERVICE_SDORES, 0U);
    (void)write_le16(out_tx_payload, 0U, coe_hdr.number_service);
    uint8_t unused = static_cast<uint8_t>(
        constants::SDO_EXPEDITED_MAX_BYTES - payload.size());
    uint8_t cs = static_cast<uint8_t>(
        constants::SDO_SCS_UPLOAD_INIT_RES | constants::SDO_FLAG_EXPEDITED |
        constants::SDO_FLAG_SIZE_IND | ca_bit |
        static_cast<uint8_t>(unused << constants::SDO_SIZE_SHIFT));
    out_tx_payload[2U] = static_cast<byte>(cs);
    (void)write_le16(out_tx_payload, 3U, hdr.index);
    out_tx_payload[5U] = static_cast<byte>(hdr.subindex);
    // Copy expedited scalar/string payload (up to 4 bytes).
    for (size_t k = 0U; k < payload.size(); ++k) {
      out_tx_payload[6U + k] = payload[k];
    }
    return;
  }
  // Normal upload: fit first chunk inside remaining mailbox space.
  size_t hdr_bytes = constants::COE_HEADER_SIZE + constants::SDO_HEADER_SIZE;
  size_t max_first =
      (max_mbx_payload > hdr_bytes) ? (max_mbx_payload - hdr_bytes) : 0U;
  size_t first_chunk = std::min(payload.size(), max_first);
  out_tx_payload.assign(hdr_bytes + first_chunk, byte{0});
  CoeHeader coe_hdr{};
  coe_hdr.set(constants::COE_SERVICE_SDORES, 0U);
  (void)write_le16(out_tx_payload, 0U, coe_hdr.number_service);
  uint8_t cs = static_cast<uint8_t>(constants::SDO_SCS_UPLOAD_INIT_RES |
                                    constants::SDO_FLAG_SIZE_IND | ca_bit);
  out_tx_payload[2U] = static_cast<byte>(cs);
  // Encode index/subindex, total byte count, and initial payload segment.
  (void)write_le16(out_tx_payload, 3U, hdr.index);
  out_tx_payload[5U] = static_cast<byte>(hdr.subindex);
  (void)write_le32(out_tx_payload, 6U, static_cast<uint32_t>(payload.size()));
  for (size_t k = 0U; k < first_chunk; ++k) {
    out_tx_payload[hdr_bytes + k] = payload[k];
  }
  // Track remaining bytes if transfer spans follow-up SDO Upload Segments.
  if (first_chunk < payload.size()) {
    seg_.mode = SegmentedMode::Upload;
    seg_.index = hdr.index;
    seg_.subindex = hdr.subindex;
    seg_.complete_access = ca;
    seg_.expected_toggle = false;
    seg_.total_bytes = static_cast<uint32_t>(payload.size());
    seg_.offset = first_chunk;
    seg_.buffer = std::move(payload);
  } else {
    seg_ = SegmentedSession{};
  }
}

/// @brief Handle Segmented SDO Upload Request cleanly checking toggle bit.
void CoeServer::handle_upload_seg_locked(const SdoHeader &hdr,
                                         uint16_t max_mbx_payload,
                                         std::vector<byte> &out_tx_payload) {
  if (seg_.mode != SegmentedMode::Upload) {
    build_sdo_abort(hdr.index, hdr.subindex, constants::SDO_ABORT_CMD_INVALID,
                    out_tx_payload);
    return;
  }
  bool toggle = (hdr.command & constants::SDO_FLAG_TOGGLE) != 0U;
  if (toggle != seg_.expected_toggle) {
    seg_ = SegmentedSession{};
    build_sdo_abort(hdr.index, hdr.subindex, constants::SDO_ABORT_TOGGLE,
                    out_tx_payload);
    return;
  }
  size_t seg_hdr_bytes = constants::COE_HEADER_SIZE + 1U;
  size_t remaining = seg_.buffer.size() - seg_.offset;
  size_t max_chunk =
      (max_mbx_payload > seg_hdr_bytes) ? (max_mbx_payload - seg_hdr_bytes) : 0U;
  size_t chunk = std::min(remaining, max_chunk);
  bool last = (seg_.offset + chunk) >= seg_.buffer.size();
  // Minimum 10-byte SDO segment response layout per ETG.1000.6.
  size_t payload_pad = std::max(chunk, size_t{7U});
  out_tx_payload.assign(seg_hdr_bytes + payload_pad, byte{0});
  CoeHeader coe_hdr{};
  coe_hdr.set(constants::COE_SERVICE_SDORES, 0U);
  (void)write_le16(out_tx_payload, 0U, coe_hdr.number_service);
  uint8_t toggle_bit = (toggle == true) ? constants::SDO_FLAG_TOGGLE : 0U;
  uint8_t more_bit = (last == true) ? 0U : constants::SDO_FLAG_MORE_FOLLOWS_LAST;
  uint8_t unused = (chunk < 7U) ? static_cast<uint8_t>((7U - chunk) << 1U) : 0U;
  out_tx_payload[2U] = static_cast<byte>(
      constants::SDO_SCS_UPLOAD_SEG_RES | toggle_bit | more_bit | unused);
  for (size_t k = 0U; k < chunk; ++k) {
    out_tx_payload[seg_hdr_bytes + k] = seg_.buffer[seg_.offset + k];
  }
  seg_.offset += chunk;
  seg_.expected_toggle = !seg_.expected_toggle;
  if (last == true) {
    seg_ = SegmentedSession{};
  }
}

/// @brief Handle Initiate SDO Download (expedited <=4B or segmented normal >4B).
void CoeServer::handle_download_init_locked(const SdoHeader &hdr,
                                            std::span<const byte> req_payload,
                                            std::vector<byte> &out_tx_payload) {
  bool ca = (hdr.command & constants::SDO_FLAG_COMPLETE_ACCESS) != 0U;
  bool expedited = (hdr.command & constants::SDO_FLAG_EXPEDITED) != 0U;
  bool size_ind = (hdr.command & constants::SDO_FLAG_SIZE_IND) != 0U;
  std::vector<byte> rx_data{};
  if (expedited == true) {
    size_t unused = (size_ind == true)
                        ? static_cast<size_t>((hdr.command >> 2U) & 0x03U)
                        : 0U;
    size_t len = constants::SDO_EXPEDITED_MAX_BYTES - unused;
    rx_data.assign(len, byte{0});
    for (size_t k = 0U; k < len; ++k) {
      rx_data[k] = req_payload[6U + k];
    }
    uint32_t err = commit_download_bytes(hdr.index, hdr.subindex, ca, rx_data);
    if (err != 0U) {
      build_sdo_abort(hdr.index, hdr.subindex, err, out_tx_payload);
      return;
    }
  } else {
    size_t total_len = static_cast<size_t>(hdr.size_or_data);
    size_t hdr_bytes = constants::COE_HEADER_SIZE + constants::SDO_HEADER_SIZE;
    size_t avail = (req_payload.size() > hdr_bytes) ? (req_payload.size() - hdr_bytes) : 0U;
    size_t first_copy = std::min(total_len, avail);
    if (first_copy == total_len) {
      std::span<const byte> full{req_payload.data() + hdr_bytes, total_len};
      uint32_t err = commit_download_bytes(hdr.index, hdr.subindex, ca, full);
      if (err != 0U) {
        build_sdo_abort(hdr.index, hdr.subindex, err, out_tx_payload);
        return;
      }
    } else {
      seg_.mode = SegmentedMode::Download;
      seg_.index = hdr.index;
      seg_.subindex = hdr.subindex;
      seg_.complete_access = ca;
      seg_.expected_toggle = false;
      seg_.total_bytes = hdr.size_or_data;
      seg_.offset = first_copy;
      seg_.buffer.assign(total_len, byte{0});
      for (size_t k = 0U; k < first_copy; ++k) {
        seg_.buffer[k] = req_payload[hdr_bytes + k];
      }
    }
  }
  // Build standard Initiate SDO Download response.
  out_tx_payload.assign(constants::COE_HEADER_SIZE + constants::SDO_HEADER_SIZE,
                        byte{0});
  CoeHeader coe_hdr{};
  coe_hdr.set(constants::COE_SERVICE_SDORES, 0U);
  (void)write_le16(out_tx_payload, 0U, coe_hdr.number_service);
  uint8_t ca_bit = (ca == true) ? constants::SDO_FLAG_COMPLETE_ACCESS : 0U;
  out_tx_payload[2U] =
      static_cast<byte>(constants::SDO_SCS_DOWNLOAD_INIT_RES | ca_bit);
  (void)write_le16(out_tx_payload, 3U, hdr.index);
  out_tx_payload[5U] = static_cast<byte>(hdr.subindex);
}

/// @brief Handle Segmented SDO Download Request and commit when final segment arrives.
void CoeServer::handle_download_seg_locked(const SdoHeader &hdr,
                                           std::span<const byte> req_payload,
                                           std::vector<byte> &out_tx_payload) {
  if (seg_.mode != SegmentedMode::Download) {
    build_sdo_abort(hdr.index, hdr.subindex, constants::SDO_ABORT_CMD_INVALID,
                    out_tx_payload);
    return;
  }
  bool toggle = (hdr.command & constants::SDO_FLAG_TOGGLE) != 0U;
  if (toggle != seg_.expected_toggle) {
    seg_ = SegmentedSession{};
    build_sdo_abort(hdr.index, hdr.subindex, constants::SDO_ABORT_TOGGLE,
                    out_tx_payload);
    return;
  }
  size_t seg_hdr_bytes = constants::COE_HEADER_SIZE + 1U;
  size_t avail =
      (req_payload.size() > seg_hdr_bytes) ? (req_payload.size() - seg_hdr_bytes) : 0U;
  size_t remaining = seg_.buffer.size() - seg_.offset;
  size_t take = std::min(avail, remaining);
  for (size_t k = 0U; k < take; ++k) {
    seg_.buffer[seg_.offset + k] = req_payload[seg_hdr_bytes + k];
  }
  seg_.offset += take;
  bool last = (hdr.command & constants::SDO_FLAG_MORE_FOLLOWS_LAST) == 0U;
  if ((last == true) || (seg_.offset >= seg_.buffer.size())) {
    uint32_t err = commit_download_bytes(seg_.index, seg_.subindex,
                                         seg_.complete_access, seg_.buffer);
    uint16_t err_idx = seg_.index;
    uint8_t err_sub = seg_.subindex;
    seg_ = SegmentedSession{};
    if (err != 0U) {
      build_sdo_abort(err_idx, err_sub, err, out_tx_payload);
      return;
    }
  } else {
    seg_.expected_toggle = !seg_.expected_toggle;
  }
  out_tx_payload.assign(constants::COE_HEADER_SIZE + constants::SDO_HEADER_SIZE,
                        byte{0});
  CoeHeader coe_hdr{};
  coe_hdr.set(constants::COE_SERVICE_SDORES, 0U);
  (void)write_le16(out_tx_payload, 0U, coe_hdr.number_service);
  uint8_t toggle_bit = (toggle == true) ? constants::SDO_FLAG_TOGGLE : 0U;
  out_tx_payload[2U] =
      static_cast<byte>(constants::SDO_SCS_DOWNLOAD_SEG_RES | toggle_bit);
}

/// @brief Handle CoE SDO Information queries (`GET_ODLIST`, `GET_OD`, `GET_ED`).
void CoeServer::handle_sdo_info_locked(std::span<const byte> req_payload,
                                       std::vector<byte> &out_tx_payload) {
  if (req_payload.size() <
      (constants::COE_HEADER_SIZE + constants::SDO_INFO_HEADER_SIZE)) {
    return;
  }
  uint8_t opcode = static_cast<uint8_t>(
      std::to_integer<uint8_t>(req_payload[2U]) & 0x7FU);
  uint16_t arg = read_le16(req_payload, 6U).value_or(0U);
  CoeHeader coe_hdr{};
  coe_hdr.set(constants::COE_SERVICE_SDOINFO, 0U);
  if (opcode == constants::SDOINFO_OP_GET_ODLIST_REQ) {
    std::vector<uint16_t> indices = od_.list_indices();
    out_tx_payload.assign(constants::COE_HEADER_SIZE + constants::SDO_INFO_HEADER_SIZE +
                              (indices.size() * 2U),
                          byte{0});
    (void)write_le16(out_tx_payload, 0U, coe_hdr.number_service);
    out_tx_payload[2U] = static_cast<byte>(constants::SDOINFO_OP_GET_ODLIST_RES);
    (void)write_le16(out_tx_payload, 6U, arg);
    for (size_t k = 0U; k < indices.size(); ++k) {
      (void)write_le16(out_tx_payload, 8U + (k * 2U), indices[k]);
    }
  } else if (opcode == constants::SDOINFO_OP_GET_OD_REQ) {
    Result<OdObjectEntry> obj_res = od_.get_object(arg);
    if (obj_res.has_value() == false) {
      out_tx_payload.assign(constants::COE_HEADER_SIZE + 10U, byte{0});
      (void)write_le16(out_tx_payload, 0U, coe_hdr.number_service);
      out_tx_payload[2U] = static_cast<byte>(constants::SDOINFO_OP_ERROR_RES);
      (void)write_le32(out_tx_payload, 8U, constants::SDO_ABORT_OBJ_NOT_EXIST);
      return;
    }
    const OdObjectEntry &OBJ = obj_res.value();
    size_t name_len = OBJ.name.size();
    out_tx_payload.assign(constants::COE_HEADER_SIZE + constants::SDO_INFO_HEADER_SIZE +
                              6U + name_len,
                          byte{0});
    (void)write_le16(out_tx_payload, 0U, coe_hdr.number_service);
    out_tx_payload[2U] = static_cast<byte>(constants::SDOINFO_OP_GET_OD_RES);
    (void)write_le16(out_tx_payload, 6U, OBJ.index);
    uint16_t first_dtype = OBJ.subindices.empty()
                               ? uint16_t{0}
                               : static_cast<uint16_t>(OBJ.subindices[0U].data_type);
    (void)write_le16(out_tx_payload, 8U, first_dtype);
    out_tx_payload[10U] = static_cast<byte>(OBJ.max_subindex);
    out_tx_payload[11U] = static_cast<byte>(static_cast<uint16_t>(OBJ.object_type) & 0xFFU);
    // Append ASCII object name string.
    for (size_t k = 0U; k < name_len; ++k) {
      out_tx_payload[14U + k] = static_cast<byte>(OBJ.name[k]);
    }
  } else if (opcode == constants::SDOINFO_OP_GET_ED_REQ) {
    if (req_payload.size() < 9U) {
      return;
    }
    uint8_t sub_idx = std::to_integer<uint8_t>(req_payload[8U]);
    Result<OdSubindexEntry> sub_res = od_.get_subindex(arg, sub_idx);
    if (sub_res.has_value() == false) {
      out_tx_payload.assign(constants::COE_HEADER_SIZE + 10U, byte{0});
      (void)write_le16(out_tx_payload, 0U, coe_hdr.number_service);
      out_tx_payload[2U] = static_cast<byte>(constants::SDOINFO_OP_ERROR_RES);
      (void)write_le32(out_tx_payload, 8U, constants::SDO_ABORT_SUBINDEX_NOT_EXIST);
      return;
    }
    const OdSubindexEntry &SUB = sub_res.value();
    size_t name_len = SUB.name.size();
    out_tx_payload.assign(constants::COE_HEADER_SIZE + constants::SDO_INFO_HEADER_SIZE +
                              10U + name_len,
                          byte{0});
    (void)write_le16(out_tx_payload, 0U, coe_hdr.number_service);
    out_tx_payload[2U] = static_cast<byte>(constants::SDOINFO_OP_GET_ED_RES);
    (void)write_le16(out_tx_payload, 6U, arg);
    out_tx_payload[8U] = static_cast<byte>(SUB.subindex);
    out_tx_payload[9U] = byte{0}; // ValueInfo
    (void)write_le16(out_tx_payload, 10U, static_cast<uint16_t>(SUB.data_type));
    (void)write_le16(out_tx_payload, 12U, SUB.bit_length);
    (void)write_le16(out_tx_payload, 14U, SUB.access_flags);
    for (size_t k = 0U; k < name_len; ++k) {
      out_tx_payload[16U + k] = static_cast<byte>(SUB.name[k]);
    }
  } else {
    // Unknown SDO Information opcode safely ignored.
  }
}

/// @brief Sum mapped PDO bit lengths under `0x1C12` or `0x1C13` (`sizeOfPDO` symmetry).
Result<uint32_t> CoeServer::compute_pdo_size_bits(
    uint16_t sm_assign_index) const {
  Result<OdObjectEntry> assign_res = od_.get_object(sm_assign_index);
  if (assign_res.has_value() == false) {
    return 0U;
  }
  uint32_t count = od_.read_u32(sm_assign_index, 0U).value_or(0U);
  uint32_t total_bits = 0U;
  for (uint32_t pdo_sub = 1U; pdo_sub <= count; ++pdo_sub) {
    if (pdo_sub > constants::MAX_OD_ITERATIONS) {
      return std::unexpected(SlvError::LoopLimitExceeded);
    }
    uint32_t map_idx_u32 =
        od_.read_u32(sm_assign_index, static_cast<uint8_t>(pdo_sub)).value_or(0U);
    uint16_t map_idx = static_cast<uint16_t>(map_idx_u32 & 0xFFFFU);
    uint32_t map_cnt = od_.read_u32(map_idx, 0U).value_or(0U);
    for (uint32_t entry_sub = 1U; entry_sub <= map_cnt; ++entry_sub) {
      if (entry_sub > constants::MAX_OD_ITERATIONS) {
        return std::unexpected(SlvError::LoopLimitExceeded);
      }
      uint32_t map_word =
          od_.read_u32(map_idx, static_cast<uint8_t>(entry_sub)).value_or(0U);
      uint8_t bit_len = static_cast<uint8_t>(map_word & 0xFFU);
      total_bits += static_cast<uint32_t>(bit_len);
    }
  }
  return total_bits;
}

/// @brief Compute rounded-up RxPDO and TxPDO byte lengths from `0x1C12` & `0x1C13`.
Result<> CoeServer::compute_pdo_sizes_bytes(uint16_t &out_rx_bytes,
                                            uint16_t &out_tx_bytes) const {
  Result<uint32_t> rx_bits = compute_pdo_size_bits(constants::OD_IDX_SM2_ASSIGN);
  Result<uint32_t> tx_bits = compute_pdo_size_bits(constants::OD_IDX_SM3_ASSIGN);
  if ((rx_bits.has_value() == false) || (tx_bits.has_value() == false)) {
    return std::unexpected(SlvError::OutOfBounds);
  }
  out_rx_bytes = static_cast<uint16_t>((rx_bits.value() + 7U) / 8U);
  out_tx_bytes = static_cast<uint16_t>((tx_bits.value() + 7U) / 8U);
  return {};
}

/// @brief Pack mapped TxPDO (`0x1C13`) variables bit-by-bit into SM3 byte buffer.
Result<uint32_t> CoeServer::pack_tx_pdo(std::span<byte> tx_buffer) const {
  uint32_t count = od_.read_u32(constants::OD_IDX_SM3_ASSIGN, 0U).value_or(0U);
  uint32_t bit_offset = 0U;
  for (uint32_t pdo_sub = 1U; pdo_sub <= count; ++pdo_sub) {
    uint16_t map_idx = static_cast<uint16_t>(
        od_.read_u32(constants::OD_IDX_SM3_ASSIGN, static_cast<uint8_t>(pdo_sub))
            .value_or(0U));
    uint32_t map_cnt = od_.read_u32(map_idx, 0U).value_or(0U);
    for (uint32_t entry_sub = 1U; entry_sub <= map_cnt; ++entry_sub) {
      // Unpack mapping word: [index:16 | subindex:8 | bit_length:8].
      uint32_t map_word =
          od_.read_u32(map_idx, static_cast<uint8_t>(entry_sub)).value_or(0U);
      uint16_t target_idx = static_cast<uint16_t>((map_word >> 16U) & 0xFFFFU);
      uint8_t target_sub = static_cast<uint8_t>((map_word >> 8U) & 0xFFU);
      uint16_t bit_len = static_cast<uint16_t>(map_word & 0xFFU);
      if (target_idx != 0U) {
        Result<std::vector<byte>> val = od_.read_bytes(target_idx, target_sub);
        if (val.has_value() == false) {
          return std::unexpected(val.error());
        }
        // Pack variable value at current unaligned bit cursor inside SM3.
        copy_bits_to_buffer(tx_buffer, bit_offset, val.value(), bit_len);
      }
      bit_offset += static_cast<uint32_t>(bit_len);
    }
  }
  return bit_offset;
}

/// @brief Unpack SM2 RxPDO bytes bit-by-bit into mapped `0x1C12` OD variables.
Result<uint32_t> CoeServer::unpack_rx_pdo(std::span<const byte> rx_buffer) {
  uint32_t count = od_.read_u32(constants::OD_IDX_SM2_ASSIGN, 0U).value_or(0U);
  uint32_t bit_offset = 0U;
  for (uint32_t pdo_sub = 1U; pdo_sub <= count; ++pdo_sub) {
    uint16_t map_idx = static_cast<uint16_t>(
        od_.read_u32(constants::OD_IDX_SM2_ASSIGN, static_cast<uint8_t>(pdo_sub))
            .value_or(0U));
    uint32_t map_cnt = od_.read_u32(map_idx, 0U).value_or(0U);
    for (uint32_t entry_sub = 1U; entry_sub <= map_cnt; ++entry_sub) {
      uint32_t map_word =
          od_.read_u32(map_idx, static_cast<uint8_t>(entry_sub)).value_or(0U);
      uint16_t target_idx = static_cast<uint16_t>((map_word >> 16U) & 0xFFFFU);
      uint8_t target_sub = static_cast<uint8_t>((map_word >> 8U) & 0xFFU);
      uint16_t bit_len = static_cast<uint16_t>(map_word & 0xFFU);
      if (target_idx != 0U) {
        // Extract mapped SM2 bit field and write updated value back to OD.
        Result<std::vector<byte>> cur = od_.read_bytes(target_idx, target_sub);
        if (cur.has_value() == false) {
          return std::unexpected(cur.error());
        }
        std::vector<byte> updated{};
        updated.resize(cur.value().size(), byte{0});
        extract_bits_from_buffer(rx_buffer, bit_offset, updated, bit_len);
        Result<> wres = od_.write_bytes(target_idx, target_sub, updated);
        if (wres.has_value() == false) {
          return std::unexpected(wres.error());
        }
      }
      bit_offset += static_cast<uint32_t>(bit_len);
    }
  }
  return bit_offset;
}

/// @brief Enqueue an 8-byte CoE Emergency object onto MailboxServer (`0x01`).
Result<> CoeServer::emit_emergency(MailboxServer &mailbox, uint16_t error_code,
                                   uint8_t error_register,
                                   std::span<const byte> manufacturer_data) {
  std::array<byte, 10U> payload{};
  CoeHeader coe_hdr{};
  coe_hdr.set(constants::COE_SERVICE_EMERGENCY, 0U);
  (void)write_le16(payload, 0U, coe_hdr.number_service);
  (void)write_le16(payload, 2U, error_code);
  payload[4U] = static_cast<byte>(error_register);
  size_t copy_n = std::min(manufacturer_data.size(), size_t{5U});
  for (size_t k = 0U; k < copy_n; ++k) {
    payload[5U + k] = manufacturer_data[k];
  }
  return mailbox.enqueue_tx(constants::MBX_TYPE_COE, payload);
}

} // namespace resoes
