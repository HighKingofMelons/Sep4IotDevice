#include <gtest/gtest.h>

extern "C" {
    #include "ATMEGA_FreeRTOS.h"
    #include <utils.h>
    #include "co2.h"
    #include "fakes.h"
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
TEST(co2, Create_co2){
    co2_c co2_1 = co2_create(22, pdMS_TO_TICKS((uint16_t)300000UL));
    co2_c co2_2 = co2_1;
    co2_c co2_3 = co2_create(22, pdMS_TO_TICKS((uint16_t)300000UL));
    EXPECT_EQ(co2_1, co2_2);
    EXPECT_FALSE(co2_1 == co2_3);
}
TEST(co2, Avg_co2){
    BaseType_t return_sec[] = {pdFALSE, pdTRUE};
    SET_RETURN_SEQ(xSemaphoreTake, &return_sec[1], 1);
    EXPECT_FALSE(co2_get_latest_average_co2(co2_create(22, pdMS_TO_TICKS((uint16_t)300000UL))));
}
TEST(co2, AddCo2){
    co2_c co2 = co2_create(22, pdMS_TO_TICKS((uint16_t)300000UL));
    addCo2(co2, 666);
    EXPECT_TRUE(co2);
}
TEST(co2, Mesure){
    co2_c co2 = co2_create(22, pdMS_TO_TICKS((uint16_t) 300000UL));
    EXPECT_EQ(makeOneCo2Mesurment(co2), 0);
}

