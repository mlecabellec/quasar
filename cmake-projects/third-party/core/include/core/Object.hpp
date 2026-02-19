#pragma once

#include <Smp/IObject.h>
#include <string>

namespace core {

/**
 * Base class for core objects.
 */
class Object : public virtual Smp::IObject {
public:
  Object(Smp::String8 name, Smp::String8 description = "",
         Smp::IObject *parent = nullptr)
      : _name(name ? name : ""), _description(description ? description : ""),
        _parent(parent) {}

  virtual ~Object() noexcept = default;

  Smp::String8 GetName() const override { return _name.c_str(); }
  Smp::String8 GetDescription() const override { return _description.c_str(); }
  Smp::IObject *GetParent() const override { return _parent; }

  Smp::IObject *GetChild(Smp::String8 name) const override { return nullptr; }

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

protected:
  std::string _name;
  std::string _description;
  Smp::IObject *_parent;
};

} // namespace core
