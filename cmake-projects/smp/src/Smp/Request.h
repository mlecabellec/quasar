#ifndef SMP_REQUEST_H
#define SMP_REQUEST_H

#include "Smp/IRequest.h"
#include <map>
#include <string>
#include <vector>

namespace Smp {

class Request : public virtual IRequest {
public:
  Request(String8 operationName);
  virtual ~Request() noexcept = default;

  String8 GetOperationName() const override;
  Int32 GetParameterCount() const override;
  Int32 GetParameterIndex(String8 parameterName) const override;
  void SetParameterValue(Int32 index, AnySimple value) override;
  AnySimple GetParameterValue(Int32 index) const override;
  void SetReturnValue(AnySimple value) override;
  AnySimple GetReturnValue() const override;

  // Helper for implementation
  void AddParameter(String8 name, String8 description, PrimitiveTypeKind kind);
  void SetReturnKind(PrimitiveTypeKind kind);

private:
  struct ParameterInfo {
    std::string name;
    PrimitiveTypeKind kind;
    AnySimple value;
  };

  std::string operationName;
  std::vector<ParameterInfo> parameters;
  std::map<std::string, Int32> parameterMap;
  AnySimple returnValue;
  PrimitiveTypeKind returnKind;
  bool isVoid;
};

} // namespace Smp

#endif // SMP_REQUEST_H
