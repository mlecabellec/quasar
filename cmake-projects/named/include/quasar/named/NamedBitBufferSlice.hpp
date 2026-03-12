/**
 * @file NamedBitBufferSlice.hpp
 * @brief Class for named views into bit-addressable buffers.
 */

#ifndef QUASAR_NAMED_NAMEDBITBUFFERSLICE_HPP
#define QUASAR_NAMED_NAMEDBITBUFFERSLICE_HPP

#include "quasar/coretypes/BitBufferSlice.hpp"
#include "quasar/named/NamedObject.hpp"
#include "quasar/named/CopyPolicy.hpp"

namespace quasar::named {

class NamedBitBuffer;

/**
 * @class NamedBitBufferSlice
 * @brief A named object representing a view (slice) of an existing bit buffer.
 *
 * This class inherits from NamedObject for hierarchy management and
 * coretypes::BitBufferSlice for bit-level view operations.
 *
 * **Compliance**:
 * - Fulfills [FE-0030.7] Create a NamedBitBufferSlice class which is a view of
 * a BitBuffer.
 */
class NamedBitBufferSlice : public NamedObject,
                            public quasar::coretypes::BitBufferSlice {
public:
  /**
   * @brief Factory method to create a new NamedBitBufferSlice.
   *
   * Fulfills [FE-0030.7.5] Slices can be created from a BitBuffer.
   *
   * @param name The name of the slice object.
   * @param buffer The underlying bit buffer to view.
   * @param startBit The starting bit offset within the buffer.
   * @param bitLength The number of bits in the slice.
   * @param parent Optional parent in the hierarchy.
   * @return A shared_ptr to the newly created NamedBitBufferSlice.
   */
  static std::shared_ptr<NamedBitBufferSlice>
  create(const std::string &name,
         std::shared_ptr<quasar::coretypes::BitBuffer> buffer, size_t startBit,
         size_t bitLength, std::shared_ptr<NamedObject> parent = nullptr);

  /**
   * @brief Factory method to create a new NamedBitBufferSlice from a
   * NamedBitBuffer.
   *
   * Fulfills [FE-0030.7.5] Slices can be created from a BitBuffer.
   * Fulfills [TSK-20260301-001.9] NamedBitBufferSlice can be constructed from a
   * NamedBitBuffer.
   *
   * @param name The name of the slice object.
   * @param buffer The underlying NamedBitBuffer to view.
   * @param startBit The starting bit offset within the buffer.
   * @param bitLength The number of bits in the slice.
   * @param parent Optional parent in the hierarchy.
   * @return A shared_ptr to the newly created NamedBitBufferSlice.
   */
  static std::shared_ptr<NamedBitBufferSlice>
  create(const std::string &name,
         std::shared_ptr<quasar::named::NamedBitBuffer> buffer, size_t startBit,
         size_t bitLength, std::shared_ptr<NamedObject> parent = nullptr);

  /**
   * @brief Factory method to create a new NamedBitBufferSlice from another
   * NamedBitBufferSlice.
   *
   * Fulfills [FE-0030.7.6] Slices can be created from a slice.
   * Fulfills [TSK-20260301-001.9] NamedBitBufferSlice can be constructed from a
   * NamedBitBufferSlice.
   *
   * @param name The name of the slice object.
   * @param slice The underlying NamedBitBufferSlice to view.
   * @param startBit The relative starting bit offset within the slice.
   * @param bitLength The number of bits in the new slice.
   * @param parent Optional parent in the hierarchy.
   * @return A shared_ptr to the newly created NamedBitBufferSlice.
   */
  static std::shared_ptr<NamedBitBufferSlice>
  create(const std::string &name,
         std::shared_ptr<quasar::named::NamedBitBufferSlice> slice,
         size_t startBit, size_t bitLength,
         std::shared_ptr<NamedObject> parent = nullptr);

  /**
   * @brief Constructs a NamedBitBufferSlice instance.
   *
   * Fulfills [FE-0030.7.1] A slice shall be defined by a starting offset and a
   * length.
   *
   * @param name The name of the object.
   * @param buffer The underlying bit buffer.
   * @param startBit Starting bit index.
   * @param bitLength Number of bits in the slice.
   */
  NamedBitBufferSlice(const std::string &name,
                      std::shared_ptr<quasar::coretypes::BitBuffer> buffer,
                      size_t startBit, size_t bitLength);

  /**
   * @brief Virtual destructor.
   */
  virtual ~NamedBitBufferSlice() = default;

  /**
   * @brief Creates a standalone copy of this slice.
   *
   * Fulfills [FE-0030.7.2] A slice shall be able to be copied to a new
   * BitBuffer.
   *
   * @param policy Memory policy (DUPLICATE vs SHARE).
   * @return A new NamedBitBufferSlice pointing to the same bit buffer region,
   * with the same name, but no hierarchy.
   */
  std::shared_ptr<NamedObject> clone(CopyPolicy policy = CopyPolicy::DUPLICATE) const override;

  /**
   * @brief Returns the type of the object.
   * @return "NamedBitBufferSlice"
   */
  std::string getType() const override;

  /**
   * @brief Performs a deep copy, rebasing underlying bit buffer if it views its
   * parent.
   * @param originalParent Original parent.
   * @param newParent New parent.
   * @param policy Memory policy (DUPLICATE vs SHARE).
   * @return Cloned object.
   * @compliance [FE-0020.14.2] Deep copy rebase slices.
   */
  std::shared_ptr<NamedObject>
  deepCopy(std::shared_ptr<NamedObject> originalParent,
           std::shared_ptr<NamedObject> newParent, CopyPolicy policy = CopyPolicy::DUPLICATE) const override;

  /**
   * @brief Creates a sub-slice view of this bit slice.
   *
   * Fulfills [FE-0030.7.6] Slices can be created from a slice.
   *
   * @param startBit The relative starting bit offset within this slice.
   * @param bitLength The length of the new sub-slice in bits.
   * @return A new NamedBitBufferSlice that is a view into the same underlying
   * bit buffer.
   */
  std::shared_ptr<NamedBitBufferSlice> sliceView(size_t startBit,
                                                 size_t bitLength) const;
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDBITBUFFERSLICE_HPP

