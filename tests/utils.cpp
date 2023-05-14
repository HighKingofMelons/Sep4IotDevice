#include <gtest/gtest.h>

extern "C" {
    #include "ATMEGA_FreeRTOS.h"
    #include <utils.h>
    #include "co2.h"
    #include "fff.h"
}

TEST(Utils, floatToIntX10) {
    float zero = 0.0f;
    EXPECT_EQ(float_to_int_x10(zero), 0);

    float pointOne = 0.1f;
    EXPECT_EQ(float_to_int_x10(pointOne), 1);

    float onePointOne = 1.1f;
    EXPECT_EQ(float_to_int_x10(onePointOne), 11);
}

TEST(co2, Initialize){
    EXPECT_EQ(initializeCo2Driver(), 0);
}
