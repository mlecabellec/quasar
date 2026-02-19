#pragma once

#include "core/BitBuffer.hpp"
#include "core/TypeTraits.hpp"
#include <cstdint>
#include <cstring>

namespace core {

/**
 * Typed wrapper around a BitBuffer.
 */
template <typename T> class Value : public BitBuffer {
public:
  /**
   * Constructor: Creates a new value.
   */
  explicit Value() : BitBuffer(sizeof(T)) {}

  /**
   * Constructor: Creates a new value initialized with given value.
   */
  explicit Value(const T &val) : BitBuffer(sizeof(T)) { set(val); }

  /**
   * Constructor: Wraps an existing pointer.
   */
  explicit Value(T *ptr)
      : BitBuffer(reinterpret_cast<uint8_t *>(ptr), sizeof(T)) {}

  /**
   * Set value.
   */
  void set(const T &val) {
    // Simple memcpy for POD types, care needed for complex types
    // For this core library, we assume usage with POD-like SMP types
    copyFrom(&val, sizeof(T));
  }

  /**
   * Get value.
   */
  T get() const {
    T val;
    std::memcpy(&val, this->_data, sizeof(T));
    return val;
  }

  /**
   * Assignment operator.
   */
  Value<T> &operator=(const T &val) {
    set(val);
    return *this;
  }

  /**
   * Cast operator.
   */
  operator T() const { return get(); }
};

} // namespace core
