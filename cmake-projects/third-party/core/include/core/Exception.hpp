#pragma once

#include <Smp/Exception.h>
#include <core/Object.hpp>
#include <string>

namespace core {

class Exception : public core::Object, public virtual Smp::Exception {
public:
  Exception(Smp::String8 name, Smp::String8 description,
            Smp::IObject *sender = nullptr, Smp::String8 message = "")
      : core::Object(name, description, sender),
        _message(message ? message : "") {}

  virtual ~Exception() noexcept = default;

  Smp::String8 GetName() const noexcept override {
    return core::Object::GetName();
  }
  Smp::String8 GetDescription() const noexcept override {
    return core::Object::GetDescription();
  }
  Smp::String8 GetMessage() const noexcept override { return _message.c_str(); }

  const Smp::IObject *GetSender() const noexcept override {
    return GetParent();
  }

  // For STL compatibility (catch std::exception)
  virtual const char *what() const noexcept override {
    return _message.c_str();
  }

protected:
  std::string _message;
};

} // namespace core
