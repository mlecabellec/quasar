#include <gtest/gtest.h>
#include "quasar/coretypes/Integer.hpp"
#include "quasar/coretypes/FloatingPoint.hpp"
#include <tuple>
#include <type_traits>
#include <limits>

using namespace quasar::coretypes;

template <typename T, typename U>
struct IntegerPolymorphicTest {
    static void Run() {
        T v1 = 10;
        U v2 = 20;
        std::shared_ptr<Number> n1 = std::make_shared<Integer<T>>(v1);
        std::shared_ptr<Number> n2 = std::make_shared<Integer<U>>(v2);

        using CommonT = std::common_type_t<T, U>;

        // add
        {
            auto res = n1->add(*n2);
            EXPECT_EQ(res->getType(), "Integer");
            if constexpr (std::is_signed_v<CommonT>) {
                EXPECT_EQ(res->toInt64(), (int64_t)(CommonT)((CommonT)v1 + (CommonT)v2));
            } else {
                EXPECT_EQ(res->toUInt64(), (uint64_t)(CommonT)((CommonT)v1 + (CommonT)v2));
            }
        }

        // subtract
        {
            auto res = n1->subtract(*n2);
            EXPECT_EQ(res->getType(), "Integer");
            if constexpr (std::is_signed_v<CommonT>) {
                EXPECT_EQ(res->toInt64(), (int64_t)(CommonT)((CommonT)v1 - (CommonT)v2));
            } else {
                EXPECT_EQ(res->toUInt64(), (uint64_t)(CommonT)((CommonT)v1 - (CommonT)v2));
            }
        }

        // multiply
        {
            auto res = n1->multiply(*n2);
            EXPECT_EQ(res->getType(), "Integer");
            if constexpr (std::is_signed_v<CommonT>) {
                EXPECT_EQ(res->toInt64(), (int64_t)(CommonT)((CommonT)v1 * (CommonT)v2));
            } else {
                EXPECT_EQ(res->toUInt64(), (uint64_t)(CommonT)((CommonT)v1 * (CommonT)v2));
            }
        }

        // divide
        {
            auto res = n1->divide(*n2);
            EXPECT_EQ(res->getType(), "Integer");
            if constexpr (std::is_signed_v<CommonT>) {
                EXPECT_EQ(res->toInt64(), (int64_t)(CommonT)((CommonT)v1 / (CommonT)v2));
            } else {
                EXPECT_EQ(res->toUInt64(), (uint64_t)(CommonT)((CommonT)v1 / (CommonT)v2));
            }
        }

        // bitwiseAnd
        {
            auto res = n1->bitwiseAnd(*n2);
            EXPECT_EQ(res->getType(), "Integer");
            if constexpr (std::is_signed_v<CommonT>) {
                EXPECT_EQ(res->toInt64(), (int64_t)(CommonT)((CommonT)v1 & (CommonT)v2));
            } else {
                EXPECT_EQ(res->toUInt64(), (uint64_t)(CommonT)((CommonT)v1 & (CommonT)v2));
            }
        }

        // bitwiseOr
        {
            auto res = n1->bitwiseOr(*n2);
            EXPECT_EQ(res->getType(), "Integer");
            if constexpr (std::is_signed_v<CommonT>) {
                EXPECT_EQ(res->toInt64(), (int64_t)(CommonT)((CommonT)v1 | (CommonT)v2));
            } else {
                EXPECT_EQ(res->toUInt64(), (uint64_t)(CommonT)((CommonT)v1 | (CommonT)v2));
            }
        }

        // bitwiseXor
        {
            auto res = n1->bitwiseXor(*n2);
            EXPECT_EQ(res->getType(), "Integer");
            if constexpr (std::is_signed_v<CommonT>) {
                EXPECT_EQ(res->toInt64(), (int64_t)(CommonT)((CommonT)v1 ^ (CommonT)v2));
            } else {
                EXPECT_EQ(res->toUInt64(), (uint64_t)(CommonT)((CommonT)v1 ^ (CommonT)v2));
            }
        }

        // bitwiseLeftShift
        {
            // Use a small shift amount to avoid UB
            std::shared_ptr<Number> shift_amt = std::make_shared<Integer<uint8_t>>(2);
            auto res = n1->bitwiseLeftShift(*shift_amt);
            EXPECT_EQ(res->getType(), "Integer");
            EXPECT_EQ(res->toInt64(), (int64_t)(T)((T)v1 << 2));
        }

        // bitwiseRightShift
        {
            std::shared_ptr<Number> shift_amt = std::make_shared<Integer<uint8_t>>(1);
            auto res = n1->bitwiseRightShift(*shift_amt);
            EXPECT_EQ(res->getType(), "Integer");
            EXPECT_EQ(res->toInt64(), (int64_t)(T)((T)v1 >> 1));
        }

        // compareTo
        {
            int cmp = n1->compareTo(*n2);
            if (v1 < v2) EXPECT_EQ(cmp, -1);
            else if (v1 > v2) EXPECT_EQ(cmp, 1);
            else EXPECT_EQ(cmp, 0);
        }

        // equals
        {
            EXPECT_FALSE(n1->equals(*n2));
            std::shared_ptr<Number> n3 = std::make_shared<Integer<T>>(v1);
            EXPECT_TRUE(n1->equals(*n3));
        }

        // safeAdd
        {
            auto res = n1->safeAdd(*n2);
            if constexpr (std::is_signed_v<CommonT>) {
                EXPECT_EQ(res->toInt64(), (int64_t)v1 + (int64_t)v2);
            } else {
                EXPECT_EQ(res->toUInt64(), (uint64_t)v1 + (uint64_t)v2);
            }
        }
    }

    static void RunEdgeCases() {
        // Test max values to check precision
        if constexpr (std::is_same_v<T, uint64_t> && std::is_same_v<U, uint64_t>) {
            uint64_t v1 = 0xFFFFFFFFFFFFFFFFULL;
            uint64_t v2 = 0;
            std::shared_ptr<Number> n1 = std::make_shared<Integer<T>>(v1);
            std::shared_ptr<Number> n2 = std::make_shared<Integer<U>>(v2);
            auto res = n1->add(*n2);
            EXPECT_EQ(res->toUInt64(), v1);
            
            // Adding 1 to a large number to ensure no precision loss (like with double)
            uint64_t large = 1ULL << 60;
            std::shared_ptr<Number> n_large = std::make_shared<Integer<uint64_t>>(large);
            std::shared_ptr<Number> n_one = std::make_shared<Integer<uint64_t>>(1);
            auto res_plus_one = n_large->add(*n_one);
            EXPECT_EQ(res_plus_one->toUInt64(), large + 1);
        }

        // Test mixed signed/unsigned comparison
        if constexpr (std::is_signed_v<T> && !std::is_signed_v<U>) {
            T neg = -1;
            U pos = 1;
            std::shared_ptr<Number> n_neg = std::make_shared<Integer<T>>(neg);
            std::shared_ptr<Number> n_pos = std::make_shared<Integer<U>>(pos);
            EXPECT_EQ(n_neg->compareTo(*n_pos), -1);
            EXPECT_EQ(n_pos->compareTo(*n_neg), 1);
        }
    }
};

using AllIntegerTypes = std::tuple<
    int8_t, int16_t, int32_t, int64_t,
    uint8_t, uint16_t, uint32_t, uint64_t,
    size_t, ptrdiff_t
>;

template<size_t I, size_t J>
void RunCombinations() {
    using T = std::tuple_element_t<I, AllIntegerTypes>;
    using U = std::tuple_element_t<J, AllIntegerTypes>;
    IntegerPolymorphicTest<T, U>::Run();
    IntegerPolymorphicTest<T, U>::RunEdgeCases();

    if constexpr (J + 1 < std::tuple_size_v<AllIntegerTypes>) {
        RunCombinations<I, J + 1>();
    } else if constexpr (I + 1 < std::tuple_size_v<AllIntegerTypes>) {
        RunCombinations<I + 1, 0>();
    }
}

TEST(IntegerPolymorphicTest, MatrixTests) {
    RunCombinations<0, 0>();
}

TEST(IntegerPolymorphicTest, MixedWithFloatingPoint) {
    auto i = std::make_shared<Integer<int64_t>>(100);
    auto f = std::make_shared<FloatingPoint<double>>(1.5);
    
    auto res = i->add(*f);
    EXPECT_EQ(res->getType(), "FloatingPoint");
    EXPECT_DOUBLE_EQ(res->toDouble(), 101.5);
    
    auto res2 = f->add(*i);
    EXPECT_EQ(res2->getType(), "FloatingPoint");
    EXPECT_DOUBLE_EQ(res2->toDouble(), 101.5);
}

TEST(IntegerPolymorphicTest, OverflowChecks) {
    auto max_u64 = std::make_shared<Integer<uint64_t>>(std::numeric_limits<uint64_t>::max());
    auto one = std::make_shared<Integer<uint64_t>>(1);
    EXPECT_THROW(max_u64->safeAdd(*one), std::overflow_error);
    
    auto min_i64 = std::make_shared<Integer<int64_t>>(std::numeric_limits<int64_t>::min());
    auto neg_one = std::make_shared<Integer<int64_t>>(-1);
    EXPECT_THROW(min_i64->safeDivide(*neg_one), std::overflow_error);
}

TEST(IntegerPolymorphicTest, BitwiseNotAndShifts) {
    auto i = std::make_shared<Integer<uint8_t>>(0x0F);
    auto shift4 = std::make_shared<Integer<int>>(4);
    auto shift2 = std::make_shared<Integer<int>>(2);
    EXPECT_EQ(i->bitwiseNot()->toUInt8(), 0xF0);
    EXPECT_EQ(i->bitwiseLeftShift(*shift4)->toUInt8(), 0xF0);
    EXPECT_EQ(i->bitwiseRightShift(*shift2)->toUInt8(), 0x03);
}
