#ifndef QUASAR_NAMED_NAMEDBUFFER_HPP
#define QUASAR_NAMED_NAMEDBUFFER_HPP

#include "quasar/coretypes/Buffer.hpp"
#include "quasar/named/NamedObject.hpp"

namespace quasar::named {

/**
 * @brief A named buffer.
 * Inherits from NamedObject and coretypes::Buffer.
 */
class NamedBuffer : public NamedObject, public quasar::coretypes::Buffer {
public:
  /**
   * @brief Destructor.
   */
  virtual ~NamedBuffer() = default;

  /**
   * @brief Creates a NamedBuffer with given size.
   * @param name The name.
   * @param size The size in bytes.
   * @param parent Optional parent.
   * @return Shared pointer to created NamedBuffer.
   */
  static std::shared_ptr<NamedBuffer>
  create(const std::string &name, size_t size,
         std::shared_ptr<NamedObject> parent = nullptr);

  /**
   * @brief Creates a NamedBuffer with initial data.
   * @param name The name.
   * @param data The initial data.
   * @param parent Optional parent.
   * @return Shared pointer to created NamedBuffer.
   */
  static std::shared_ptr<NamedBuffer>
  create(const std::string &name, const std::vector<uint8_t> &data,
         std::shared_ptr<NamedObject> parent = nullptr);

  std::shared_ptr<NamedObject> clone() const override {
    // Accessing protected data_ from Buffer
    return create(getName(), data_);
  }

  /**
   * @brief Constructs a NamedBuffer.
   * @param name The name.
   * @param size The initial size.
   */
  NamedBuffer(const std::string &name, size_t size);

  /**
   * @brief Constructs a NamedBuffer.
   * @param name The name.
   * @param data The initial data.
   */
  NamedBuffer(const std::string &name, const std::vector<uint8_t> &data);
};

} // namespace quasar::named

#endif // QUASAR_NAMED_NAMEDBUFFER_HPP
