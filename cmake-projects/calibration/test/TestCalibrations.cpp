#include <gtest/gtest.h>
#include "quasar/calibration/Calibrations.hpp"

using namespace quasar::calibration;

TEST(CalibrationsTest, Identity) {
    IdentityCalibration cal;
    EXPECT_EQ(std::get<double>(cal.rawToEng(5.5)), 5.5);
    EXPECT_EQ(std::get<int64_t>(cal.rawToEng(int64_t(42))), 42);
}

TEST(CalibrationsTest, Linear) {
    LinearCalibration cal(2.0, 10.0);
    // raw = 5 -> eng = 20
    EXPECT_DOUBLE_EQ(std::get<double>(cal.rawToEng(5.0)), 20.0);
    // eng = 20 -> raw = 5
    EXPECT_DOUBLE_EQ(std::get<double>(cal.engToRaw(20.0)), 5.0);
}

TEST(CalibrationsTest, Polynomial) {
    // 2 + 3x + x^2
    PolynomialCalibration cal({2.0, 3.0, 1.0});
    // raw = 2 -> eng = 2 + 6 + 4 = 12
    EXPECT_DOUBLE_EQ(std::get<double>(cal.rawToEng(2.0)), 12.0);
    
    EXPECT_THROW(cal.engToRaw(12.0), std::logic_error);
}

TEST(CalibrationsTest, EnumMapping) {
    std::map<int64_t, std::string> m = {{0, "OFF"}, {1, "ON"}};
    EnumCalibration cal(m);
    
    EXPECT_EQ(std::get<std::string>(cal.rawToEng(int64_t(0))), "OFF");
    EXPECT_EQ(std::get<int64_t>(cal.engToRaw("ON")), 1);
    EXPECT_THROW(cal.rawToEng(int64_t(2)), std::invalid_argument);
    EXPECT_THROW(cal.engToRaw("UNKNOWN"), std::invalid_argument);
}

TEST(CalibrationsTest, Format) {
    FormatCalibration cal(2, "V");
    EXPECT_EQ(std::get<std::string>(cal.rawToEng(3.14159)), "3.14 V");
    EXPECT_DOUBLE_EQ(std::get<double>(cal.engToRaw("3.14 V")), 3.14);
}
