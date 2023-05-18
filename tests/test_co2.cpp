#include <gtest/gtest.h>

extern "C"
{
#include "ATMEGA_FreeRTOS.h"
#include <utils.h>
#include "co2.h"
#include "fakes.h"
}

class Test_co2 : public ::testing::Test{
    protected:
        void SetUp() override{
            RESET_FAKE(xTaskCreate);
            RESET_FAKE(xTaskGetTickCount);
            RESET_FAKE(vTaskDelay);
            RESET_FAKE(xSemaphoreTake);
            RESET_FAKE(xSemaphoreGive);
            RESET_FAKE(vTaskDelete);
            
            RESET_FAKE(xSemaphoreCreateMutex);
            RESET_FAKE(mh_z19_initialise);

            RESET_FAKE(xTaskDelayUntil);
            RESET_FAKE(mh_z19_takeMeassuring);
            RESET_FAKE(mh_z19_getCo2Ppm);

        }

        void TearDown() override{

        }

        void callFunctionNTimes(void (*func) (co2_c), co2_c parameter, int n){
            for(int i = 0; i < n; i++){
                func(parameter);
            }
        }
};

TEST_F(Test_co2, co2_create_freaquency_300000UL)
{
        mh_z19_takeMeassuring_fake.return_val = MHZ19_OK;
        xTaskGetTickCount_fake.return_val = (TickType_t)50;
        TickType_t freaquency = pdMS_TO_TICKS(300000UL);

        co2_c result_co2 = co2_create(freaquency);
        EXPECT_EQ(1, mh_z19_takeMeassuring_fake.call_count);
        EXPECT_EQ(1, mh_z19_takeMeassuring_fake.call_count);
        EXPECT_EQ(1, xTaskGetTickCount_fake.call_count);
        EXPECT_EQ(1, xSemaphoreCreateMutex_fake.call_count);
        EXPECT_EQ(result_co2, xTaskCreate_fake.arg3_val);
}

TEST_F(Test_co2, co2_get_latest_average_co2_0)
{
        mh_z19_takeMeassuring_fake.return_val = MHZ19_OK;
        BaseType_t semaphoreTakeReturnVals[3] = {0, 0, 1};
        SET_RETURN_SEQ(xSemaphoreTake, semaphoreTakeReturnVals, 3);
        co2_c result_co2 = co2_create(pdMS_TO_TICKS(300000UL));

        uint16_t result_average = co2_get_latest_average_co2(result_co2);

        EXPECT_EQ(3, xSemaphoreTake_fake.call_count);
        EXPECT_EQ(1, xSemaphoreGive_fake.call_count);
        EXPECT_EQ(0, result_average);
}



TEST(co2, Initialize)
{
    EXPECT_EQ(initializeCo2Driver(), 0);
}
TEST(co2, Create_co2)
{
    co2_c co2_1 = co2_create(pdMS_TO_TICKS((uint16_t)300000UL));
    co2_c co2_2 = co2_1;
    co2_c co2_3 = co2_create(pdMS_TO_TICKS((uint16_t)300000UL));
    EXPECT_EQ(co2_1, co2_2);
    EXPECT_FALSE(co2_1 == co2_3);
}
TEST(co2, Avg_co2)
{
    BaseType_t return_sec[] = {pdFALSE, pdTRUE};
    SET_RETURN_SEQ(xSemaphoreTake, &return_sec[1], 1);
    EXPECT_FALSE(co2_get_latest_average_co2(co2_create(pdMS_TO_TICKS((uint16_t)300000UL))));
}
TEST(co2, AddCo2)
{
    co2_c co2 = co2_create(pdMS_TO_TICKS((uint16_t)300000UL));
    addCo2(co2, 666);
    EXPECT_TRUE(co2);
}
TEST(co2, Mesure)
{
    co2_c co2 = co2_create(pdMS_TO_TICKS((uint16_t)300000UL));
    EXPECT_EQ(makeOneCo2Mesurment(co2), 0);
}
TEST(co2, Reset)
{
    co2_c co2 = co2_create(pdMS_TO_TICKS((uint16_t)300000UL));
    resetCo2Array(co2);
    EXPECT_TRUE(co2);
}
TEST(co2, Calculate)
{
    co2_c co2 = co2_create(pdMS_TO_TICKS((uint16_t)300000UL));
    BaseType_t return_sec[] = {pdFALSE, pdTRUE};
    SET_RETURN_SEQ(xSemaphoreTake, &return_sec[1], 1);
    calculateCo2(co2);
    EXPECT_TRUE(co2);
}