#ifndef SMP_INVALIDPARAMETERINDEX_H
#define SMP_INVALIDPARAMETERINDEX_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"
#include <string> // Required for std::string and std::to_string

namespace Smp {
class InvalidParameterIndex : public virtual Exception {
public:
  InvalidParameterIndex(Smp::Int32 index, Smp::Int32 count,
                        Smp::String8 operationName = "")
      : Exception("InvalidParameterIndex", "Parameter index out of range",
                  (std::string("Index '") + std::to_string(index) +
                   "' out of '" + std::to_string(count) + "'")
                      .c_str()),
        index(index), count(count),
        operationName(operationName ? operationName : "") {}
  virtual ~InvalidParameterIndex() noexcept = default;

  virtual String8 GetOperationName() const noexcept {
    return operationName.c_str();
  }
  virtual Int32 GetParameterIndex() const noexcept { return index; }
  virtual Int32 GetParameterCount() const noexcept { return count; }

  const char *what() const noexcept override { return message.c_str(); }

protected:
  Int32 index;
  Int32 count;
  std::string operationName;
};
} // namespace Smp

#endif // SMP_INVALIDPARAMETERINDEX_H
