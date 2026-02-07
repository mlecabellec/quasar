#ifndef SMP_PUBLICATION_PARAMETERDIRECTIONKIND_H
#define SMP_PUBLICATION_PARAMETERDIRECTIONKIND_H

#include "Smp/PrimitiveTypes.h"
#include <iostream>

namespace Smp {
namespace Publication {
enum class ParameterDirectionKind : Int32 {
  PDK_In = 0,
  PDK_Out = 1,
  PDK_InOut = 2,
  PDK_Return = 3
};

std::ostream &operator<<(std::ostream &os, const ParameterDirectionKind &obj);
} // namespace Publication
} // namespace Smp

#endif // SMP_PUBLICATION_PARAMETERDIRECTIONKIND_H
