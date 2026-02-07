#ifndef SMP_INVALIDRETURNVALUE_H
#define SMP_INVALIDRETURNVALUE_H

#include "Smp/AnySimple.h"
#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class InvalidReturnValue : public virtual Exception {
public:
  InvalidReturnValue(Smp::String8 operationName, AnySimple value)
      : Exception("InvalidReturnValue", "Invalid return value for operation",
                  (std::string("Operation '") +
                   (operationName ? operationName : "") +
                   "' returned invalid value")
                      .c_str()),
        operationName(operationName ? operationName : ""), value(value) {}
  virtual ~InvalidReturnValue() noexcept = default;

  virtual String8 GetOperationName() const noexcept {
    return operationName.c_str();
  }
  virtual AnySimple GetValue() const noexcept { return value; }

  const char *what() const noexcept override { return message.c_str(); }

protected:
  std::string operationName;
  AnySimple value;
};
} // namespace Smp

#endif // SMP_INVALIDRETURNVALUE_H
