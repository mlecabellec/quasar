#ifndef SMP_EXCEPTION_H
#define SMP_EXCEPTION_H

#include "Smp/IObject.h"
#include <exception>
#include <string>

namespace Smp {
/// Base class for all SMP exceptions.
class Exception : public virtual std::exception {
public:
  Exception() : sender(nullptr) {}
  Exception(String8 name, String8 description, String8 message,
            const IObject *sender = nullptr)
      : name(name ? name : ""), description(description ? description : ""),
        message(message ? message : ""), sender(sender) {}
  virtual ~Exception() noexcept = default;

  const char *what() const noexcept override { return message.c_str(); }
  virtual String8 GetName() const noexcept { return name.c_str(); }
  virtual String8 GetDescription() const noexcept {
    return description.c_str();
  }
  virtual String8 GetMessage() const noexcept { return message.c_str(); }
  virtual const IObject *GetSender() const noexcept { return sender; }

protected:
  std::string name;
  std::string description;
  std::string message;
  const IObject *sender;
};
} // namespace Smp

#endif // SMP_EXCEPTION_H
