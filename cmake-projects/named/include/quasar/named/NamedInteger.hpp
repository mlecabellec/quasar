/**
 * @file NamedInteger.hpp
 * @brief Template class for named integer values.
 */

#ifndef QUASAR_NAMED_NAMEDINTEGER_HPP
#define QUASAR_NAMED_NAMEDINTEGER_HPP

#include <optional>
#include "quasar/coretypes/Integer.hpp"
#include "quasar/coretypes/Buffer.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/IBoundPrimitive.hpp"
#include "quasar/named/CopyPolicy.hpp"

namespace quasar::named {

/**
 * @class NamedInteger
 * @brief A named object that holds an integer value.
 *
 * This class combines the hierarchical capabilities of NamedObject with the
 * integer value management of coretypes::Integer.
 * 
 * **Compliance**:
 * - Fulfills [FE-0020.4] Derivative class for Integer core type.
 * - Fulfills [CS-0010.34] auto forbidden.
 *
 * @tparam T The underlying integer type (e.g., int, uint32_t, int64_t).
 */
template <typename T>
class NamedInteger : public NamedObject, public quasar::coretypes::Integer<T>, public IBoundPrimitive {
public:
  /**
   * @brief Destructor.
   */
  virtual ~NamedInteger() = default;

  /**
   * @brief factory method to create a NamedInteger.
   * 
   * Fulfills [FE-0020.6] static method "create".
   *
   * @param name The name of the integer object.
   * @param value The initial integer value.
   * @param parent Optional parent in the hierarchy.
   * @return Shared pointer to the new NamedInteger instance.
   */
  static std::shared_ptr<NamedInteger<T>>
  create(const std::string &name, T value,
         std::shared_ptr<NamedObject> parent = nullptr) {
    // Instantiate the NamedInteger using shared_ptr to comply with CS-0010.6.
    std::shared_ptr<NamedInteger<T>> obj =
        std::make_shared<NamedInteger<T>>(name, value);

    // Initialize self-reference for getSelf().
    obj->setSelf(obj);

    // Attach to parent if provided.
    if (parent != nullptr) {
      obj->setParent(parent);
    }
    return obj;
  }

  /**
   * @brief Creates a clone of this NamedInteger.
   * 
   * Fulfills [FE-0020.14] Utilities for copying parts of the tree.
   * 
   * @param policy Memory policy (DUPLICATE vs SHARE).
   * @return A new NamedInteger with the same name and value, but no hierarchy.
   */
  std::shared_ptr<NamedObject> clone(CopyPolicy policy = CopyPolicy::DUPLICATE) const override {
    if (policy == CopyPolicy::SHARE && m_bound == true) {
        std::shared_ptr<NamedInteger<T>> newObj = create(this->getName(), this->value());
        newObj->bind(m_backingStore.lock(), m_bound_offset);
        newObj->setEndianness(m_endian);
        return newObj;
    }
    return create(this->getName(), this->value());
  }

  // --- IBoundPrimitive implementation ---
  /**
   * @brief Returns whether this object is currently bound to a parent buffer.
   * @return true if bound.
   * @feature [TSK-20260311-001.6] Buffer-to-Primitive Binding.
   */
  bool isBound() const override { return (m_bound == true); }
  /** @brief Returns the offset in bytes. */
  std::size_t getBoundOffset() const override { return m_bound_offset; }
  /** @brief Returns the length in bytes. */
  std::size_t getBoundLength() const override { return sizeof(T); }

  /**
   * @brief Sets the endianness for buffer synchronization.
   * @param endian The endianness to use.
   */
  void setEndianness(quasar::coretypes::Endianness endian) override {
      m_endian = endian;
      if (m_bound == true) {
          syncFromBuffer();
      }
  }

  /**
   * @brief Returns the current endianness used for synchronization.
   * @return The endianness.
   */
  quasar::coretypes::Endianness getEndianness() const override {
      return m_endian;
  }

  /**
   * @brief Binds this integer to a specific memory offset in a buffer.
   * 
   * Fulfills [TSK-20260311-001.6] Buffer-to-Primitive Binding.
   * 
   * @param buffer The source buffer.
   * @param offset The byte offset.
   * @param endian Optional endianness (defaults to current setting).
   * @throws std::out_of_range If offset is invalid.
   */
  void bind(std::shared_ptr<quasar::coretypes::Buffer> buffer, std::size_t offset,
            std::optional<quasar::coretypes::Endianness> endian = std::nullopt) {
      if (!buffer) return;
      if (offset + sizeof(T) > buffer->size()) {
          throw std::out_of_range("Binding offset out of range for buffer size");
      }
      m_bound = true;
      m_backingStore = buffer;
      m_bound_offset = offset;
      if (endian.has_value()) {
          m_endian = endian.value();
      }
      // Sync local value with buffer
      syncFromBuffer();
  }

  /**
   * @brief Retrieves the integer value, syncing from buffer if bound.
   * @return The current value.
   */
  T value() const {
      if (m_bound) {
          // Const-cast used to update cache from backing store in logically const operation.
          const_cast<NamedInteger<T>*>(this)->syncFromBuffer();
      }
      return quasar::coretypes::Integer<T>::value();
  }

  /**
   * @brief Sets the integer value, syncing to buffer if bound.
   * @param value The new value.
   */
  void setValue(T value) {
      if (quasar::coretypes::Integer<T>::value() != value) {
          quasar::coretypes::Integer<T>::setValue(value);
          if (m_bound) {
              syncToBuffer();
          }
          notifyObservers(getSelf());
      }
  }

  /**
   * @brief Returns the type of the object.
   * @return "NamedInteger"
   */
  std::string getType() const override { return "NamedInteger"; }

  /**
   * @brief Constructs a NamedInteger.
   * @param name The name of the object.
   * @param value The initial value.
   */
  NamedInteger(const std::string &name, T value)
      : NamedObject(name), quasar::coretypes::Integer<T>(value),
        m_bound(false), m_bound_offset(0), m_backingStore(),
        m_endian(quasar::coretypes::Endianness::BigEndian) {
  }

protected:
  /** @brief Flag indicating if the value is bound. */
  bool m_bound;
  /** @brief Byte offset in backing buffer. */
  std::size_t m_bound_offset;
  /** @brief Weak reference to backing store. */
  std::weak_ptr<quasar::coretypes::Buffer> m_backingStore;
  /** @brief Endianness for synchronization. */
  quasar::coretypes::Endianness m_endian;

  /**
   * @brief Synchronizes the local member value from the backing buffer.
   */
  void syncFromBuffer() {
      std::shared_ptr<quasar::coretypes::Buffer> buf = m_backingStore.lock();
      if (buf) {
          T val = buf->read<T>(m_bound_offset, m_endian);
          quasar::coretypes::Integer<T>::setValue(val);
      }
  }

  /**
   * @brief Synchronizes the local member value to the backing buffer.
   */
  void syncToBuffer() {
      std::shared_ptr<quasar::coretypes::Buffer> buf = m_backingStore.lock();
      if (buf) {
          T val = quasar::coretypes::Integer<T>::value();
          buf->write<T>(val, m_bound_offset, m_endian);
      }
  }
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDINTEGER_HPP
