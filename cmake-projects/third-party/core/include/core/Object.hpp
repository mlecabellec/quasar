#pragma once

namespace core {

/**
 * Base class for core objects.
 */
class Object {
public:
  /**
   * Default constructor.
   */
  Object() = default;

  /**
   * Destructor.
   */
  virtual ~Object() = default;

  /**
   * Helper to cast this object to a derived type.
   * @tparam T The target type.
   * @return Pointer to the target type or nullptr if cast fails.
   */
  template <typename T> T *cast() { return dynamic_cast<T *>(this); }

  /**
   * Helper to cast this object to a derived type (const version).
   * @tparam T The target type.
   * @return Const pointer to the target type or nullptr if cast fails.
   */
  template <typename T> const T *cast() const {
    return dynamic_cast<const T *>(this);
  }

  /**
   * Check if this object is an instance of type T.
   * @tparam T The target type.
   * @return True if this object can be cast to T.
   */
  template <typename T> bool isInstanceOf() const {
    return cast<T>() != nullptr;
  }
};

} // namespace core
