/**
 * @file NamedBufferSlice.hpp
 * @brief Class for named views into byte buffers.
 */

#pragma once

#include "quasar/coretypes/BufferSlice.hpp"
#include "quasar/named/NamedObject.hpp"

namespace quasar::named {

/**
 * @class NamedBufferSlice
 * @brief A named object representing a view (slice) of an existing buffer.
 * 
 * This class inherits from NamedObject for hierarchy management and 
 * coretypes::BufferSlice for buffer view operations.
 */
class NamedBufferSlice : public NamedObject,
                         public quasar::coretypes::BufferSlice {
public:
  /**
   * @brief Factory method to create a new NamedBufferSlice.
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
   * @brief Constructs a NamedBufferSlice instance.
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
   * @return A new NamedBufferSlice pointing to the same buffer region, with the same name, but no hierarchy.
   */
  std::shared_ptr<NamedObject> clone() const override;

  /**
   * @brief Creates a sub-slice view of this slice.
   * @param start The relative starting offset within this slice.
   * @param length The length of the new sub-slice.
   * @return A new NamedBufferSlice that is a view into the same underlying buffer.
   */
  std::shared_ptr<NamedBufferSlice> sliceView(size_t start,
                                              size_t length) const;
};

} // namespace quasar::named
