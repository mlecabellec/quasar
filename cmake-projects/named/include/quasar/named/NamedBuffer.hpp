/**
 * @file NamedBuffer.hpp
 * @brief Class for named byte buffers.
 */

#ifndef QUASAR_NAMED_NAMEDBUFFER_HPP
#define QUASAR_NAMED_NAMEDBUFFER_HPP

#include "quasar/coretypes/Buffer.hpp"
#include "quasar/named/NamedObject.hpp"

namespace quasar::named {

/**
 * @class NamedBuffer
 * @brief A named object that manages a contiguous byte buffer.
 * 
 * Inherits from NamedObject for hierarchy and coretypes::Buffer for buffer operations.
 */
class NamedBuffer : public NamedObject, public quasar::coretypes::Buffer {
public:
  /**
   * @brief Destructor.
   */
  virtual ~NamedBuffer() = default;

  /**
   * @brief Factory method to create a NamedBuffer with a specific size.
   * 
   * @param name The name of the buffer object.
   * @param size The initial size of the buffer in bytes.
   * @param parent Optional parent in the hierarchy.
   * @return A shared_ptr to the newly created NamedBuffer.
   */
  static std::shared_ptr<NamedBuffer>
  create(const std::string &name, size_t size,
         std::shared_ptr<NamedObject> parent = nullptr);

  /**
   * @brief Factory method to create a NamedBuffer with initial data.
   * 
   * @param name The name of the buffer object.
   * @param data A vector of bytes to initialize the buffer content.
   * @param parent Optional parent in the hierarchy.
   * @return A shared_ptr to the newly created NamedBuffer.
   */
  static std::shared_ptr<NamedBuffer>
  create(const std::string &name, const std::vector<uint8_t> &data,
         std::shared_ptr<NamedObject> parent = nullptr);

  /**
   * @brief Creates a standalone copy of this NamedBuffer.
   * @return A new NamedBuffer with the same name and content, but no hierarchy.
   */
  std::shared_ptr<NamedObject> clone() const override {
    // Accessing protected data_ from Buffer base class.
    return create(getName(), data_);
  }

  /**
   * @brief Constructs a NamedBuffer instance with a given size.
   * @param name The name of the object.
   * @param size The initial buffer size.
   */
  NamedBuffer(const std::string &name, size_t size);

  /**
   * @brief Constructs a NamedBuffer instance with initial data.
   * @param name The name of the object.
   * @param data Initial buffer content.
   */
  NamedBuffer(const std::string &name, const std::vector<uint8_t> &data);
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDBUFFER_HPP
