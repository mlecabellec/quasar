#ifndef SMP_OPERATION_IMPL_H
#define SMP_OPERATION_IMPL_H

#include "Smp/Collection.h"
#include "Smp/IOperation.h"
#include "Smp/Object.h"

namespace Smp {

class Operation : public virtual IOperation, public Object {
public:
  Operation(String8 name, String8 description, IObject *parent, ViewKind view)
      : Object(name, description, parent), view(view),
        parameters("Parameters", "", this), returnParameter(nullptr) {}

  virtual ~Operation() noexcept = default;

  const ParameterCollection *GetParameters() const override {
    return &parameters;
  }

  IParameter *GetParameter(String8 name) const override {
    return parameters.at(name);
  }

  void AddParameter(IParameter *parameter) { parameters.Add(parameter); }

  void SetReturnParameter(IParameter *parameter) {
    returnParameter = parameter;
  }

  IParameter *GetReturnParameter() const override { return returnParameter; }

  ViewKind GetView() const override { return view; }

  IRequest *CreateRequest() override {
    // Returns a request object that can be used to invoke the operation
    return nullptr; // TODO: Implement IRequest and Request concrete class
  }

  void Invoke(IRequest *request) override {
    // Invokes the operation using the provided request
  }

  void DeleteRequest(IRequest *request) override {
    // Deletes a request object previously created by CreateRequest
  }

private:
  ViewKind view;
  Collection<IParameter> parameters;
  IParameter *returnParameter;
};

} // namespace Smp

#endif // SMP_OPERATION_IMPL_H
