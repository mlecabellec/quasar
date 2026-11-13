/**
 * @file ObjectDictionary.cpp
 * @brief Thread-safe Object Dictionary implementation replacing global SDOobjects[].
 * @feature [TSK-20261113-001]
 */

#include "resoes/ObjectDictionary.hpp"
#include <array>
#include <chrono>

namespace resoes {

/// @brief Insert or update an OdObjectEntry inside the ordered dictionary map.
Result<> ObjectDictionary::register_object(OdObjectEntry entry) {
  if (entry.subindices.empty() == true) {
    return std::unexpected(SlvError::InvalidParameter);
  }
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  uint16_t idx = entry.index;
  objects_.insert_or_assign(idx, std::move(entry));
  return {};
}

/// @brief Register a simple scalar variable entry (`OdObjectType::Var`).
Result<> ObjectDictionary::add_variable(uint16_t index, OdDataType data_type,
                                        uint16_t bit_length, uint16_t access,
                                        const std::string &name,
                                        std::span<const byte> initial_bytes) {
  OdSubindexEntry sub{};
  sub.subindex = 0U;
  sub.data_type = data_type;
  sub.bit_length = bit_length;
  sub.access_flags = access;
  sub.name = name;
  // Populate initial scalar/string bytes.
  sub.data.assign(initial_bytes.begin(), initial_bytes.end());
  OdObjectEntry obj{};
  obj.index = index;
  obj.object_type = OdObjectType::Var;
  obj.max_subindex = 0U;
  obj.name = name;
  // Store single scalar entry at subindex 0 cleanly.
  obj.subindices.push_back(std::move(sub));
  return register_object(std::move(obj));
}

/// @brief List all object indices in ascending order.
std::vector<uint16_t> ObjectDictionary::list_indices() const {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return {};
  }
  std::vector<uint16_t> out{};
  out.reserve(objects_.size());
  for (const std::pair<const uint16_t, OdObjectEntry> &ITEM : objects_) {
    out.push_back(ITEM.first);
  }
  return out;
}

/// @brief Look up a full OdObjectEntry descriptor by index.
Result<OdObjectEntry> ObjectDictionary::get_object(uint16_t index) const {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  std::map<uint16_t, OdObjectEntry>::const_iterator it = objects_.find(index);
  if (it == objects_.end()) {
    return std::unexpected(SlvError::ObjectNotFound);
  }
  return it->second;
}

/// @brief Look up a single subindex descriptor by index & subindex.
Result<OdSubindexEntry> ObjectDictionary::get_subindex(uint16_t index,
                                                       uint8_t subindex) const {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  std::map<uint16_t, OdObjectEntry>::const_iterator it = objects_.find(index);
  if (it == objects_.end()) {
    return std::unexpected(SlvError::ObjectNotFound);
  }
  for (size_t i = 0U; i < it->second.subindices.size(); ++i) {
    if (it->second.subindices[i].subindex == subindex) {
      return it->second.subindices[i];
    }
  }
  return std::unexpected(SlvError::ObjectNotFound);
}

/// @brief Read raw little-endian bytes for an Object Dictionary subindex.
Result<std::vector<byte>> ObjectDictionary::read_bytes(uint16_t index,
                                                       uint8_t subindex) const {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  std::map<uint16_t, OdObjectEntry>::const_iterator it = objects_.find(index);
  if (it == objects_.end()) {
    return std::unexpected(SlvError::ObjectNotFound);
  }
  for (size_t i = 0U; i < it->second.subindices.size(); ++i) {
    if (it->second.subindices[i].subindex == subindex) {
      return it->second.subindices[i].data;
    }
  }
  return std::unexpected(SlvError::ObjectNotFound);
}

/// @brief Write raw little-endian bytes into an existing subindex slot.
Result<> ObjectDictionary::write_bytes(uint16_t index, uint8_t subindex,
                                       std::span<const byte> data) {
  std::unique_lock<std::timed_mutex> lock{
      mutex_, std::chrono::milliseconds{constants::MUTEX_TIMEOUT_MS}};
  if (lock.owns_lock() == false) {
    return std::unexpected(SlvError::MutexTimeout);
  }
  std::map<uint16_t, OdObjectEntry>::iterator it = objects_.find(index);
  if (it == objects_.end()) {
    return std::unexpected(SlvError::ObjectNotFound);
  }
  for (size_t i = 0U; i < it->second.subindices.size(); ++i) {
    if (it->second.subindices[i].subindex == subindex) {
      OdSubindexEntry &entry = it->second.subindices[i];
      if ((entry.data_type == OdDataType::VisibleString) ||
          (entry.data_type == OdDataType::OctetString)) {
        entry.data.assign(data.begin(), data.end());
        return {};
      } else {
        if (data.size() != entry.data.size()) {
          return std::unexpected(SlvError::InvalidParameter);
        }
        for (size_t k = 0U; k < data.size(); ++k) {
          entry.data[k] = data[k];
        }
        return {};
      }
    }
  }
  return std::unexpected(SlvError::ObjectNotFound);
}

/// @brief Decode scalar 8/16/32-bit unsigned value from subindex.
Result<uint32_t> ObjectDictionary::read_u32(uint16_t index,
                                            uint8_t subindex) const {
  Result<std::vector<byte>> bytes_res = read_bytes(index, subindex);
  if (bytes_res.has_value() == false) {
    return std::unexpected(bytes_res.error());
  }
  const std::vector<byte> &BUF = bytes_res.value();
  if (BUF.empty() == true) {
    return std::unexpected(SlvError::BufferTooSmall);
  }
  uint32_t accum = 0U;
  size_t count = std::min(BUF.size(), size_t{4U});
  for (size_t k = 0U; k < count; ++k) {
    accum |= (std::to_integer<uint32_t>(BUF[k]) << (k * constants::BITS_PER_BYTE));
  }
  return accum;
}

/// @brief Encode scalar unsigned value into matching 1/2/4-byte subindex storage.
Result<> ObjectDictionary::write_u32(uint16_t index, uint8_t subindex,
                                     uint32_t value) {
  Result<std::vector<byte>> cur = read_bytes(index, subindex);
  if (cur.has_value() == false) {
    return std::unexpected(cur.error());
  }
  size_t width = cur.value().size();
  if ((width == 0U) || (width > 4U)) {
    return std::unexpected(SlvError::InvalidParameter);
  }
  std::array<byte, 4U> tmp{};
  for (size_t k = 0U; k < width; ++k) {
    tmp[k] = static_cast<byte>((value >> (k * constants::BITS_PER_BYTE)) & 0xFFU);
  }
  return write_bytes(index, subindex, std::span<const byte>{tmp.data(), width});
}

} // namespace resoes
