#include <sim/PublishOperation.hpp>

namespace sim {

PublishOperation::PublishOperation(
    Smp::String8 name, Smp::String8 description,
    Smp::Publication::ITypeRegistry *typeRegistry)
    : _name(name), _description(description), _typeRegistry(typeRegistry) {}

void PublishOperation::PublishParameter(
    Smp::String8 name, Smp::String8 description, Smp::Uuid typeUuid,
    Smp::Publication::ParameterDirectionKind direction) {
  // Stub for parameter publication
}

} // namespace sim
