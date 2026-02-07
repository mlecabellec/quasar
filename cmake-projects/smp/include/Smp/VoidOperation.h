#ifndef SMP_VOIDOPERATION_H
#define SMP_VOIDOPERATION_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class VoidOperation : public virtual Smp::Exception {
public:
  VoidOperation(Smp::String8 operationName = "")
      : Exception("VoidOperation",
                  "Attempted to use return value of a void operation",
                  (std::string("Operation '") +
                   (operationName ? operationName : "") + "' is void")
                      .c_str()),
        operationName(operationName ? operationName : "") {}
  virtual ~VoidOperation() noexcept = default;

  virtual Smp::String8 GetOperationName() const noexcept {
    return operationName.c_str();
  }

  const char *what() const noexcept override { return message.c_str(); }

protected:
  std::string operationName;
};
} // namespace Smp

#endif // SMP_VOIDOPERATION_H
