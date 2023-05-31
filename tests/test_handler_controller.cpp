#include "gtest/gtest.h"

extern "C"
{
    #include "fakes.h"
    #include <taskConfig.h>
    #include "Controls/temperature.h"
    #include "Controls/humidity.h"
    #include "Controls/co2.h"
    #include "Controls/error.h"
    #include "Controls/actuation.h"
    #include "Controls/LoRaWANHandler.h"
    #include "Controls/handler_controller.h"
}

class Test_handler_controller : public ::testing::Test
{
protected:
    void SetUp() override
    {
        RESET_FAKE(status_leds_initialise);
        RESET_FAKE(xTaskDelayUntil);
        RESET_FAKE(stdio_initialise);
        RESET_FAKE(status_leds_initialise);
        RESET_FAKE(vTaskDelay);
        RESET_FAKE(xTaskCreate);
        RESET_FAKE(xTaskGetTickCount);
        FFF_RESET_HISTORY();
    }

    void TearDown() override
    {
    }

    temperature_st makeTemp() {
        temperature_st _temp = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0,
            32,
            0,
            0,
            0,
            0,
            0,
            0,
            0
        };

        return _temp;
    }

    humidity_st makeHum() {
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

    co2_st makeCo2() {
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

    error_handler makeError() {
        error_handler _error = {
            0,
            0,
            2,
            0,
            0
        };

        return _error;
    }

    actuation_handler makeActuation(temperature_t temp, humidity_t hum) {
        actuation_handler _actuation = {
            temp,
            hum,
            0,
            0,
            VENT_OFF,
            AIRCON_OFF,
            ACTUATORS_OFF
        };

        return _actuation;
    }
};