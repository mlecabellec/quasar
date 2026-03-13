#ifndef SMP_MODEL_HPP
#define SMP_MODEL_HPP

#include <Smp/IModel.h>
#include <Smp/IComposite.h>
#include <Smp/ISimulator.h>
#include <Smp/ComponentStateKind.h>
#include <Smp/ContainerCollection.h>
#include <Smp/FieldCollection.h>
#include "common/SimpleCollection.hpp"
#include <string>

namespace sample {

/**
 * @brief Base class for SMP models that do not depend on the sim library.
 */
class SmpModel : public virtual Smp::IModel, public virtual Smp::IComposite {
public:
    SmpModel(Smp::String8 name, Smp::String8 description, Smp::IObject* parent);
    virtual ~SmpModel() noexcept = default;

    // IObject methods
    Smp::String8 GetName() const override;
    Smp::String8 GetDescription() const override;
    Smp::IObject* GetParent() const override;
    Smp::IObject* GetChild(Smp::String8 name) const override;

    // IComposite methods
    const Smp::ContainerCollection* GetContainers() const override;
    Smp::IContainer* GetContainer(Smp::String8 name) const override;

    // IComponent methods
    Smp::ComponentStateKind GetState() const override;
    void Publish(Smp::IPublication* receiver) override;
    void Configure(Smp::Services::ILogger* logger, Smp::Services::ILinkRegistry* linkRegistry = nullptr) override;
    void Connect(Smp::ISimulator* simulator) override;
    void Disconnect() override;
    
    Smp::IField* GetField(Smp::String8 fullName) const override;
    const Smp::FieldCollection* GetFields() const override;
    
    // Abstract, as the child needs to provide its UUID
    const Smp::Uuid& GetUuid() const override = 0;

    Smp::AnySimple GetSimpleValue(Smp::String8 fullName) const override;
    void SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) override;
    void GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length, Smp::AnySimple* values, Smp::UInt64 startIndex = 0) const override;
    void SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length, Smp::AnySimpleArray values, Smp::UInt64 startIndex = 0) override;

    Smp::Bool AddChild(Smp::IObject* child, const Smp::ICollectionBase* collection) override;
    Smp::Bool RemoveChild(Smp::IObject* child, const Smp::ICollectionBase* collection) override;
    Smp::IObject* IsChildInCollection(Smp::String8 child, const Smp::ICollectionBase* collection) const override;

protected:
    std::string _name;
    std::string _description;
    Smp::IObject* _parent;
    Smp::ISimulator* _simulator;
    Smp::ComponentStateKind _state;

    // Ordered collections using the simple implementation
    SimpleCollection<Smp::IContainer> _containers;
    SimpleCollection<Smp::IField> _fields;
};

} // namespace sample

#endif // SMP_MODEL_HPP
