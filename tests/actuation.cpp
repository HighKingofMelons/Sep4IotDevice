#include <gtest/gtest.h>

extern "C" {
    #include <stdint.h>
    #include "fakes.h"

    #include "Controls/actuation.h"
    #include "Controls/temperature.h"
    #include "Controls/humidity.h"
}

humidity_st makeHumid (int8_t accept_result) {
    return (humidity_st) {
        (uint16_t[10]) {0},
        0,
        (uint8_t) (16 + (15 * accept_result)),
        0,
        0,
        0,
        1,
        1,
        20,
        10
    };
}

temperature_st makeTemp (int8_t accept_result) {
    return (temperature_st) {
        (int16_t[10]) {0},
        0,
        (uint8_t) (16 + (15 * accept_result)),
        0,
        0,
        0,
        1,
        1,
        20,
        10
    };
}

TEST(actuation_handler, test_fakestructs) {
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
    FFF_RESET_HISTORY();

    humidity_st hum1 = makeHumid(1);
    humidity_st hum2 = makeHumid(0);
    humidity_st hum3 = makeHumid(-1);

    xSemaphoreTake_fake.return_val = pdTRUE;

    ASSERT_EQ(humidity_acceptability_status(&hum1), 1);
    ASSERT_EQ(humidity_acceptability_status(&hum2), 0);
    ASSERT_EQ(humidity_acceptability_status(&hum3), -1);

    temperature_st tem1 = makeTemp(1);
    temperature_st tem2 = makeTemp(0);
    temperature_st tem3 = makeTemp(-1);

    ASSERT_EQ(temperature_acceptability_status(&tem1), 1);
    ASSERT_EQ(temperature_acceptability_status(&tem2), 0);
    ASSERT_EQ(temperature_acceptability_status(&tem3), -1);
}

TEST(actuation_handler, update_aircon_overrides) {
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
    RESET_FAKE(rc_servo_setPosition);
    FFF_RESET_HISTORY();

    temperature_st temp = makeTemp(0);

    struct actuation_handler _hand = {
        &temp,
        NULL,
        0,
        pdFALSE,
        pdTRUE
    };

    xSemaphoreTake_fake.return_val = pdFALSE;
    
    update_aircon(&_hand);
    
    ASSERT_EQ(xSemaphoreGive_fake.call_count, 0);

    xSemaphoreTake_fake.return_val = pdTRUE;
    
    update_aircon(&_hand);
    
    ASSERT_EQ(xSemaphoreGive_fake.call_count, 1);
    ASSERT_EQ(rc_servo_setPosition_fake.call_count, 0);

    _hand.aircon_overriden = pdFALSE;
    
    update_aircon(&_hand);
    
    ASSERT_GE(xSemaphoreGive_fake.call_count, 2);
    ASSERT_EQ(rc_servo_setPosition_fake.call_count, 1);
}

TEST(actuation_handler, update_vent_overrides) {
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
    RESET_FAKE(rc_servo_setPosition);
    FFF_RESET_HISTORY();

    humidity_st humid = makeHumid(1);

    struct actuation_handler _hand = {
        NULL,
        &humid,
        0,
        pdTRUE,
        pdFALSE
    };

    xSemaphoreTake_fake.return_val = pdFALSE;
    
    update_vent(&_hand);
    
    ASSERT_EQ(xSemaphoreGive_fake.call_count, 0);

    xSemaphoreTake_fake.return_val = pdTRUE;

    update_vent(&_hand);
    
    ASSERT_EQ(xSemaphoreGive_fake.call_count, 1);
    ASSERT_EQ(rc_servo_setPosition_fake.call_count, 0);

    _hand.ventilation_overriden = pdFALSE;
    
    update_vent(&_hand);
    
    ASSERT_GE(xSemaphoreGive_fake.call_count, 2);
    ASSERT_EQ(rc_servo_setPosition_fake.call_count, 1);
}

TEST(actuation_handler, update_aircon_temp_aircon_OFF) {
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
    RESET_FAKE(rc_servo_setPosition);
    FFF_RESET_HISTORY();

    temperature_st temp = makeTemp(0);

    struct actuation_handler _hand = {
        &temp,
        NULL,
        0,
        pdTRUE,
        pdFALSE
    };

    xSemaphoreTake_fake.return_val = pdTRUE;
    
    update_aircon(&_hand);
    
    ASSERT_GE(xSemaphoreGive_fake.call_count, 2);
    ASSERT_EQ(rc_servo_setPosition_fake.call_count, 1);
    ASSERT_EQ(rc_servo_setPosition_fake.arg1_val, AIRCON_OFF);
}

TEST(actuation_handler, update_aircon_temp_aircon_COOL) {
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
    RESET_FAKE(rc_servo_setPosition);
    FFF_RESET_HISTORY();

    temperature_st temp = makeTemp(1);

    struct actuation_handler _hand = {
        &temp,
        NULL,
        0,
        pdTRUE,
        pdFALSE
    };

    xSemaphoreTake_fake.return_val = pdTRUE;
    
    update_aircon(&_hand);
    
    ASSERT_GE(xSemaphoreGive_fake.call_count, 2);
    ASSERT_EQ(rc_servo_setPosition_fake.call_count, 1);
    ASSERT_EQ(rc_servo_setPosition_fake.arg1_val, AIRCON_COOL);
}

TEST(actuation_handler, update_aircon_temp_aircon_HEAT) {
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
    RESET_FAKE(rc_servo_setPosition);
    FFF_RESET_HISTORY();

    temperature_st temp = makeTemp(-1);

    struct actuation_handler _hand = {
        &temp,
        NULL,
        0,
        pdTRUE,
        pdFALSE
    };

    xSemaphoreTake_fake.return_val = pdTRUE;
    
    update_aircon(&_hand);
    
    ASSERT_GE(xSemaphoreGive_fake.call_count, 2);
    ASSERT_EQ(rc_servo_setPosition_fake.call_count, 1);
    ASSERT_EQ(rc_servo_setPosition_fake.arg1_val, AIRCON_HEAT);
}

TEST(actuation_handler, update_vent_humid_vent_ON) {
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
    RESET_FAKE(rc_servo_setPosition);
    FFF_RESET_HISTORY();

    humidity_st humid = makeHumid(1);

    struct actuation_handler _hand = {
        NULL,
        &humid,
        0,
        pdFALSE,
        pdTRUE
    };

    xSemaphoreTake_fake.return_val = pdTRUE;

    update_vent(&_hand);

    ASSERT_GE(xSemaphoreGive_fake.call_count, 2);
    ASSERT_EQ(rc_servo_setPosition_fake.call_count, 1);
    ASSERT_EQ(rc_servo_setPosition_fake.arg1_val, VENT_ON);
}

TEST(actuation_handler, update_vent_humid_vent_OFF1) {
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
    RESET_FAKE(rc_servo_setPosition);
    FFF_RESET_HISTORY();

    humidity_st humid = makeHumid(0);

    struct actuation_handler _hand = {
        NULL,
        &humid,
        0,
        pdFALSE,
        pdTRUE
    };

    xSemaphoreTake_fake.return_val = pdTRUE;

    update_vent(&_hand);
    
    ASSERT_GE(xSemaphoreGive_fake.call_count, 2);
    ASSERT_EQ(rc_servo_setPosition_fake.call_count, 1);
    ASSERT_EQ(rc_servo_setPosition_fake.arg1_val, VENT_OFF);
}

TEST(actuation_handler, update_vent_humid_vent_OFF2) {
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
    RESET_FAKE(rc_servo_setPosition);
    FFF_RESET_HISTORY();

    humidity_st humid = makeHumid(-1);

    struct actuation_handler _hand = {
        NULL,
        &humid,
        0,
        pdFALSE,
        pdTRUE
    };

    xSemaphoreTake_fake.return_val = pdTRUE;
    
    update_vent(&_hand);
    
    ASSERT_GE(xSemaphoreGive_fake.call_count, 2);
    ASSERT_EQ(rc_servo_setPosition_fake.call_count, 1);
    ASSERT_EQ(rc_servo_setPosition_fake.arg1_val, VENT_OFF);
}
