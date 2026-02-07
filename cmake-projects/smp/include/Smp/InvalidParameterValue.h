#ifndef SMP_INVALIDPARAMETERVALUE_H
#define SMP_INVALIDPARAMETERVALUE_H

#include "Smp/AnySimple.h"
#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
class InvalidParameterValue : public virtual Exception {
public:
  InvalidParameterValue(Smp::String8 parameterName, AnySimple value)
      : Exception("InvalidParameterValue", "Invalid value for parameter",
                  (std::string("Parameter '") +
                   (parameterName ? parameterName : "") +
                   "' assigned invalid value")
                      .c_str()),
        parameterName(parameterName ? parameterName : ""), value(value) {}
  virtual ~InvalidParameterValue() noexcept = default;

  virtual String8 GetParameterName() const noexcept {
    return parameterName.c_str();
  }
  virtual AnySimple GetValue() const noexcept { return value; }

  const char *what() const noexcept override { return message.c_str(); }

protected:
  std::string parameterName;
  AnySimple value;
};
} // namespace Smp

#endif // SMP_INVALIDPARAMETERVALUE_H
