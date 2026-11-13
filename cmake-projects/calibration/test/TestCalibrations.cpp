/**
 * @file TestCalibrations.cpp
 * @brief GoogleTest unit tests verifying core Quasar calibration algorithms and error handling.
 * @details Exercises IdentityCalibration, LinearCalibration, PolynomialCalibration,
 * EnumCalibration, and FormatCalibration forward and inverse conversion paths.
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @compliance Ensures unit-test macros and assertions comply with CS-0090.1, CS-0090.2,
 * CS-0090.5, and CS-0010.44 density rules.
 */
// Include GoogleTest assertions and concrete calibration model definitions.
#include <gtest/gtest.h>
#include "quasar/calibration/Calibrations.hpp"

// Bring calibration symbols into test translation-unit scope.
using namespace quasar::calibration;

/**
 * @brief Verifies pass-through behavior of IdentityCalibration for double and integer values.
 * @details Confirms rawToEng preserves both floating-point and signed 64-bit variant alternatives.
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @compliance Documents test case scope and traceability (CS-0090.1, CS-0090.2, CS-0090.5).
 */
TEST(CalibrationsTest, Identity) {
    // Construct stateless identity calibration and verify exact type-preserving pass-through.
    IdentityCalibration cal;
    EXPECT_EQ(std::get<double>(cal.rawToEng(5.5)), 5.5);
    EXPECT_EQ(std::get<int64_t>(cal.rawToEng(int64_t(42))), 42);
}

/**
 * @brief Verifies forward and inverse scaling of LinearCalibration.
 * @details Tests affine transform y = 2.0 * x + 10.0 and its exact inverse x = (y - 10.0) / 2.0.
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @compliance Documents linear calibration test case and assertions (CS-0090.1, CS-0090.2, CS-0090.5).
 */
TEST(CalibrationsTest, Linear) {
    // Instantiate affine calibration with scale 2.0 and offset 10.0.
    LinearCalibration cal(2.0, 10.0);
    // raw = 5 -> eng = 20
    EXPECT_DOUBLE_EQ(std::get<double>(cal.rawToEng(5.0)), 20.0);
    // eng = 20 -> raw = 5
    EXPECT_DOUBLE_EQ(std::get<double>(cal.engToRaw(20.0)), 5.0);
}

/**
 * @brief Verifies PolynomialCalibration evaluation and inverse rejection semantics.
 * @details Tests 2 + 3x + x^2 at x = 2 and verifies engToRaw throws std::logic_error.
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @compliance Documents polynomial test coverage and error contract verification (CS-0090.1, CS-0090.2, CS-0090.5).
 */
TEST(CalibrationsTest, Polynomial) {
    // 2 + 3x + x^2
    PolynomialCalibration cal({2.0, 3.0, 1.0});
    // raw = 2 -> eng = 2 + 6 + 4 = 12
    EXPECT_DOUBLE_EQ(std::get<double>(cal.rawToEng(2.0)), 12.0);
    
    // Confirm non-invertible polynomial throws logic_error on inverse conversion request.
    EXPECT_THROW(cal.engToRaw(12.0), std::logic_error);
}

/**
 * @brief Verifies discrete EnumCalibration forward/reverse lookup and invalid-key exceptions.
 * @details Maps 0/1 integer states to OFF/ON labels and verifies invalid_argument on unknown keys.
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @compliance Documents bidirectional enum mapping test assertions (CS-0090.1, CS-0090.2, CS-0090.5).
 */
TEST(CalibrationsTest, EnumMapping) {
    // Build discrete state lookup table and instantiate EnumCalibration.
    std::map<int64_t, std::string> m = {{0, "OFF"}, {1, "ON"}};
    EnumCalibration cal(m);
    
    // Verify valid forward and inverse mappings succeed, then verify invalid inputs throw.
    EXPECT_EQ(std::get<std::string>(cal.rawToEng(int64_t(0))), "OFF");
    EXPECT_EQ(std::get<int64_t>(cal.engToRaw("ON")), 1);
    EXPECT_THROW(cal.rawToEng(int64_t(2)), std::invalid_argument);
    EXPECT_THROW(cal.engToRaw("UNKNOWN"), std::invalid_argument);
}

/**
 * @brief Verifies FormatCalibration numeric string formatting and unit-stripping parse path.
 * @details Tests precision formatting with unit suffix "V" and inverse double parsing.
 * @task TSK-20261113-003, TSK-20260311-010
 * @feature FE-0050, FE-0160, FE-0170, FE-0210
 * @compliance Documents string-formatting calibration test coverage (CS-0090.1, CS-0090.2, CS-0090.5).
 */
TEST(CalibrationsTest, Format) {
    // Configure 2-decimal-place voltage formatter and verify round-trip string/double conversion.
    FormatCalibration cal(2, "V");
    EXPECT_EQ(std::get<std::string>(cal.rawToEng(3.14159)), "3.14 V");
    EXPECT_DOUBLE_EQ(std::get<double>(cal.engToRaw("3.14 V")), 3.14);
}
