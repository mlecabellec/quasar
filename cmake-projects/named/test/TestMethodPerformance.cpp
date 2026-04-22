#include <gtest/gtest.h>
#include "quasar/named/NamedMethod.hpp"
#include "quasar/named/TypedNamedMethod.hpp"
#include "datacodec/Schema.hpp"
#include <chrono>
#include <iostream>

using namespace quasar::named;
using namespace datacodec;

class MethodPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

/**
 * @brief Benchmark comparing standard NamedMethod vs TypedNamedMethod call overhead.
 */
TEST_F(MethodPerformanceTest, CallOverheadComparison) {
    const int iterations = 1000000;
    
    // 1. Create a standard NamedMethod
    std::shared_ptr<NamedMethod> standardMethod = NamedMethod::create("standard", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner; (void)args;
        return std::shared_ptr<NamedObject>(nullptr);
    });

    // 2. Create a TypedNamedMethod (no schema)
    std::shared_ptr<TypedNamedMethod> typedMethodNoSchema = TypedNamedMethod::create("typed_no_schema", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner; (void)args;
        return std::shared_ptr<NamedObject>(nullptr);
    });

    // 3. Create a TypedNamedMethod (with simple schema)
    std::shared_ptr<ContainerDef> schema = ContainerDef::create("dummy_schema");
    std::shared_ptr<TypedNamedMethod> typedMethodWithSchema = TypedNamedMethod::create("typed_with_schema", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner; (void)args;
        return std::shared_ptr<NamedObject>(nullptr);
    }, schema);

    std::shared_ptr<NamedObject> args = NamedObject::create("args");

    // Benchmark Standard
    std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        standardMethod->execute(args);
    }
    std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
    int64_t standardDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    // Benchmark Typed (No Schema)
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        typedMethodNoSchema->execute(args);
    }
    end = std::chrono::high_resolution_clock::now();
    int64_t typedNoSchemaDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    // Benchmark Typed (With Schema)
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        typedMethodWithSchema->execute(args);
    }
    end = std::chrono::high_resolution_clock::now();
    int64_t typedWithSchemaDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "\n--- Call Overhead Results (1M iterations) ---" << std::endl;
    std::cout << "Standard NamedMethod:      " << standardDuration << " us" << std::endl;
    std::cout << "TypedMethod (No Schema):   " << typedNoSchemaDuration << " us" << std::endl;
    std::cout << "TypedMethod (With Schema): " << typedWithSchemaDuration << " us" << std::endl;
    std::cout << "-------------------------------------------\n" << std::endl;

    // Verify overhead is reasonable (within 2x for virtual chain and validation logic)
    EXPECT_LT(typedNoSchemaDuration, standardDuration * 2.0); 
}

/**
 * @brief Benchmark to prove that new Typed subclasses don't slow down the core NamedObject.
 */
TEST_F(MethodPerformanceTest, CoreNamedObjectIsolation) {
    const int iterations = 1000000;
    
    // Performance check for core NamedObject (which should be identical to previous releases)
    std::shared_ptr<NamedObject> obj = NamedObject::create("baseline");
    
    std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        std::string name = obj->getName();
        (void)name;
    }
    std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
    int64_t coreDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    
    std::cout << "Core NamedObject::getName: " << coreDuration << " us (1M iterations)" << std::endl;
    
    //getName should remain sub-microsecond on modern HW.
    EXPECT_LT(coreDuration, 1000000); // Sanity check: < 1s for 1M calls.
}

/**
 * @brief Verify that TypedNamedMethod rejects missing arguments when a schema is present.
 */
TEST_F(MethodPerformanceTest, ValidationRejection) {
    std::shared_ptr<ContainerDef> schema = ContainerDef::create("input_schema");
    std::shared_ptr<TypedNamedMethod> method = TypedNamedMethod::create("test", [](std::shared_ptr<NamedObject> owner, std::shared_ptr<NamedObject> args) {
        (void)owner; (void)args;
        return std::shared_ptr<NamedObject>(nullptr);
    }, schema);

    // Should throw because schema is present but args is null
    EXPECT_THROW(method->execute(nullptr), std::invalid_argument);
    
    // Should NOT throw if args is provided
    std::shared_ptr<NamedObject> args = NamedObject::create("args");
    EXPECT_NO_THROW(method->execute(args));
}
