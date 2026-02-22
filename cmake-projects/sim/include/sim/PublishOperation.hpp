#ifndef SIM_PUBLISHOPERATION_HPP
#define SIM_PUBLISHOPERATION_HPP

#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <string>
#include <vector>

namespace sim {

class PublishOperation : public virtual Smp::Publication::IPublishOperation {
public:
  PublishOperation(Smp::String8 name, Smp::String8 description,
                   Smp::Publication::ITypeRegistry *typeRegistry);
  virtual ~PublishOperation() noexcept = default;

  void PublishParameter(
      Smp::String8 name, Smp::String8 description, Smp::Uuid typeUuid,
      Smp::Publication::ParameterDirectionKind direction =
          Smp::Publication::ParameterDirectionKind::PDK_In) override;

private:
  std::string _name;
  std::string _description;
  Smp::Publication::ITypeRegistry *_typeRegistry;
};

} // namespace sim

#endif // SIM_PUBLISHOPERATION_HPP
