#ifndef QUASAR_CORETYPES_BUFFERSLICE_HPP
#define QUASAR_CORETYPES_BUFFERSLICE_HPP

#include "quasar/coretypes/Buffer.hpp"
#include "quasar/coretypes/Number.hpp"
#include "quasar/coretypes/String.hpp"
#include <memory>
#include <string>
#include <vector>

namespace quasar::coretypes {

/**
 * @brief A view into a Buffer object.
 *
 * Holds a reference to the underlying Buffer to ensure lifetime safety.
 * (Plan: shared_ptr).
 */
class BufferSlice {
public:
  /**
   * @brief Constructs a new BufferSlice.
   * @param buffer The parent buffer.
   * @param offset The starting offset.
   * @param length The length of the slice.
   */
  BufferSlice(std::shared_ptr<Buffer> buffer, size_t offset, size_t length);

  /**
   * @brief Destructor.
   */
  virtual ~BufferSlice() = default;

  // Accessors
  // Accessors
  /**
   * @brief Returns the size of the slice.
   * @return Size in bytes.
   */
  size_t size() const;

  /**
   * @brief Gets a byte at the specified index relative to the slice.
   * @param index The index.
   * @return The byte value.
   */
  uint8_t get(size_t index) const;

  /**
   * @brief Sets a byte at the specified index relative to the slice.
   * @param index The index.
   * @param value The value to set.
   */
  void set(size_t index, uint8_t value);

  // Conversion
  /**
   * @brief Converts the slice to a std::vector.
   * @return Vector copy of the slice data.
   */
  std::vector<uint8_t> toVector() const;

  /**
   * @brief Converts the slice to a hex string.
   * @return String representation.
   */
  std::string toString() const;

  // Slicing a slice
  // Slicing a slice
  /**
   * @brief Creates a sub-slice from this slice.
   * @param index The start index relative to this slice.
   * @param length The length of the sub-slice.
   * @return A new BufferSlice.
   */
  BufferSlice slice(size_t index, size_t length) const;

  // Concatenation
  // "A slice shall be able to be concatenated with other slices." - returns
  // Buffer? or BufferSlice (new buffer)? Usually concatenation creates a new
  // Buffer. Let's return Buffer (new object). Or return BufferSlice if we
  // implement a CompositeBuffer? Requirement says "concatenated with other
  // slices". "Slices can be created from a concatenation of slices..." implies
  // maybe a chain? "Tree of slices can be created." This implies
  // CompositeBuffer logic. For now, let's implement basic valid buffer creation
  // from concat.
  // Concatenation
  /**
   * @brief Concatenates this slice with another.
   * @param other The other slice.
   * @return A new Buffer containing the concatenated data.
   */
  std::shared_ptr<Buffer> concat(const BufferSlice &other) const;

  // Interaction with parent
  // Interaction with parent
  /**
   * @brief Returns the underlying parent Buffer.
   * @return Shared pointer to parent Buffer.
   */
  std::shared_ptr<Buffer> getParent() const;

  /**
   * @brief Returns the absolute offset of this slice in the parent buffer.
   * @return Offset in bytes.
   */
  size_t getOffset() const;

  // Comparison
  /**
   * @brief Checks equality with another slice.
   * @param other The other slice.
   * @return true if contents are equal.
   */
  bool equals(const BufferSlice &other) const;

  // Serialization (Placeholder)
  // Conversion from/to Number/String
  void fromNumber(const Number &n); // Write number to slice?
                                    // Helper to read number from slice
                                    // ...

private:
  std::shared_ptr<Buffer> buffer_;
  size_t offset_;
  size_t length_;
};

} // namespace quasar::coretypes

#endif // QUASAR_CORETYPES_BUFFERSLICE_HPP
