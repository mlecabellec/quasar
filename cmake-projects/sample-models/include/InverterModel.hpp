#ifndef INVERTER_MODEL_HPP
#define INVERTER_MODEL_HPP

#include <sim/Model.hpp>
#include <Smp/IEntryPoint.h>

namespace sample {

class InverterModel : public sim::Model {
public:
  InverterModel(Smp::String8 name, Smp::String8 description,
                Smp::IComposite *parent);
  virtual ~InverterModel() noexcept override = default;

  // IComponent methods
  void Publish(Smp::IPublication *receiver) override;
  void Configure(Smp::Services::ILogger *logger,
                 Smp::Services::ILinkRegistry *linkRegistry = nullptr) override;
  void Connect(Smp::ISimulator *simulator) override;

  const Smp::Uuid &GetUuid() const override;

  Smp::AnySimple GetSimpleValue(Smp::String8 fullName) const override;
  void SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) override;

  Smp::IObject *GetChild(Smp::String8 name) const override;

  // Entry point for execution
  void Execute();

private:
  class ExecuteEntryPoint : public virtual Smp::IEntryPoint {
  public:
    ExecuteEntryPoint(InverterModel *model) : _model(model) {}
    Smp::String8 GetName() const override { return "Execute"; }
    Smp::String8 GetDescription() const override { return "Execute Inverter"; }
    Smp::IObject *GetParent() const override { return _model; }
    Smp::IObject *GetChild(Smp::String8 name) const override { return nullptr; }
    void Execute() const override { _model->Execute(); }
  private:
    InverterModel *_model;
  };

  Smp::Bool _input;
  Smp::Bool _output;
  static const Smp::Uuid _uuid;
  ExecuteEntryPoint _executeEntryPoint;
};

} // namespace sample

#endif // INVERTER_MODEL_HPP
