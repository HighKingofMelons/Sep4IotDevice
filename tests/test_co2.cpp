#include <gtest/gtest.h>
#include "fff.h"


extern "C"
{
    #include "fakes.h"
    #include "../include/co2.h"
    #include <../include/taskConfig.h>
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

        void callFunctionNTimes(void (*func)(co2_t, uint16_t), co2_t parameter, uint16_t ppm, int n)
        {
            for (int i = 0; i < n; i++)
            {
                func(parameter, ppm);
            }
        }
};
uint16_t co2item;
mh_z19_returnCode_t returnco2;

mh_z19_returnCode_t mh_z19_getCo2Ppm_custom_func(uint16_t *ppm)
{
        *ppm = co2item;

        return returnco2;
}

TEST_F(Test_co2, co2_treate_freaquency_300000UL)
{
        mh_z19_takeMeassuring_fake.return_val = MHZ19_OK;
        xTaskGetTickCount_fake.return_val = (TickType_t)50;
        TickType_t freaquency = pdMS_TO_TICKS(300000UL);

        co2_t result_co2 = co2_create(freaquency);
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
        co2_t result_co2 = co2_create(pdMS_TO_TICKS(300000UL));

        uint16_t result_average = co2_get_latest_average_co2(result_co2);

        EXPECT_EQ(3, xSemaphoreTake_fake.call_count);
        EXPECT_EQ(1, xSemaphoreGive_fake.call_count);
        EXPECT_EQ(0, result_average);
}
TEST_F(Test_co2, co2_initialize_co2_driver){
        mh_z19_initialise((ser_USART3));
        mh_z19_takeMeassuring_fake.return_val = MHZ19_OK;
        xTaskGetTickCount_fake.return_val = (TickType_t)50;
        TickType_t freaquency = pdMS_TO_TICKS(300000UL);

        co2_t result_co2 = co2_create(freaquency);
        EXPECT_EQ(1, mh_z19_takeMeassuring_fake.call_count);
        EXPECT_EQ(1, xTaskGetTickCount_fake.call_count);
}
TEST(co2, Create_co2)
{
    co2_t co2_1 = co2_create(pdMS_TO_TICKS((uint16_t)300000UL));
    co2_t co2_2 = co2_1;
    co2_t co2_3 = co2_create(pdMS_TO_TICKS((uint16_t)300000UL));
    EXPECT_EQ(co2_1, co2_2);
    EXPECT_FALSE(co2_1 == co2_3);
}
TEST(co2, Avg_co2)
{
    BaseType_t return_sec = pdTRUE;
    SET_RETURN_SEQ(xSemaphoreTake, &return_sec, 1);
    EXPECT_FALSE(co2_get_latest_average_co2(co2_create(pdMS_TO_TICKS((uint16_t)300000UL))));
}
TEST(co2, AddCo2)
{
    co2_t co2 = co2_create(pdMS_TO_TICKS((uint16_t)300000UL));
    addCo2(co2, 666);
    EXPECT_TRUE(co2);
}
TEST(co2, Mesure)
{
    co2_t co2 = co2_create(pdMS_TO_TICKS((uint16_t)300000UL));
    EXPECT_EQ(makeOneCo2Mesurment(co2), 0);
}
TEST(co2, Reset)
{
    co2_t co2 = co2_create(pdMS_TO_TICKS((uint16_t)300000UL));
    resetCo2Array(co2);
    EXPECT_TRUE(co2);
}
TEST(co2, Calculate)
{
    co2_t co2 = co2_create(pdMS_TO_TICKS((uint16_t)300000UL));
    BaseType_t return_sec[] = {pdFALSE, pdTRUE};
    SET_RETURN_SEQ(xSemaphoreTake, &return_sec[1], 1);
    calculateCo2(co2);
    EXPECT_TRUE(co2);
}

TEST_F(Test_co2, co2_Messure)
{
    xTaskGetTickCount_fake.return_val = (TickType_t)50;
    mh_z19_takeMeassuring_fake.return_val = MHZ19_OK;
    mh_z19_getCo2Ppm_fake.custom_fake = mh_z19_getCo2Ppm_custom_func;
    co2item = 234;
    returnco2 = MHZ19_OK;
    
    xSemaphoreTake_fake.return_val = true;
    co2_t result_co2 = co2_create(pdMS_TO_TICKS(300000UL));
    uint16_t ppm;
    mh_z19_getCo2Ppm(&ppm);

    EXPECT_EQ(234, ppm);
}

TEST_F(Test_co2, co2_get_latest_average_co2)
{
    mh_z19_takeMeassuring_fake.return_val = MHZ19_OK;
    BaseType_t semaphoreTakeReturnVals[3] = {0, 0, 1};
    SET_RETURN_SEQ(xSemaphoreTake, semaphoreTakeReturnVals, 3);
    co2_t result_co2 = co2_create(pdMS_TO_TICKS(300000UL));

    uint16_t result_average = co2_get_latest_average_co2(result_co2);

    EXPECT_EQ(3, xSemaphoreTake_fake.call_count);
    EXPECT_EQ(1, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(0, result_average);
}
TEST_F(Test_co2, co2_get_latest_average_co2_10x10)
{
    xTaskGetTickCount_fake.return_val = (TickType_t)50;
    mh_z19_initialise_fake;
    mh_z19_takeMeassuring_fake.return_val = MHZ19_OK;

    mh_z19_getCo2Ppm_fake.custom_fake = mh_z19_getCo2Ppm_custom_func;
    co2item = 234;
    returnco2 = MHZ19_OK;

    BaseType_t semaphoreTakeReturnVals[11] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    SET_RETURN_SEQ(xSemaphoreTake, semaphoreTakeReturnVals, 11);
    co2_t result_co2 = co2_create(pdMS_TO_TICKS(300000UL));
    uint16_t ppm;
    mh_z19_getCo2Ppm(&ppm);

    callFunctionNTimes(&co2_makeOneMesuremnt, result_co2, ppm, 10);
    uint16_t result_average = co2_get_latest_average_co2(result_co2);

    EXPECT_EQ(3, xSemaphoreTake_fake.call_count);
    EXPECT_EQ(2, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(234, result_average);
}
TEST_F(Test_co2, co2_acceptability_max_limit_exeeded_1){
    xTaskGetTickCount_fake.return_val = (TickType_t) 50;
    mh_z19_initialise_fake;
    mh_z19_takeMeassuring_fake.return_val = MHZ19_OK;

    mh_z19_getCo2Ppm_fake.custom_fake = mh_z19_getCo2Ppm_custom_func;
    co2item = 934;
    returnco2 = MHZ19_OK;

    BaseType_t semaphoreTakeReturnVals[11] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    SET_RETURN_SEQ(xSemaphoreTake, semaphoreTakeReturnVals, 11);
    co2_t result_co2 = co2_create(pdMS_TO_TICKS(300000UL));
    uint16_t ppm;

    mh_z19_getCo2Ppm(&ppm);
    co2_set_limits(result_co2, 900, 200);

    callFunctionNTimes(&co2_makeOneMesuremnt, result_co2, ppm, 10);
    uint16_t result_average = co2_get_latest_average_co2(result_co2);
    int8_t result_acceptability = co2_acceptability_status(result_co2);

    EXPECT_EQ(934, result_average);
    EXPECT_EQ((int8_t)1, result_acceptability);
}
TEST_F(Test_co2, co2_acceptability_min_limit_exeeded_minus1)
{
    xTaskGetTickCount_fake.return_val = (TickType_t)50;
    mh_z19_initialise_fake;
    mh_z19_takeMeassuring_fake.return_val = MHZ19_OK;

    mh_z19_getCo2Ppm_fake.custom_fake = mh_z19_getCo2Ppm_custom_func;
    co2item = 199;
    returnco2 = MHZ19_OK;

    BaseType_t semaphoreTakeReturnVals[11] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    SET_RETURN_SEQ(xSemaphoreTake, semaphoreTakeReturnVals, 11);
    co2_t result_co2 = co2_create(pdMS_TO_TICKS(300000UL));
    uint16_t ppm;

    mh_z19_getCo2Ppm(&ppm);
    co2_set_limits(result_co2, 900, 200);

    callFunctionNTimes(&co2_makeOneMesuremnt, result_co2, ppm, 10);
    uint16_t result_average = co2_get_latest_average_co2(result_co2);
    int8_t result_acceptability = co2_acceptability_status(result_co2);

    EXPECT_EQ(199, result_average);
    EXPECT_EQ((int8_t) -1, result_acceptability);
}
TEST_F(Test_co2, co2_acceptability_limit_not_exeeded_0)
{
    xTaskGetTickCount_fake.return_val = (TickType_t)50;
    mh_z19_initialise_fake;
    mh_z19_takeMeassuring_fake.return_val = MHZ19_OK;

    mh_z19_getCo2Ppm_fake.custom_fake = mh_z19_getCo2Ppm_custom_func;
    co2item = 200;
    returnco2 = MHZ19_OK;

    BaseType_t semaphoreTakeReturnVals[11] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    SET_RETURN_SEQ(xSemaphoreTake, semaphoreTakeReturnVals, 11);
    co2_t result_co2 = co2_create(pdMS_TO_TICKS(300000UL));
    uint16_t ppm;

    mh_z19_getCo2Ppm(&ppm);
    co2_set_limits(result_co2, 900, 200);

    callFunctionNTimes(&co2_makeOneMesuremnt, result_co2, ppm, 10);
    uint16_t result_average = co2_get_latest_average_co2(result_co2);
    int8_t result_acceptability = co2_acceptability_status(result_co2);

    EXPECT_EQ(200, result_average);
    EXPECT_EQ((int8_t) 0, result_acceptability);
}
