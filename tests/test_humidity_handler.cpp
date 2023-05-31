#include "gtest/gtest.h"

extern "C"
{
    #include "fakes.h"
    #include "Controls/error_handler.h"
    #include "./Controls/humidity_handler.h"
    #include <../include/taskConfig.h>
}

class Test_humidity : public ::testing::Test
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
        RESET_FAKE(hih8120_getHumidityPercent_x10);
        RESET_FAKE(hih8120_isReady);
        FFF_RESET_HISTORY();
    }

    void TearDown() override
    {
    }

    void callFunctionNTimes(void (*func)(humidity_t), humidity_t parameter, int n) {
        for (int i = 0; i < n; i++) {
            func(parameter);
        }
    }

    error_handler makeErrorHandler() {
        error_handler _error = {
            0,
            0,
            0,
            0,
            0
        };

        return _error;
    }
};

TEST_F(Test_humidity, humidity_create)
{
    hih8120_initialise_fake.return_val = HIH8120_OK;
    xTaskGetTickCount_fake.return_val = (TickType_t) 50;
    error_handler errorH = makeErrorHandler();
    
    humidity_t result_humidity = humidity_create(&errorH, 0);
    
    EXPECT_EQ(1, hih8120_initialise_fake.call_count);
    EXPECT_EQ(1, hih8120_initialise_fake.call_count);
    EXPECT_EQ(0, xTaskGetTickCount_fake.call_count);
    EXPECT_EQ(3, xSemaphoreCreateMutex_fake.call_count);
    EXPECT_EQ(result_humidity, xTaskCreate_fake.arg3_val);
}

TEST_F(Test_humidity, humidity_get_latest_average_humidity_0)
{
    hih8120_initialise_fake.return_val = HIH8120_OK;
    BaseType_t semaphoreTakeReturnVals[3] = { 0, 0, 1 };
    SET_RETURN_SEQ(xSemaphoreTake, semaphoreTakeReturnVals, 3);
    error_handler errorH = makeErrorHandler();    
    humidity_t result_humidity = humidity_create(&errorH, 0);

    uint16_t result_average = humidity_get_latest_average_humidity(result_humidity);

    EXPECT_EQ(3, xSemaphoreTake_fake.call_count);
    EXPECT_EQ(1, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(0, result_average);
}

TEST_F(Test_humidity, humidity_get_latest_average_humidity_10X10)
{
    xTaskGetTickCount_fake.return_val = (TickType_t) 50;
    hih8120_initialise_fake.return_val = HIH8120_OK;
    hih8120_wakeup_fake.return_val = HIH8120_OK;
    hih8120_measure_fake.return_val = HIH8120_OK;
    BaseType_t hih8120_isReadyReturnVals[11] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    SET_RETURN_SEQ(hih8120_isReady, hih8120_isReadyReturnVals, 11);
    hih8120_getHumidityPercent_x10_fake.return_val = (uint16_t) 610;
    xSemaphoreTake_fake.return_val = true;
    error_handler errorH = makeErrorHandler();    
    humidity_t result_humidity = humidity_create(&errorH, 0);

    callFunctionNTimes(&humidity_makeOneMesurment, result_humidity, 10);
    uint8_t result_average = humidity_get_latest_average_humidity(result_humidity);

    EXPECT_EQ(10, hih8120_wakeup_fake.call_count);
    EXPECT_EQ(10, hih8120_measure_fake.call_count);
    EXPECT_EQ(11, hih8120_isReady_fake.call_count);
    EXPECT_EQ(11, vTaskDelay_fake.call_count);
    EXPECT_EQ(10, hih8120_getHumidityPercent_x10_fake.call_count);
    EXPECT_EQ(2, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(1, xTaskDelayUntil_fake.call_count);
    EXPECT_EQ((uint8_t) 61, result_average);
}

TEST_F(Test_humidity, humidity_acceptability_max_limit_exeeded_1)
{
    xTaskGetTickCount_fake.return_val = (TickType_t) 50;
    hih8120_initialise_fake.return_val = HIH8120_OK;
    hih8120_wakeup_fake.return_val = HIH8120_OK;
    hih8120_measure_fake.return_val = HIH8120_OK;
    hih8120_isReady_fake.return_val = (BaseType_t) 1;
    hih8120_getHumidityPercent_x10_fake.return_val = (uint16_t) 810;
    xSemaphoreTake_fake.return_val = true;
    error_handler errorH = makeErrorHandler();    
    humidity_t result_humidity = humidity_create(&errorH, 0);


    humidity_set_limits(result_humidity, 80, 13);
    callFunctionNTimes(&humidity_makeOneMesurment, result_humidity, 10);
    int8_t result_acceptability = humidity_get_acceptability_status(result_humidity);

    EXPECT_EQ(10, vTaskDelay_fake.call_count);
    EXPECT_EQ(10, hih8120_getHumidityPercent_x10_fake.call_count);
    EXPECT_EQ(5, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(5, xSemaphoreTake_fake.call_count);
    EXPECT_EQ((int8_t) 1, result_acceptability);
}

TEST_F(Test_humidity, humidity_acceptability_min_limit_exeeded_minus1)
{
    xTaskGetTickCount_fake.return_val = (TickType_t) 50;
    hih8120_initialise_fake.return_val = HIH8120_OK;
    hih8120_wakeup_fake.return_val = HIH8120_OK;
    hih8120_measure_fake.return_val = HIH8120_OK;
    hih8120_isReady_fake.return_val = (BaseType_t) 1;
    hih8120_getHumidityPercent_x10_fake.return_val = (uint8_t) 143;
    xSemaphoreTake_fake.return_val = true;
    error_handler errorH = makeErrorHandler();    
    humidity_t result_humidity = humidity_create(&errorH, 0);


    humidity_set_limits(result_humidity, 70, 15);
    callFunctionNTimes(&humidity_makeOneMesurment, result_humidity, 10);
    int8_t result_acceptability = humidity_get_acceptability_status(result_humidity);

    EXPECT_EQ(10, vTaskDelay_fake.call_count);
    EXPECT_EQ(10, hih8120_getHumidityPercent_x10_fake.call_count);
    EXPECT_EQ(6, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(6, xSemaphoreTake_fake.call_count);
    EXPECT_EQ((int8_t) -1, result_acceptability);
}

TEST_F(Test_humidity, humidity_acceptability_limis_not_exeeded_0)
{
    xTaskGetTickCount_fake.return_val = (TickType_t) 50;
    hih8120_initialise_fake.return_val = HIH8120_OK;
    hih8120_wakeup_fake.return_val = HIH8120_OK;
    hih8120_measure_fake.return_val = HIH8120_OK;
    hih8120_isReady_fake.return_val = (BaseType_t) 1;
    hih8120_getHumidityPercent_x10_fake.return_val = (int16_t) 160;
    xSemaphoreTake_fake.return_val = true;
    error_handler errorH = makeErrorHandler();    
    humidity_t result_humidity = humidity_create(&errorH, 0);


    humidity_set_limits(result_humidity, 67, 16);
    callFunctionNTimes(&humidity_makeOneMesurment, result_humidity, 10);
    int8_t result_acceptability = humidity_get_acceptability_status(result_humidity);

    EXPECT_EQ(10, vTaskDelay_fake.call_count);
    EXPECT_EQ(10, hih8120_getHumidityPercent_x10_fake.call_count);
    EXPECT_EQ(6, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(6, xSemaphoreTake_fake.call_count);
    EXPECT_EQ((int8_t) 0, result_acceptability);
}

TEST_F(Test_humidity, humidity_acceptability_hum_avg_not_calculated)
{
    xTaskGetTickCount_fake.return_val = (TickType_t) 50;
    hih8120_initialise_fake.return_val = HIH8120_OK;
    hih8120_wakeup_fake.return_val = HIH8120_OK;
    hih8120_measure_fake.return_val = HIH8120_OK;
    hih8120_isReady_fake.return_val = (BaseType_t) 1;
    hih8120_getHumidityPercent_x10_fake.return_val = (int16_t) 500;
    xSemaphoreTake_fake.return_val = true;
    error_handler errorH = makeErrorHandler();    
    humidity_t result_humidity = humidity_create(&errorH, 0);


    humidity_set_limits(result_humidity, 67, 57);
    callFunctionNTimes(&humidity_makeOneMesurment, result_humidity, 9);
    int8_t result_acceptability = humidity_get_acceptability_status(result_humidity);

    EXPECT_EQ(9, vTaskDelay_fake.call_count);
    EXPECT_EQ(9, hih8120_getHumidityPercent_x10_fake.call_count);
    EXPECT_EQ(3, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(3, xSemaphoreTake_fake.call_count);
    EXPECT_EQ((int8_t) 0, result_acceptability);
}

TEST_F(Test_humidity, humidity_destroy_ok)
{
    hih8120_initialise_fake.return_val = HIH8120_OK;
    hih8120_destroy_fake.return_val = HIH8120_OK;
    xSemaphoreTake_fake.return_val = true;
    error_handler errorH = makeErrorHandler();    
    humidity_t result_humidity = humidity_create(&errorH, 0);
    uint16_t result_average = humidity_get_latest_average_humidity(result_humidity);

    humidity_destroy(result_humidity);

    EXPECT_EQ(1, hih8120_destroy_fake.call_count);
}

TEST_F(Test_humidity, humidity_destroy_out_of_heap)
{
    hih8120_initialise_fake.return_val = HIH8120_OUT_OF_HEAP;
    hih8120_destroy_fake.return_val = HIH8120_OK;
    xSemaphoreTake_fake.return_val = true;
    error_handler errorH = makeErrorHandler();    
    humidity_t result_humidity = humidity_create(&errorH, 0);
    uint16_t result_average = humidity_get_latest_average_humidity(result_humidity);

    humidity_destroy(result_humidity);

    EXPECT_EQ(1, hih8120_destroy_fake.call_count);
}

