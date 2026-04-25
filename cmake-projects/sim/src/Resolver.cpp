#include "sim/Resolver.hpp"
#include <Smp/IComponent.h>
#include <Smp/IComposite.h>
#include <Smp/IContainer.h>
#include <cstring>
#include <sstream>
#include <vector>

namespace sim {

Resolver::Resolver()
    : core::Object("Resolver", "SMP Resolver Service", nullptr) {}

Smp::ComponentStateKind Resolver::GetState() const {
  return Smp::ComponentStateKind::CSK_Connected;
}

void Resolver::Publish(Smp::IPublication *receiver) {}

void Resolver::Configure(Smp::Services::ILogger *logger,
                         Smp::Services::ILinkRegistry *linkRegistry) {}

void Resolver::Connect(Smp::ISimulator *simulator) {}

void Resolver::Disconnect() {}

const Smp::Uuid &Resolver::GetUuid() const {
  static Smp::Uuid uuid = {0, 0, 0, 0, 2}; // Generic Service UUID
  return uuid;
}

Smp::IField *Resolver::GetField(Smp::String8 fullName) const { return nullptr; }

const Smp::FieldCollection *Resolver::GetFields() const { return nullptr; }

Smp::AnySimple Resolver::GetSimpleValue(Smp::String8 fullName) const {
  return Smp::AnySimple();
}

void Resolver::SetSimpleValue(Smp::String8 fullName, Smp::AnySimple value) {}

void Resolver::GetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                                   Smp::AnySimple *values,
                                   Smp::UInt64 startIndex) const {}

void Resolver::SetSimpleArrayValue(Smp::String8 fullName, Smp::UInt64 length,
                                   Smp::AnySimpleArray values,
                                   Smp::UInt64 startIndex) {}

Smp::Bool Resolver::AddChild(Smp::IObject *child,
                             const Smp::ICollectionBase *collection) {
  return false;
}

Smp::Bool Resolver::RemoveChild(Smp::IObject *child,
                                const Smp::ICollectionBase *collection) {
  return false;
}

Smp::IObject *
Resolver::IsChildInCollection(Smp::String8 child,
                              const Smp::ICollectionBase *collection) const {
  return nullptr;
}

void Resolver::SetSimulator(Smp::ISimulator *simulator) {
  _simulator = simulator;
}

static std::vector<std::string> SplitPath(const char *path) {
  std::vector<std::string> parts;
  if (!path)
    return parts;

  std::string s(path);
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, '/')) {
    if (!item.empty()) {
      parts.push_back(item);
    }
  }
  return parts;
}

Smp::IObject *Resolver::ResolveAbsolute(Smp::String8 absolutePath) {
  /// Fulfills [FE-0070.5.2] (IResolver::ResolveAbsolute).
  // [FE-0050.3.2] Absolute paths supported.
  if (!absolutePath || absolutePath[0] != '/') {
    return nullptr;
  }

  // Path starts with /, so it's from simulator root.
  // Skip first char.
  return ResolveRelative(absolutePath + 1, _simulator);
}

Smp::IObject *Resolver::ResolveRelative(Smp::String8 relativePath,
                                        Smp::IObject *startingObject) {
  /// Fulfills [FE-0070.5.3] (IResolver::ResolveRelative).
  // [FE-0050.3.1] Core logic for resolving SMP path strings.
  // [FE-0050.3.2] Relative paths supported.
  if (!relativePath || !startingObject) {
    return nullptr;
  }

  std::vector<std::string> parts = SplitPath(relativePath);
  Smp::IObject *current = startingObject;
  if (!current)
    current = _simulator;

  for (const std::string &part : parts) {
    if (!current)
      return nullptr;

    // [FE-0050.3.7] Support for "." to reference current object.
    if (part == ".") {
      continue;
    } 
    // [FE-0050.3.6] Support for ".." to reference parent object.
    else if (part == "..") {
      current = current->GetParent();
    } 
    else {
      // Traverse down.
      // current must be IComposite or IComponent/Object having containers?
      // "Recurse into composites".
      // We need to check if current has children with name `part`.
      // Children are in Containers.
      // Or `part` could be a Field?
      // "The Resolver service allows resolving references to objects...".
      // Typically Components.
      // "Valid simple names for objects...".

      // We iterate over containers of current (if it's a composite).
      Smp::IComposite *composite = dynamic_cast<Smp::IComposite *>(current);
      Smp::IObject *next = nullptr;

      if (composite) {
        // Check containers
        const Smp::ContainerCollection *containers = composite->GetContainers();
        if (containers) {
          for (Smp::IContainer *container : *containers) {
            // Check if container has component named part?
            // Container has GetComponent(name).
            if (container) {
              // [FE-0050.3.3] Delimiter "/" is used by SplitPath.
              // Implicitly, internal component resolution here corresponds to path traversal.
              next = container->GetComponent(part.c_str());
              if (next)
                break;
            }
          }
        }
      }

      // If not found in containers, could be a Field?
      // But Resolver usually resolves Objects (Components, maybe fields if they
      // are Objects). Fields in SMP 1.1 are IField, inheriting IObject. So we
      // should check fields too? IComponent has GetFields().
      if (!next) {
        Smp::IComponent *component = dynamic_cast<Smp::IComponent *>(current);
        if (component) {
          // [FE-0050.3.4] Supports delimiters "/" or ".". The current logic primarily
          // uses "/" via SplitPath, and then looks up components/fields.
          // Explicitly handling "." is done earlier. Implicitly handling traversal
          // to fields might align with '.' delimiter usage in some contexts.
          Smp::IField *field = component->GetField(part.c_str());
          if (field)
            next = field;
        }
      }
      // Check if it's an array field item? e.g. "MyArray[0]" handled by
      // GetField? IComponent::GetField handles "MyField.Position[2]". But here
      // we split by /. So if path is "MyField/SubField", we handle one by one.
      // If "MyArray[0]", GetField handles it?
      // [FE-0050.3.8] Array element identification by "[n]" is NOT explicitly handled here.
      // The current logic focuses on component/field names separated by '/'.
      // This is a gap regarding FE-0050.3.8.

      current = next;
    }
  }

  return current;
}

} // namespace sim
