#include "Smp/PrimitiveTypes.h"
#include "Smp/Uuid.h"
#include <cassert>
#include <iostream>

int main() {
  // Step: Initialize Uuids
  std::cout << "Step: Initialize Uuids" << std::endl;
  Smp::Uuid uuid1 = Smp::Uuids::Uuid_Uuid;
  Smp::Uuid uuid2 = Smp::Uuids::Uuid_Void;

  // Step: Output Uuids
  std::cout << "Step: Output Uuids" << std::endl;
  std::cout << "Uuid_Uuid: " << uuid1 << std::endl;
  std::cout << "Uuid_Void: " << uuid2 << std::endl;

  // Assertion: Check if uuid1 != uuid2
  std::cout << "Assertion: Check if uuid1 is not equal to uuid2" << std::endl;
  if (!(uuid1 != uuid2)) {
    std::cout << "Error: Uuid_Uuid and Uuid_Void should be different"
              << std::endl;
    return 1;
  }

  // Assertion: Check if uuid1 is Uuid_Uuid
  std::cout << "Assertion: Check if uuid1 matches Smp::Uuids::Uuid_Uuid"
            << std::endl;
  if (!(uuid1 == Smp::Uuids::Uuid_Uuid)) {
    std::cout << "Error: uuid1 does not match Uuid_Uuid" << std::endl;
    return 1;
  }

  // Step: Initialize PrimitiveTypeKind
  std::cout << "Step: Initialize PrimitiveTypeKind" << std::endl;
  Smp::PrimitiveTypeKind kind = Smp::PrimitiveTypeKind::PTK_Int32;

  // Step: Output kind
  std::cout << "Step: Output kind" << std::endl;
  std::cout << "Kind: " << kind << std::endl;

  std::cout << "All Structure tests passed!" << std::endl;
  return 0;
}
