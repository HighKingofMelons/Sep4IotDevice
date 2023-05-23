#include "gtest/gtest.h"

extern "C"
{
    #include "fakes.h"
    #include "Controls/LoRaWANHandler.h"
    #include <../include/taskConfig.h>
    #include "Controls/temperature.h"
    #include "Controls/humidity.h"
    #include "Controls/error.h"
    #include <handlers.h>
}

class Test_lorawan_handler : public ::testing::Test
{
protected:
    void SetUp() override
    {
        RESET_FAKE(lora_driver_initialise);
        RESET_FAKE(status_leds_slowBlink);
        RESET_FAKE(lora_driver_mapReturnCodeToText);
        RESET_FAKE(lora_driver_rn2483FactoryReset);
        RESET_FAKE(lora_driver_configureToEu868);
        RESET_FAKE(lora_driver_getRn2483Hweui);
        RESET_FAKE(lora_driver_setOtaaIdentity);
        RESET_FAKE(lora_driver_saveMac);
        RESET_FAKE(lora_driver_setAdaptiveDataRate);
        RESET_FAKE(lora_driver_setReceiveDelay);
        RESET_FAKE(lora_driver_join);
        RESET_FAKE(lora_driver_resetRn2483);
        RESET_FAKE(lora_driver_flushBuffers);
        RESET_FAKE(_lora_setup);
        RESET_FAKE(lora_driver_sendUploadMessage);
        RESET_FAKE(status_leds_slowBlink);
        RESET_FAKE(status_leds_longPuls);
        RESET_FAKE(status_leds_ledOn);
        RESET_FAKE(status_leds_ledOff);
        RESET_FAKE(status_leds_fastBlink);
        RESET_FAKE(status_leds_shortPuls);
        RESET_FAKE(xMessageBufferReceive);
        RESET_FAKE(xMessageBufferCreate);
        FFF_RESET_HISTORY();
    }

    void TearDown() override
    {
    }

    temperature_st makeTemp(int8_t accept_result) {
        temperature_st _temp = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0,
            32,
            0,
            0,
            0,
            1,
            1,
            0,
            0
        };

        return _temp;
    }

    humidity_st makeHum(int8_t accept_result) {
        humidity_st _hum = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0,
            43,
            0,
            0,
            0,
            0,
            0,
            0,
            0
        };

        return _hum;
    }

    co2_st makeCo2(int8_t accept_result) {
        co2_st _co2 = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0,
            547,
            0,
            0,
            0
        };

        return _co2;
    }

    error_st makeCo2(int8_t accept_result) {
        co2_st _co2 = {
            0,
            0,
            2,
            0,
            0
        };

        return _co2;
    }

    void callFunctionNTimes(void (*func)(temperature_t), temperature_t parameter, int n) {
        for (int i = 0; i < n; i++) {
            func(parameter);
        }
    }
};

TEST_F(Test_lorawan_handler, lorawan_handler_create)
{   char mBuf = "y";
    void* mBufP= &mBuf;
    xMessageBufferCreate_fake.return_val =  mBufP;
    
    temperature_st tempHandler = makeTemp();
    humidity_st humHandler = makeHum();
    co2_st co2Handler = makeCo2();
    error_st errorHandler = makeError();
    handlers_t handlers = handlers_create(&tempHandler, &humHandler, &co2Handler, &errorHandler)
    lorawan_handler_t result_lorawan_handler = lorawan_handler_create(freaquency);
    
    EXPECT_EQ(1, xMessageBufferCreate_fake.call_count);
}