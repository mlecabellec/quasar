#pragma once

#include <Smp/IComponent.h>
#include <Smp/IContainer.h>
#include <core/Object.hpp>
#include <core/SimpleCollection.hpp>
#include <core/StandardExceptions.hpp>

namespace core {

class Container : public core::Object, public Smp::IContainer {
public:
  Container(Smp::String8 name, Smp::String8 description, Smp::IObject *parent,
            Smp::Int64 lower = 0, Smp::Int64 upper = -1)
      : core::Object(name, description, parent), _lower(lower), _upper(upper) {}

  virtual ~Container() noexcept = default;

  const Smp::ComponentCollection *GetComponents() const override {
    return &_components;
  }

  Smp::IComponent *GetComponent(Smp::String8 name) const override {
    return _components.at(name);
  }

  void AddComponent(Smp::IComponent *component) override {
    if (!component)
      return;

    if (_upper != -1 && static_cast<Smp::Int64>(_components.size()) >= _upper) {
      // throw Smp::ContainerFull(this, _upper); // Need to implement
      // ContainerFull in core
      throw core::Exception("ContainerFull", "Container is full");
    }

    if (_components.at(component->GetName())) {
      throw core::DuplicateName(component->GetName());
    }

    _components.Add(component);
  }

  void DeleteComponent(Smp::IComponent *component) override {
    if (!component)
      return;

    if (!_components.at(component->GetName())) {
      // throw Smp::NotContained(this, component);
      throw core::Exception("NotContained", "Component not contained");
    }

    if (static_cast<Smp::Int64>(_components.size()) <= _lower) {
      // throw Smp::CannotDelete(this, component);
      throw core::Exception("CannotDelete", "Cannot delete below minimum");
    }

    _components.Remove(component);
    // Ownership? Standard says "Delete a contained component from the
    // container, and from memory."
    delete component;
  }

  Smp::Int64 GetCount() const override {
    return static_cast<Smp::Int64>(_components.size());
  }

  Smp::Int64 GetUpper() const override { return _upper; }

  Smp::Int64 GetLower() const override { return _lower; }

private:
  core::SimpleCollection<Smp::IComponent> _components;
  Smp::Int64 _lower;
  Smp::Int64 _upper;
};

} // namespace core
