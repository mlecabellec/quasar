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
 * 
 * **Compliance**:
 * - Fulfills [FE-0030.6] support for a named Buffer.
 * - Fulfills [FE-0020.4] Derivative of NamedObject.
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
   * Fulfills [FE-0020.6] static method "create".
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
   * Fulfills [FE-0020.6] static method "create".
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
   * 
   * Fulfills [FE-0020.14] Utilities for copying parts of the tree.
   * 
   * @return A new NamedBuffer with the same name and content, but no hierarchy.
   */
  std::shared_ptr<NamedObject> clone() const override {
    // Accessing protected data_ from Buffer base class.
    return create(getName(), data_);
  }

  /**
   * @brief Returns the type of the object.
   * @return "NamedBuffer"
   */
  std::string getType() const override;

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

