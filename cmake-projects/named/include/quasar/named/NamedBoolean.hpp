/**
 * @file NamedBoolean.hpp
 * @brief Class for named boolean values.
 */

#ifndef QUASAR_NAMED_NAMEDBOOLEAN_HPP
#define QUASAR_NAMED_NAMEDBOOLEAN_HPP

#include <optional>
#include "quasar/coretypes/Boolean.hpp"
#include "quasar/coretypes/Buffer.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/IBoundPrimitive.hpp"
#include "quasar/named/CopyPolicy.hpp"

namespace quasar::named {

/**
 * @class NamedBoolean
 * @brief A named object that holds a boolean value.
 * 
 * Inherits from NamedObject for hierarchy and coretypes::Boolean for boolean state.
 * 
 * **Compliance**:
 * - Fulfills [FE-0020.4] Derivative class for Boolean core type.
 * - Fulfills [CS-0010.34] Explicit type declarations.
 */
class NamedBoolean : public NamedObject, public quasar::coretypes::Boolean, public IBoundPrimitive {
public:
  /**
   * @brief Destructor.
   */
  virtual ~NamedBoolean() = default;

  /**
   * @brief Factory method to create a new NamedBoolean.
   * 
   * Fulfills [FE-0020.6] static method "create".
   * 
   * @param name The name of the object.
   * @param value The initial boolean value.
   * @param parent Optional parent in the hierarchy.
   * @return A shared_ptr to the newly created NamedBoolean.
   */
  static std::shared_ptr<NamedBoolean>
  create(const std::string &name, bool value,
         std::shared_ptr<NamedObject> parent = nullptr);

  /**
   * @brief Creates a standalone copy of this NamedBoolean.
   * 
   * Fulfills [FE-0020.14] Utilities for copying parts of the tree.
   * 
   * @param policy Memory policy (DUPLICATE vs SHARE).
   * @return A new NamedBoolean with the same name and value, but no hierarchy.
   */
  std::shared_ptr<NamedObject> clone(CopyPolicy policy = CopyPolicy::DUPLICATE) const override;

  /**
   * @brief Returns whether this object is currently bound to a parent buffer.
   * @return true if bound.
   * @feature [TSK-20260311-001.6] Buffer-to-Primitive Binding.
   */
  bool isBound() const override { return m_bound; }

  /**
   * @brief Returns the offset within the parent buffer.
   * @return The offset in bytes.
   */
  std::size_t getBoundOffset() const override { return m_bound_offset; }

  /**
   * @brief Returns the length of this primitive (1 byte for boolean).
   * @return 1
   */
  std::size_t getBoundLength() const override { return 1; }

  /**
   * @brief Sets the endianness for buffer synchronization.
   * @param endian The endianness to use.
   */
  void setEndianness(quasar::coretypes::Endianness endian) override;

  /**
   * @brief Returns the current endianness used for synchronization.
   * @return The endianness.
   */
  quasar::coretypes::Endianness getEndianness() const override;

  /**
   * @brief Binds this boolean to a specific memory offset in a buffer.
   * 
   * Fulfills [TSK-20260311-001.6] Buffer-to-Primitive Binding.
   * 
   * @param buffer The source buffer.
   * @param offset The byte offset.
   * @param endian Optional endianness (defaults to current setting).
   * @throws std::out_of_range If offset is invalid.
   */
  void bind(std::shared_ptr<quasar::coretypes::Buffer> buffer, std::size_t offset,
            std::optional<quasar::coretypes::Endianness> endian = std::nullopt);

  /**
   * @brief Retrieves the boolean value, syncing from buffer if bound.
   * @return The current value.
   */
  bool booleanValue() const;

  /**
   * @brief Sets the boolean value, syncing to buffer if bound.
   * @param value The new value.
   */
  void setValue(bool value);

  /**
   * @brief Returns the type of the object.
   * @return "NamedBoolean"
   */
  std::string getType() const override;

  /**
   * @brief Constructor for NamedBoolean.
   * @param name Object name.
   * @param value Initial value.
   */
  NamedBoolean(const std::string &name, bool value);

protected:
  /** @brief Flag indicating if the value is bound to a buffer. */
  bool m_bound;
  /** @brief Offset in the backing buffer. */
  std::size_t m_bound_offset;
  /** @brief Weak reference to the backing buffer to avoid cycles. */
  std::weak_ptr<quasar::coretypes::Buffer> m_backingStore;
  /** @brief Endianness for synchronization (metadata only for 1-byte). */
  quasar::coretypes::Endianness m_endian;

  /**
   * @brief Internal helper to sync local state from the backing buffer.
   */
  void syncFromBuffer();

  /**
   * @brief Internal helper to sync local state to the backing buffer.
   */
  void syncToBuffer();
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDBOOLEAN_HPP
