#include "quasar/named/NamedBoolean.hpp"

namespace quasar::named {

std::shared_ptr<NamedBoolean> NamedBoolean::create(const std::string &name,
                                                   bool value,
                                                   std::shared_ptr<NamedObject> parent) {
  std::shared_ptr<NamedBoolean> obj = std::make_shared<NamedBoolean>(name, value);
  obj->setSelf(obj);
  if (parent) {
    obj->setParent(parent);
  }
  return obj;
}

std::string NamedBoolean::getType() const {
  return "NamedBoolean";
}

std::shared_ptr<NamedObject> NamedBoolean::clone(CopyPolicy policy) const {
  if (policy == CopyPolicy::SHARE && m_bound) {
    std::shared_ptr<NamedBoolean> newObj = create(getName(), booleanValue());
    newObj->bind(m_backingStore.lock(), m_bound_offset);
    newObj->setEndianness(m_endian);
    return newObj;
  }
  return create(getName(), booleanValue());
}

NamedBoolean::NamedBoolean(const std::string &name, bool value)
    : NamedObject(name), quasar::coretypes::Boolean(value),
      m_bound(false), m_bound_offset(0), m_backingStore(),
      m_endian(quasar::coretypes::Endianness::BigEndian) {}

void NamedBoolean::bind(std::shared_ptr<quasar::coretypes::Buffer> buffer, std::size_t offset,
                        std::optional<quasar::coretypes::Endianness> endian) {
  if (!buffer) {
    return;
  }
  // Check bounds: boolean is assumed to take 1 byte in the buffer.
  if (offset + 1 > buffer->size()) {
    throw std::out_of_range("Binding offset out of range for buffer size");
  }
  m_bound = true;
  m_backingStore = buffer;
  m_bound_offset = offset;
  if (endian.has_value()) {
      m_endian = endian.value();
  }
  // Sync local state from the newly bound buffer.
  syncFromBuffer();
}

void NamedBoolean::setEndianness(quasar::coretypes::Endianness endian) {
    m_endian = endian;
    // Endianness change triggers re-sync, though irrelevant for 1-byte.
    if (m_bound) {
        syncFromBuffer();
    }
}

quasar::coretypes::Endianness NamedBoolean::getEndianness() const {
    return m_endian;
}

bool NamedBoolean::booleanValue() const {
  if (m_bound) {
    // Sync from buffer before returning value to ensure live view behavior.
    const_cast<NamedBoolean*>(this)->syncFromBuffer();
  }
  return quasar::coretypes::Boolean::booleanValue();
}

void NamedBoolean::setValue(bool value) {
  // Only update and notify if the value actually changed.
  if (quasar::coretypes::Boolean::booleanValue() != value) {
    quasar::coretypes::Boolean::setValue(value);
    if (m_bound) {
      // Push changes to backing buffer if bound.
      syncToBuffer();
    }
    notifyObservers(getSelf());
  }
}

void NamedBoolean::syncFromBuffer() {
  std::shared_ptr<quasar::coretypes::Buffer> buf = m_backingStore.lock();
  if (buf) {
    // Map non-zero byte to true, zero to false.
    quasar::coretypes::Boolean::setValue(buf->get(m_bound_offset) != 0);
  }
}

void NamedBoolean::syncToBuffer() {
  std::shared_ptr<quasar::coretypes::Buffer> buf = m_backingStore.lock();
  if (buf) {
    // Persist boolean as 1 (true) or 0 (false) in the buffer.
    buf->set(m_bound_offset, quasar::coretypes::Boolean::booleanValue() ? 1 : 0);
  }
}

} // namespace quasar::named
