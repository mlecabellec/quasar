#ifndef SIM_PUBLISHOPERATION_HPP
#define SIM_PUBLISHOPERATION_HPP

#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <string>
#include <vector>

namespace sim {

/**
 * @brief Operation publication implementation.
 * @details Contributes to [FE-0070.9.14] (IPublishOperation interface).
 */
class PublishOperation : public virtual Smp::Publication::IPublishOperation {
public:
  PublishOperation(Smp::String8 name, Smp::String8 description,
                   Smp::Publication::ITypeRegistry *typeRegistry);
  virtual ~PublishOperation() noexcept = default;

  /// [FE-0070.9.15] Publish parameter.
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
