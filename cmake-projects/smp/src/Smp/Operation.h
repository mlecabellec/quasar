#ifndef SMP_OPERATION_IMPL_H
#define SMP_OPERATION_IMPL_H

#include "Smp/Collection.h"
#include "Smp/IOperation.h"
#include "Smp/Object.h"
#include <memory>

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

  void AddParameter(std::unique_ptr<IParameter> parameter) {
    parameters.Add(std::move(parameter));
  }

  void SetReturnParameter(std::unique_ptr<IParameter> parameter) {
    returnParameter = std::move(parameter);
  }

  IParameter *GetReturnParameter() const override {
    return returnParameter.get();
  }

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
  std::unique_ptr<IParameter> returnParameter;
};

} // namespace Smp

#endif // SMP_OPERATION_IMPL_H
