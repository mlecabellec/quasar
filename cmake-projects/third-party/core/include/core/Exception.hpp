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

  Smp::String8 GetMessage() const override { return _message.c_str(); }

  // For STL compatibility (catch std::exception)
  virtual const char *what() const noexcept { return _message.c_str(); }

protected:
  std::string _message;
};

} // namespace core
