#include "gtest/gtest.h"

extern "C"
{
    #include "fakes.h"
    #include "Controls/temperature.h"
    #include <../include/taskConfig.h>
}

class Test_temperature : public ::testing::Test
{
protected:
    void SetUp() override
    {
        RESET_FAKE(xTaskCreate);
        RESET_FAKE(xTaskGetTickCount);
        RESET_FAKE(vTaskDelay);
        RESET_FAKE(xSemaphoreTake);
        RESET_FAKE(xSemaphoreGive);
        RESET_FAKE(vTaskDelete);
        RESET_FAKE(hih8120_destroy);
        RESET_FAKE(xSemaphoreCreateMutex);
        RESET_FAKE(hih8120_initialise);
        RESET_FAKE(hih8120_wakeup);
        RESET_FAKE(xTaskDelayUntil);
        RESET_FAKE(hih8120_measure);
        RESET_FAKE(hih8120_getTemperature_x10);
        RESET_FAKE(hih8120_isReady);
        FFF_RESET_HISTORY();
    }

    void TearDown() override
    {
    }

    void callFunctionNTimes(void (*func)(temperature_t), temperature_t parameter, int n) {
        for (int i = 0; i < n; i++) {
            func(parameter);
        }
    }

};

TEST_F(Test_temperature, temperature_create_freaquency_300000UL)
{
    hih8120_initialise_fake.return_val = HIH8120_OK;
    xTaskGetTickCount_fake.return_val = (TickType_t) 50;
    TickType_t freaquency = pdMS_TO_TICKS(300000UL);
    
    temperature_t result_temperature = temperature_create(freaquency);
    
    EXPECT_EQ(1, hih8120_initialise_fake.call_count);
    EXPECT_EQ(1, hih8120_initialise_fake.call_count);
    EXPECT_EQ(1, xTaskGetTickCount_fake.call_count);
    EXPECT_EQ(3, xSemaphoreCreateMutex_fake.call_count);
    EXPECT_EQ(result_temperature, xTaskCreate_fake.arg3_val);
}

TEST_F(Test_temperature, temperature_get_latest_average_temperature_0)
{
    hih8120_initialise_fake.return_val = HIH8120_OK;
    BaseType_t semaphoreTakeReturnVals[3] = { 0, 0, 1 };
    SET_RETURN_SEQ(xSemaphoreTake, semaphoreTakeReturnVals, 3);
    temperature_t result_temperature = temperature_create(pdMS_TO_TICKS(300000UL));

    uint16_t result_average = temperature_get_latest_average_temperature(result_temperature);

    EXPECT_EQ(3, xSemaphoreTake_fake.call_count);
    EXPECT_EQ(1, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(0, result_average);
}

TEST_F(Test_temperature, temperature_get_latest_average_temperature_10X10)
{
    xTaskGetTickCount_fake.return_val = (TickType_t) 50;
    hih8120_initialise_fake.return_val = HIH8120_OK;
    hih8120_wakeup_fake.return_val = HIH8120_OK;
    hih8120_measure_fake.return_val = HIH8120_OK;
    BaseType_t hih8120_isReadyReturnVals[11] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    SET_RETURN_SEQ(hih8120_isReady, hih8120_isReadyReturnVals, 11);
    hih8120_getTemperature_x10_fake.return_val = (int16_t) 100;
    xSemaphoreTake_fake.return_val = true;
    temperature_t result_temperature = temperature_create(pdMS_TO_TICKS(300000UL));

    callFunctionNTimes(&temperature_makeOneMesurment, result_temperature, 10);
    int16_t result_average = temperature_get_latest_average_temperature(result_temperature);

    EXPECT_EQ(10, hih8120_wakeup_fake.call_count);
    EXPECT_EQ(10, hih8120_measure_fake.call_count);
    EXPECT_EQ(11, hih8120_isReady_fake.call_count);
    EXPECT_EQ(11, vTaskDelay_fake.call_count);
    EXPECT_EQ(10, hih8120_getTemperature_x10_fake.call_count);
    EXPECT_EQ(2, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(1, xTaskDelayUntil_fake.call_count);
    EXPECT_EQ((int16_t) 100, result_average);
}

TEST_F(Test_temperature, temperature_acceptability_max_limit_exeeded_1)
{
    xTaskGetTickCount_fake.return_val = (TickType_t) 50;
    hih8120_initialise_fake.return_val = HIH8120_OK;
    hih8120_wakeup_fake.return_val = HIH8120_OK;
    hih8120_measure_fake.return_val = HIH8120_OK;
    hih8120_isReady_fake.return_val = (BaseType_t) 1;
    hih8120_getTemperature_x10_fake.return_val = (int16_t) 221;
    xSemaphoreTake_fake.return_val = true;
    temperature_t result_temperature = temperature_create(pdMS_TO_TICKS(300000UL));


    temperature_set_limits(result_temperature, 220, 100);
    callFunctionNTimes(&temperature_makeOneMesurment, result_temperature, 10);
    int8_t result_acceptability = temperature_acceptability_status(result_temperature);

    EXPECT_EQ(10, vTaskDelay_fake.call_count);
    EXPECT_EQ(10, hih8120_getTemperature_x10_fake.call_count);
    EXPECT_EQ(5, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(5, xSemaphoreTake_fake.call_count);
    EXPECT_EQ((int8_t) 1, result_acceptability);
}

TEST_F(Test_temperature, temperature_acceptability_min_limit_exeeded_minus1)
{
    xTaskGetTickCount_fake.return_val = (TickType_t) 50;
    hih8120_initialise_fake.return_val = HIH8120_OK;
    hih8120_wakeup_fake.return_val = HIH8120_OK;
    hih8120_measure_fake.return_val = HIH8120_OK;
    hih8120_isReady_fake.return_val = (BaseType_t) 1;
    hih8120_getTemperature_x10_fake.return_val = (int16_t) 99;
    xSemaphoreTake_fake.return_val = true;
    temperature_t result_temperature = temperature_create(pdMS_TO_TICKS(300000UL));


    temperature_set_limits(result_temperature, 220, 100);
    callFunctionNTimes(&temperature_makeOneMesurment, result_temperature, 10);
    int8_t result_acceptability = temperature_acceptability_status(result_temperature);

    EXPECT_EQ(10, vTaskDelay_fake.call_count);
    EXPECT_EQ(10, hih8120_getTemperature_x10_fake.call_count);
    EXPECT_EQ(6, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(6, xSemaphoreTake_fake.call_count);
    EXPECT_EQ((int8_t) -1, result_acceptability);
}

TEST_F(Test_temperature, temperature_acceptability_limis_not_exeeded_0)
{
    xTaskGetTickCount_fake.return_val = (TickType_t) 50;
    hih8120_initialise_fake.return_val = HIH8120_OK;
    hih8120_wakeup_fake.return_val = HIH8120_OK;
    hih8120_measure_fake.return_val = HIH8120_OK;
    hih8120_isReady_fake.return_val = (BaseType_t) 1;
    hih8120_getTemperature_x10_fake.return_val = (int16_t) 100;
    xSemaphoreTake_fake.return_val = true;
    temperature_t result_temperature = temperature_create(pdMS_TO_TICKS(300000UL));


    temperature_set_limits(result_temperature, 220, 100);
    callFunctionNTimes(&temperature_makeOneMesurment, result_temperature, 10);
    int8_t result_acceptability = temperature_acceptability_status(result_temperature);

    EXPECT_EQ(10, vTaskDelay_fake.call_count);
    EXPECT_EQ(10, hih8120_getTemperature_x10_fake.call_count);
    EXPECT_EQ(6, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(6, xSemaphoreTake_fake.call_count);
    EXPECT_EQ((int8_t) 0, result_acceptability);
}

TEST_F(Test_temperature, temperature_acceptability_temp_avg_not_calculated)
{
    xTaskGetTickCount_fake.return_val = (TickType_t) 50;
    hih8120_initialise_fake.return_val = HIH8120_OK;
    hih8120_wakeup_fake.return_val = HIH8120_OK;
    hih8120_measure_fake.return_val = HIH8120_OK;
    hih8120_isReady_fake.return_val = (BaseType_t) 1;
    hih8120_getTemperature_x10_fake.return_val = (int16_t) 50;
    xSemaphoreTake_fake.return_val = true;
    temperature_t result_temperature = temperature_create(pdMS_TO_TICKS(300000UL));


    temperature_set_limits(result_temperature, 220, 100);
    callFunctionNTimes(&temperature_makeOneMesurment, result_temperature, 9);
    int8_t result_acceptability = temperature_acceptability_status(result_temperature);

    EXPECT_EQ(9, vTaskDelay_fake.call_count);
    EXPECT_EQ(9, hih8120_getTemperature_x10_fake.call_count);
    EXPECT_EQ(3, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(3, xSemaphoreTake_fake.call_count);
    EXPECT_EQ((int8_t) 0, result_acceptability);
}

TEST_F(Test_temperature, temperature_destroy_ok)
{
    hih8120_initialise_fake.return_val = HIH8120_OK;
    hih8120_destroy_fake.return_val = HIH8120_OK;
    xSemaphoreTake_fake.return_val = true;
    temperature_t result_temperature = temperature_create(pdMS_TO_TICKS(300000UL));
    uint16_t result_average = temperature_get_latest_average_temperature(result_temperature);

    temperature_destroy(result_temperature);

    EXPECT_EQ(1, hih8120_destroy_fake.call_count);
}

TEST_F(Test_temperature, temperature_destroy_out_of_heap)
{
    hih8120_initialise_fake.return_val = HIH8120_OUT_OF_HEAP;
    hih8120_destroy_fake.return_val = HIH8120_OK;
    xSemaphoreTake_fake.return_val = true;
    temperature_t result_temperature = temperature_create(pdMS_TO_TICKS(300000UL));
    uint16_t result_average = temperature_get_latest_average_temperature(result_temperature);

    temperature_destroy(result_temperature);

    EXPECT_EQ(1, hih8120_destroy_fake.call_count);
}

