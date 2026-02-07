#include "Smp/Publication/Publication.h"
#include "Smp/ISimpleField.h"
#include "Smp/Publication/TypeRegistry.h"
#include <cassert>
#include <iostream>

int main() {
  // Step: Initialize TypeRegistry
  std::cout << "Step: Initialize TypeRegistry" << std::endl;
  Smp::Publication::TypeRegistry *registry =
      new Smp::Publication::TypeRegistry();

  // Test 1: Primitive types should be registered
  std::cout << "Test 1: Check if primitive types are registered" << std::endl;

  // Assertion: Check PTK_Int32
  std::cout << "Assertion: Check if PTK_Int32 is registered" << std::endl;
  if (registry->GetType(Smp::PrimitiveTypeKind::PTK_Int32) == nullptr) {
    std::cout << "Error: PTK_Int32 not found" << std::endl;
    return 1;
  }

  // Assertion: Check Uuid_Int32
  std::cout << "Assertion: Check if Uuid_Int32 is registered" << std::endl;
  if (registry->GetType(Smp::Uuids::Uuid_Int32) == nullptr) {
    std::cout << "Error: Uuid_Int32 not found" << std::endl;
    return 1;
  }

  std::cout << "Test 1 Passed: Primitive types registered." << std::endl;

  // Test 2: Add custom integer type
  std::cout << "Test 2: Add custom integer type" << std::endl;
  Smp::Uuid myIntUuid = {0x12345678, 0x1234, 0x1234, 0x1234, 0x123456789012ULL};

  // Step: Register custom integer type
  std::cout << "Step: Register custom integer type \"MyInt\"" << std::endl;
  auto myInt = registry->AddIntegerType("MyInt", "Custom integer", myIntUuid, 0,
                                        100, "count");

  // Assertion: Check if registration returned valid pointer
  std::cout << "Assertion: Check if registration returned valid pointer"
            << std::endl;
  if (myInt == nullptr) {
    std::cout << "Error: Failed to add custom integer type" << std::endl;
    return 1;
  }

  // Assertion: Check if registry returns the same pointer for the Uuid
  std::cout
      << "Assertion: Check if registry returns the same pointer for the Uuid"
      << std::endl;
  if (registry->GetType(myIntUuid) != myInt) {
    std::cout << "Error: Registry lookup by Uuid failed" << std::endl;
    return 1;
  }

  std::cout << "Test 2 Passed: Custom integer type added." << std::endl;

  // Test 3: Create Publication and publish field
  std::cout << "Test 3: Create Publication and publish field" << std::endl;

  // Step: Initialize Publication
  std::cout << "Step: Initialize Publication \"MyPub\"" << std::endl;
  Smp::Publication::Publication *publication =
      new Smp::Publication::Publication("MyPub", "Description", nullptr,
                                        registry);

  Smp::Int32 myVar = 42;

  // Step: Publish field "MyVar"
  std::cout << "Step: Publish field \"MyVar\" pointing to local pointer"
            << std::endl;
  publication->PublishField("MyVar", "My variable", &myVar,
                            Smp::ViewKind::VK_All, true, true, false);

  // Assertion: Check number of fields
  std::cout << "Assertion: Check if fields collection size is 1" << std::endl;
  auto fields = publication->GetFields();
  if (fields->size() != 1) {
    std::cout << "Error: Unexpected fields size: " << fields->size()
              << std::endl;
    return 1;
  }

  // Assertion: Check if "MyVar" exists in collection
  std::cout << "Assertion: Check if \"MyVar\" exists in fields collection"
            << std::endl;
  if (fields->at("MyVar") == nullptr) {
    std::cout << "Error: Field \"MyVar\" not found" << std::endl;
    return 1;
  }

  std::cout << "Test 3 Passed: Field published." << std::endl;

  // Test 4: Verify field value access
  std::cout << "Test 4: Verify field value access" << std::endl;

  auto field = fields->at("MyVar");

  // Step: Cast to ISimpleField
  std::cout << "Step: Cast to ISimpleField" << std::endl;
  auto simpleField = dynamic_cast<Smp::ISimpleField *>(field);

  // Assertion: Check if cast successful
  std::cout << "Assertion: Check if cast successful" << std::endl;
  if (simpleField == nullptr) {
    std::cout << "Error: Cast to ISimpleField failed" << std::endl;
    return 1;
  }

  // Assertion: Check initial value (should be 42)
  std::cout << "Assertion: Check if field value is 42" << std::endl;
  if ((Smp::Int32)simpleField->GetValue() != 42) {
    std::cout << "Error: Value mismatch, expected 42" << std::endl;
    return 1;
  }

  // Step: Modify local variable
  std::cout << "Step: Modify local variable myVar to 100" << std::endl;
  myVar = 100;

  // Assertion: Check if field reflects change (pointer access)
  std::cout << "Assertion: Check if field reflects local change (100)"
            << std::endl;
  if ((Smp::Int32)simpleField->GetValue() != 100) {
    std::cout << "Error: Value mismatch, expected 100" << std::endl;
    return 1;
  }

  // Step: Set value via field API
  std::cout << "Step: Set value via field API to 200" << std::endl;
  simpleField->SetValue(
      Smp::AnySimple(Smp::PrimitiveTypeKind::PTK_Int32, Smp::Int32(200)));

  // Assertion: Check if local variable reflected change
  std::cout << "Assertion: Check if local variable myVar is 200" << std::endl;
  if (myVar != 200) {
    std::cout << "Error: Local variable mismatch, expected 200" << std::endl;
    return 1;
  }

  std::cout << "Test 4 Passed: Field value access verified." << std::endl;

  // Test 5: Composite Types
  std::cout << "Test 5: Composite Type Registrations" << std::endl;

  // Step: Register Enumeration
  std::cout << "Step: Register Enumeration \"MyEnum\"" << std::endl;
  Smp::Uuid enumUuid = {0xEA, 0x1, 0x2, 0x3, 0x4ULL};
  auto myEnum = registry->AddEnumerationType("MyEnum", "Desc", enumUuid,
                                             sizeof(Smp::Int32));
  myEnum->AddLiteral("Red", "Color red", 0);
  myEnum->AddLiteral("Green", "Color green", 1);

  // Assertion: Check if Enumeration registered
  if (registry->GetType(enumUuid) != myEnum) {
    std::cout << "Error: Enumeration registration failed" << std::endl;
    return 1;
  }

  // Step: Register Structure
  std::cout << "Step: Register Structure \"MyStruct\"" << std::endl;
  Smp::Uuid structUuid = {0xEB, 0x1, 0x2, 0x3, 0x4ULL};
  auto myStruct = registry->AddStructureType("MyStruct", "Desc", structUuid);
  myStruct->AddField("Field1", "Desc1", Smp::Uuids::Uuid_Int32, 0);

  // Assertion: Check if Structure registered
  if (registry->GetType(structUuid) != myStruct) {
    std::cout << "Error: Structure registration failed" << std::endl;
    return 1;
  }

  // Step: Register Array
  std::cout << "Step: Register Array \"MyArray\"" << std::endl;
  Smp::Uuid arrayUuid = {0xEC, 0x1, 0x2, 0x3, 0x4ULL};
  auto myArray =
      registry->AddArrayType("MyArray", "Desc", arrayUuid,
                             Smp::Uuids::Uuid_Int32, sizeof(Smp::Int32), 10);

  // Assertion: Check if Array registered
  if (registry->GetType(arrayUuid) != myArray) {
    std::cout << "Error: Array registration failed" << std::endl;
    return 1;
  }

  std::cout << "Test 5 Passed: Composite types registered." << std::endl;

  // Step: Cleanup
  std::cout << "Step: Cleaning up" << std::endl;
  delete publication;
  delete registry;

  std::cout << "All Publication tests passed!" << std::endl;

  return 0;
}
