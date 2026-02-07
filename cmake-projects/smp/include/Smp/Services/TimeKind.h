#ifndef SMP_SERVICES_TIMEKIND_H
#define SMP_SERVICES_TIMEKIND_H

#include "Smp/PrimitiveTypes.h"
#include <iostream>

namespace Smp {
namespace Services {
enum class TimeKind : Int32 {
  TK_SimulationTime = 0,
  TK_MissionTime = 1,
  TK_EpochTime = 2,
  TK_ZuluTime = 3
};

std::ostream &operator<<(std::ostream &os, const TimeKind &obj);
} // namespace Services
} // namespace Smp

#endif // SMP_SERVICES_TIMEKIND_H
