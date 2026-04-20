/**
 * @file NamedFloatingPoint.hpp
 * @brief Template class for named floating point values.
 */

#ifndef QUASAR_NAMED_NAMEDFLOATINGPOINT_HPP
#define QUASAR_NAMED_NAMEDFLOATINGPOINT_HPP

#include "quasar/coretypes/FloatingPoint.hpp"
#include "quasar/coretypes/Buffer.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/IBoundPrimitive.hpp"
#include "quasar/named/CopyPolicy.hpp"

namespace quasar::named {

/**
 * @class NamedFloatingPoint
 * @brief A named object that holds a floating point value.
 *
 * This class combines hierarchical management from NamedObject with floating
 * point value handling from coretypes::FloatingPoint.
 * 
 * **Compliance**:
 * - Fulfills [FE-0020.4] Derivative class for FloatingPoint core type.
 * - Fulfills [CS-0010.34] auto forbidden.
 *
 * @tparam T The underlying floating point type (e.g., float, double).
 */
template <typename T>
class NamedFloatingPoint : public NamedObject,
                           public quasar::coretypes::FloatingPoint<T>,
                           public IBoundPrimitive {
public:
  /**
   * @brief Destructor.
   */
  virtual ~NamedFloatingPoint() = default;

  /**
   * @brief Factory method to create a NamedFloatingPoint.
   * 
   * Fulfills [FE-0020.6] static method "create".
   *
   * @param name The name of the floating point object.
   * @param value The initial floating point value.
   * @param parent Optional parent in the hierarchy.
   * @return Shared pointer to the new NamedFloatingPoint instance.
   */
  static std::shared_ptr<NamedFloatingPoint<T>>
  create(const std::string &name, T value,
         std::shared_ptr<NamedObject> parent = nullptr) {
    // Instantiate using shared_ptr to comply with CS-0010.6.
    std::shared_ptr<NamedFloatingPoint<T>> obj =
        std::make_shared<NamedFloatingPoint<T>>(name, value);

    // Initialize self-reference for getSelf().
    obj->setSelf(obj);

    // Attach to parent if provided.
    if (parent) {
      obj->setParent(parent);
    }
    return obj;
  }

  /**
   * @brief Creates a clone of this NamedFloatingPoint.
   * 
   * Fulfills [FE-0020.14] Utilities for copying parts of the tree.
   * 
   * @param policy Memory policy (DUPLICATE vs SHARE).
   * @return A new NamedFloatingPoint with the same name and value, but no
   * hierarchy.
   */
  std::shared_ptr<NamedObject> clone(CopyPolicy policy = CopyPolicy::DUPLICATE) const override {
    if (policy == CopyPolicy::SHARE && m_bound) {
        std::shared_ptr<NamedFloatingPoint<T>> newObj = create(this->getName(), this->value());
        newObj->bind(m_backingStore.lock(), m_bound_offset);
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
  bool isBound() const override { return m_bound; }
  /** @brief Returns the offset in bytes. */
  std::size_t getBoundOffset() const override { return m_bound_offset; }
  /** @brief Returns the length in bytes. */
  std::size_t getBoundLength() const override { return sizeof(T); }

  /**
   * @brief Binds this floating point to a specific memory offset in a buffer.
   * 
   * Fulfills [TSK-20260311-001.6] Buffer-to-Primitive Binding.
   * 
   * @param buffer The source buffer.
   * @param offset The byte offset.
   * @throws std::out_of_range If offset is invalid.
   */
  void bind(std::shared_ptr<quasar::coretypes::Buffer> buffer, std::size_t offset) {
      if (!buffer) return;
      if (offset + sizeof(T) > buffer->size()) {
          throw std::out_of_range("Binding offset out of range for buffer size");
      }
      m_bound = true;
      m_backingStore = buffer;
      m_bound_offset = offset;
      // Sync local value with buffer
      syncFromBuffer();
  }

  /**
   * @brief Retrieves the value, syncing from buffer if bound.
   * @return The current value.
   */
  T value() const {
      if (m_bound) {
          // Const-cast used to update cache from backing store in logically const operation.
          const_cast<NamedFloatingPoint<T>*>(this)->syncFromBuffer();
      }
      return quasar::coretypes::FloatingPoint<T>::value();
  }

  /**
   * @brief Sets the value, syncing to buffer if bound.
   * @param value The new value.
   */
  void setValue(T value) {
      if (quasar::coretypes::FloatingPoint<T>::value() != value) {
          quasar::coretypes::FloatingPoint<T>::setValue(value);
          if (m_bound) {
              syncToBuffer();
          }
          notifyObservers(getSelf());
      }
  }


  /**
   * @brief Returns the type of the object.
   * @return "NamedFloatingPoint"
   */
  std::string getType() const override { return "NamedFloatingPoint"; }

  /**
   * @brief Constructs a NamedFloatingPoint instance.
   * @param name The name of the object.
   * @param value The initial value.
   */
  NamedFloatingPoint(const std::string &name, T value)
      : NamedObject(name), quasar::coretypes::FloatingPoint<T>(value),
        m_bound(false), m_bound_offset(0), m_backingStore() {
  }

private:
  /** @brief Flag indicating if the value is bound. */
  bool m_bound;
  /** @brief Byte offset in backing buffer. */
  std::size_t m_bound_offset;
  /** @brief Weak reference to backing store. */
  std::weak_ptr<quasar::coretypes::Buffer> m_backingStore;

  /**
   * @brief Synchronizes the local member value from the backing buffer.
   */
  void syncFromBuffer() {
      std::shared_ptr<quasar::coretypes::Buffer> buf = m_backingStore.lock();
      if (buf) {
          T val = 0;
          for (size_t i = 0; i < sizeof(T); ++i) {
              reinterpret_cast<uint8_t*>(&val)[i] = buf->get(m_bound_offset + i);
          }
          quasar::coretypes::FloatingPoint<T>::setValue(val);
      }
  }

  /**
   * @brief Synchronizes the local member value to the backing buffer.
   */
  void syncToBuffer() {
      std::shared_ptr<quasar::coretypes::Buffer> buf = m_backingStore.lock();
      if (buf) {
          T val = quasar::coretypes::FloatingPoint<T>::value();
          for (size_t i = 0; i < sizeof(T); ++i) {
              buf->set(m_bound_offset + i, reinterpret_cast<uint8_t*>(&val)[i]);
          }
      }
  }
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDFLOATINGPOINT_HPP
