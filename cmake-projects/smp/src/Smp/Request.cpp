#include "Smp/Request.h"
#include "Smp/InvalidParameterIndex.h"

namespace Smp {

Request::Request(String8 operationName)
    : operationName(operationName ? operationName : ""),
      returnValue(PrimitiveTypeKind::PTK_None),
      returnKind(PrimitiveTypeKind::PTK_None), isVoid(true) {}

String8 Request::GetOperationName() const { return operationName.c_str(); }

Int32 Request::GetParameterCount() const {
  return static_cast<Int32>(parameters.size());
}

Int32 Request::GetParameterIndex(String8 parameterName) const {
  auto it = parameterMap.find(parameterName ? parameterName : "");
  if (it != parameterMap.end())
    return it->second;
  return -1;
}

void Request::SetParameterValue(Int32 index, AnySimple value) {
  if (index < 0 || static_cast<size_t>(index) >= parameters.size())
    throw InvalidParameterIndex(index, static_cast<Int32>(parameters.size()),
                                operationName.c_str());

  // In a full implementation, check if value kind matches expected kind
  parameters[index].value = value;
}

AnySimple Request::GetParameterValue(Int32 index) const {
  if (index < 0 || static_cast<size_t>(index) >= parameters.size())
    throw InvalidParameterIndex(index, static_cast<Int32>(parameters.size()),
                                operationName.c_str());

  return parameters[index].value;
}

void Request::SetReturnValue(AnySimple value) {
  if (isVoid)
    throw VoidOperation();

  returnValue = value;
}

AnySimple Request::GetReturnValue() const {
  if (isVoid)
    throw VoidOperation();

  return returnValue;
}

void Request::AddParameter(String8 name, String8 description,
                           PrimitiveTypeKind kind) {
  Int32 index = static_cast<Int32>(parameters.size());
  parameters.push_back({name ? name : "", kind, AnySimple(kind)});
  parameterMap[name ? name : ""] = index;
}

void Request::SetReturnKind(PrimitiveTypeKind kind) {
  returnKind = kind;
  isVoid = (kind == PrimitiveTypeKind::PTK_None);
  if (!isVoid) {
    returnValue = AnySimple(kind);
  }
}

} // namespace Smp
