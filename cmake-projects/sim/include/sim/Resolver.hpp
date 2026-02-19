#pragma once

#include <Smp/ISimulator.h>
#include <Smp/Services/IResolver.h>

namespace sim {

class Resolver : public Smp::Services::IResolver {
public:
  Resolver();
  virtual ~Resolver() noexcept = default;

  void SetSimulator(Smp::ISimulator *simulator);

  Smp::IObject *ResolveAbsolute(Smp::String8 absolutePath) const override;
  Smp::IObject *ResolveRelative(Smp::String8 relativePath,
                                const Smp::IObject *relativeTo) const override;

private:
  Smp::ISimulator *_simulator = nullptr;
};

} // namespace sim
