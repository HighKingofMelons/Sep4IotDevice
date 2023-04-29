#include <gtest/gtest.h>

extern "C" {
    #include <utils.h>
}

TEST(Utils, floatToIntX10) {
    float zero = 0.0;
    EXPECT_EQ(float_to_int_x10(zero), 0);

    float pointOne = 0.1;
    EXPECT_EQ(float_to_int_x10(pointOne), 1);

    float onePointOne = 1.1;
    EXPECT_EQ(float_to_int_x10(onePointOne), 11);
}