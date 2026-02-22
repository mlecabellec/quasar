#ifndef INVERTER_MODEL_HPP
#define INVERTER_MODEL_HPP

#include <Smp/IComposite.h>
#include <Smp/IModel.h>
#include <Smp/Services/ILinkRegistry.h>
#include <Smp/Services/ILogger.h>
#include <core/Object.hpp>

namespace sample {

class InverterModel : public virtual Smp::IModel, public virtual core::Object {
public:
  InverterModel(Smp::String8 name, Smp::String8 description,
                Smp::IComposite *parent);
  virtual ~InverterModel() noexcept = default;

  // IComponent methods
  Smp::ComponentStateKind GetState() const override;
  void Publish(Smp::IPublication *receiver) override;
  void Configure(Smp::Services::ILogger *logger,
                 Smp::Services::ILinkRegistry *linkRegistry = nullptr) override;
  void Connect(Smp::ISimulator *simulator) override;
  void Disconnect() override;

  Smp::IField *GetField(Smp::String8 fullName) const override;
  const Smp::FieldCollection *GetFields() const override;
  const Smp::Uuid &GetUuid() const override;

  Smp::AnySimple GetSimpleValue(Smp::String8 fullName) const override;
  void SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) override;
  void GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimple *values,
                           Smp::UInt64 startIndex = 0) const override;
  void SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                           Smp::AnySimpleArray values,
                           Smp::UInt64 startIndex = 0) override;

  Smp::Bool AddChild(Smp::IObject *child,
                     const Smp::IObject *collection) override;
  Smp::Bool RemoveChild(Smp::IObject *child,
                        const Smp::IObject *collection) override;
  Smp::IObject *
  IsChildInCollection(Smp::String8 child,
                      const Smp::IObject *collection) const override;

  // Entry point for execution
  void Execute();

private:
  Smp::ComponentStateKind _state;
  Smp::Bool _input;
  Smp::Bool _output;
  static const Smp::Uuid _uuid;
};

} // namespace sample

#endif // INVERTER_MODEL_HPP
