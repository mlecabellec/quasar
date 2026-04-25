/**
 * @file NamedBufferSlice.hpp
 * @brief Class for named views into byte buffers.
 */

#ifndef QUASAR_NAMED_NAMEDBUFFERSLICE_HPP
#define QUASAR_NAMED_NAMEDBUFFERSLICE_HPP

#include "quasar/coretypes/BufferSlice.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/IBoundPrimitive.hpp"
#include "quasar/named/CopyPolicy.hpp"

namespace quasar::named {

class NamedBuffer;

/**
 * @class NamedBufferSlice
 * @brief A named object representing a view (slice) of an existing buffer.
 *
 * This class inherits from NamedObject for hierarchy management and
 * coretypes::BufferSlice for buffer view operations.
 *
 * **Compliance**:
 * - Fulfills [FE-0030.7] Create a NamedBufferSlice class which is a view of a
 * Buffer.
 */
class NamedBufferSlice : public NamedObject,
                         public quasar::coretypes::BufferSlice,
                         public IBoundPrimitive {
public:
  /**
   * @brief Factory method to create a new NamedBufferSlice.
   *
   * Fulfills [FE-0030.7.5] Slices can be created from a Buffer.
   *
   * @param name The name of the slice object.
   * @param buffer The underlying buffer to view.
   * @param start The starting byte offset within the buffer.
   * @param length The number of bytes in the slice.
   * @param parent Optional parent in the hierarchy.
   * @return A shared_ptr to the newly created NamedBufferSlice.
   */
  static std::shared_ptr<NamedBufferSlice>
  create(const std::string &name,
         std::shared_ptr<quasar::coretypes::Buffer> buffer, size_t start,
         size_t length, std::shared_ptr<NamedObject> parent = nullptr);

  /**
   * @brief Factory method to create a new NamedBufferSlice from a NamedBuffer.
   *
   * Fulfills [FE-0030.7.5] Slices can be created from a Buffer.
   * Fulfills [TSK-20260301-001.6] NamedBufferSlice can be constructed from a
   * NamedBuffer.
   *
   * @param name The name of the slice object.
   * @param buffer The underlying NamedBuffer to view.
   * @param start The starting byte offset within the buffer.
   * @param length The number of bytes in the slice.
   * @param parent Optional parent in the hierarchy.
   * @return A shared_ptr to the newly created NamedBufferSlice.
   */
  static std::shared_ptr<NamedBufferSlice>
  create(const std::string &name,
         std::shared_ptr<quasar::named::NamedBuffer> buffer, size_t start,
         size_t length, std::shared_ptr<NamedObject> parent = nullptr);

  /**
   * @brief Factory method to create a new NamedBufferSlice from another
   * NamedBufferSlice.
   *
   * Fulfills [FE-0030.7.6] Slices can be created from a slice.
   * Fulfills [TSK-20260301-001.7] NamedBufferSlice can be constructed from a
   * NamedBufferSlice.
   *
   * @param name The name of the slice object.
   * @param slice The underlying NamedBufferSlice to view.
   * @param start The relative starting byte offset within the slice.
   * @param length The number of bytes in the new slice.
   * @param parent Optional parent in the hierarchy.
   * @return A shared_ptr to the newly created NamedBufferSlice.
   */
  static std::shared_ptr<NamedBufferSlice>
  create(const std::string &name,
         std::shared_ptr<quasar::named::NamedBufferSlice> slice, size_t start,
         size_t length, std::shared_ptr<NamedObject> parent = nullptr);

  /**
   * @brief Constructs a NamedBufferSlice instance.
   *
   * Fulfills [FE-0030.7.1] A slice shall be defined by a starting offset and a
   * length.
   *
   * @param name The name of the object.
   * @param buffer The underlying buffer.
   * @param start Start offset.
   * @param length Length of slice.
   */
  NamedBufferSlice(const std::string &name,
                   std::shared_ptr<quasar::coretypes::Buffer> buffer,
                   size_t start, size_t length);

  /**
   * @brief Virtual destructor.
   */
  virtual ~NamedBufferSlice() = default;

  /**
   * @brief Creates a standalone copy of this slice.
   *
   * Fulfills [FE-0030.7.2] A slice shall be able to be copied to a new Buffer.
   *
   * @param policy Memory policy (DUPLICATE vs SHARE).
   * @return A new NamedBufferSlice pointing to the same buffer region, with the
   * same name, but no hierarchy.
   */
  std::shared_ptr<NamedObject> clone(CopyPolicy policy = CopyPolicy::DUPLICATE) const override;

  // --- IBoundPrimitive implementation ---
  /** @brief Slices are by definition bound to a buffer. */
  bool isBound() const override { return true; }
  /** @brief Returns the start offset. */
  std::size_t getBoundOffset() const override { return getOffset(); }
  /** @brief Returns the slice length. */
  std::size_t getBoundLength() const override { return size(); }
  /** @brief Endianness is handled by higher-level primitives. */
  void setEndianness(quasar::coretypes::Endianness endian) override { (void)endian; }
  /** @brief Returns default BigEndian. */
  quasar::coretypes::Endianness getEndianness() const override { return quasar::coretypes::Endianness::BigEndian; }

  /**
   * @brief Returns the type of the object.
   * @return "NamedBufferSlice"
   */
  std::string getType() const override;

  /**
   * @brief Performs a deep copy, rebasing underlying buffer if it views its
   * parent.
   * @param originalParent Original parent.
   * @param newParent New parent.
   * @param policy Memory policy (DUPLICATE vs SHARE).
   * @return Cloned object.
   * @compliance [FE-0020.14.2] Deep copy rebase slices.
   */
  std::shared_ptr<NamedObject>
  deepCopy(std::shared_ptr<NamedObject> originalParent,
           std::shared_ptr<NamedObject> newParent, CopyPolicy policy = CopyPolicy::DUPLICATE, int maxDepth = 256) const override;

  /**
   * @brief Creates a sub-slice view of this slice.
   *
   * Fulfills [FE-0030.7.6] Slices can be created from a slice.
   *
   * @param start The relative starting offset within this slice.
   * @param length The length of the new sub-slice.
   * @return A new NamedBufferSlice that is a view into the same underlying
   * buffer.
   */
  std::shared_ptr<NamedBufferSlice> sliceView(size_t start,
                                              size_t length) const;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDBUFFERSLICE_HPP

