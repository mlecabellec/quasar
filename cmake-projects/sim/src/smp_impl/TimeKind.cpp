#include "Smp/Services/TimeKind.h"
#include <iostream>

namespace Smp {
namespace Services {
std::ostream &operator<<(std::ostream &os, const TimeKind &obj) {
  switch (obj) {
  case TimeKind::TK_SimulationTime:
    os << "TK_SimulationTime";
    break;
  case TimeKind::TK_MissionTime:
    os << "TK_MissionTime";
    break;
  case TimeKind::TK_EpochTime:
    os << "TK_EpochTime";
    break;
  case TimeKind::TK_ZuluTime:
    os << "TK_ZuluTime";
    break;
  default:
    os << "Unknown TimeKind (" << static_cast<int>(obj) << ")";
    break;
  }
  return os;
}
} // namespace Services
} // namespace Smp
