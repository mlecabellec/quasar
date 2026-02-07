#ifndef SMP_SERVICES_INVALIDSIMULATIONTIME_H
#define SMP_SERVICES_INVALIDSIMULATIONTIME_H

#include "Smp/Exception.h"
#include "Smp/PrimitiveTypes.h"

namespace Smp {
namespace Services {
class InvalidSimulationTime : public virtual Exception {
public:
  virtual ~InvalidSimulationTime() noexcept = default;

  virtual Duration GetCurrentTime() const noexcept = 0;
  virtual Duration GetProvidedTime() const noexcept = 0;
  virtual Duration GetMaximumTime() const noexcept = 0;
};
} // namespace Services
} // namespace Smp

#endif // SMP_SERVICES_INVALIDSIMULATIONTIME_H
