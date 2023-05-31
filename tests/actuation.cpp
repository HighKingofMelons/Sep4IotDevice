#include <gtest/gtest.h>

extern "C" {
    #include <stdint.h>
    #include "fakes.h"
    #include "Controls/temperature_handler.h"
    #include "Controls/humidity_handler.h"
    #include "Controls/actuation.h"
}

humidity_st makeHumid (int8_t accept_result) {
    humidity_st _humi = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,
        (uint8_t) (16 + (15 * accept_result)),
        0,
        0,
        0,
        1,
        20,
        10,
        0,
        0
    };

    return _humi;
}

temperature_st makeTemp (int8_t accept_result) {
    temperature_st _temp = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0,
        (uint8_t) (16 + (15 * accept_result)),
        0,
        0,
        0,
        1,
        20,
        10,
        0,
        0
    };

    return _temp;
}

TEST(actuation_handler, test_fakestructs) {
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
    FFF_RESET_HISTORY();

    humidity_st hum1 = makeHumid(1);
    humidity_st hum2 = makeHumid(0);
    humidity_st hum3 = makeHumid(-1);

    xSemaphoreTake_fake.return_val = pdTRUE;

    ASSERT_EQ(humidity_get_acceptability_status(&hum1), 1);
    ASSERT_EQ(humidity_get_acceptability_status(&hum2), 0);
    ASSERT_EQ(humidity_get_acceptability_status(&hum3), -1);

    temperature_st tem1 = makeTemp(1);
    temperature_st tem2 = makeTemp(0);
    temperature_st tem3 = makeTemp(-1);

    ASSERT_EQ(temperature_get_acceptability_status(&tem1), 1);
    ASSERT_EQ(temperature_get_acceptability_status(&tem2), 0);
    ASSERT_EQ(temperature_get_acceptability_status(&tem3), -1);
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
        0,
        pdFALSE,
        pdTRUE,
        ACTUATORS_ON
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
        0,
        pdTRUE,
        pdFALSE,
        ACTUATORS_ON
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
        0,
        pdTRUE,
        pdFALSE,
        ACTUATORS_ON
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
        0,
        pdTRUE,
        pdFALSE,
        ACTUATORS_ON
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
        0,
        pdTRUE,
        pdFALSE,
        ACTUATORS_ON
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
        0,
        pdFALSE,
        pdTRUE,
        ACTUATORS_ON
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
        0,
        pdFALSE,
        pdTRUE,
        ACTUATORS_ON
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
        0,
        pdFALSE,
        pdTRUE,
        ACTUATORS_ON
    };

    xSemaphoreTake_fake.return_val = pdTRUE;
    
    update_vent(&_hand);
    
    ASSERT_GE(xSemaphoreGive_fake.call_count, 2);
    ASSERT_EQ(rc_servo_setPosition_fake.call_count, 1);
    ASSERT_EQ(rc_servo_setPosition_fake.arg1_val, VENT_OFF);
}

TEST(actuation_handler, init) {
    RESET_FAKE(rc_servo_initialise);
    RESET_FAKE(rc_servo_setPosition);
    RESET_FAKE(xSemaphoreCreateMutex);
    RESET_FAKE(xTaskCreate);
    FFF_RESET_HISTORY();

    temperature_st temp = makeTemp(0);
    humidity_st humi = makeHumid(0);

    actuation_handler_t act = actuation_handler_init(&temp, &humi);

    ASSERT_EQ(rc_servo_initialise_fake.call_count, 1);
    ASSERT_EQ(rc_servo_setPosition_fake.call_count, 2);
    ASSERT_EQ(rc_servo_setPosition_fake.arg0_history[0], 0);
    ASSERT_EQ(rc_servo_setPosition_fake.arg0_history[1], 1);
    ASSERT_EQ(rc_servo_setPosition_fake.arg1_history[0], VENT_OFF);
    ASSERT_EQ(rc_servo_setPosition_fake.arg1_history[1], AIRCON_OFF);
    ASSERT_EQ(xSemaphoreCreateMutex_fake.call_count, 2);
    ASSERT_EQ(xTaskCreate_fake.call_count, 1);

    actuation_handler_destroy(act);
}

TEST(actuation_handler, destroy) {
    temperature_st temp = makeTemp(0);
    humidity_st humi = makeHumid(0);

    actuation_handler_t act = actuation_handler_init(&temp, &humi);

    RESET_FAKE(vTaskDelete);
    RESET_FAKE(vSemaphoreDelete);
    FFF_RESET_HISTORY();

    actuation_handler_destroy(act);

    ASSERT_EQ(vTaskDelete_fake.call_count, 1);
    ASSERT_EQ(vSemaphoreDelete_fake.call_count, 2);
}

TEST(actuation_handler, ventilation_override_state) {
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
    RESET_FAKE(rc_servo_setPosition);
    FFF_RESET_HISTORY();

    struct actuation_handler _hand = {
        NULL,
        NULL,
        0,
        0,
        pdFALSE,
        pdFALSE,
        ACTUATORS_ON
    };

    BaseType_t seq[] = {pdFALSE, pdTRUE};
    SET_RETURN_SEQ(xSemaphoreTake, seq, 2);

    actuators_ventilation_override_state(&_hand, VENT_ON);
    
    ASSERT_EQ(xSemaphoreTake_fake.call_count, 2);
    ASSERT_EQ(_hand.ventilation_overriden, pdTRUE);
    ASSERT_EQ(rc_servo_setPosition_fake.call_count, 1);
    ASSERT_EQ(rc_servo_setPosition_fake.arg1_val, VENT_ON);
    ASSERT_EQ(xSemaphoreGive_fake.call_count, 1);
    ASSERT_EQ(xSemaphoreGive_fake.arg0_val, xSemaphoreTake_fake.arg0_val);
}

TEST(actuation_handler, ventilation_disable_override) {
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
    FFF_RESET_HISTORY();

    struct actuation_handler _hand = {
        NULL,
        NULL,
        0,
        0,
        pdFALSE,
        pdFALSE,
        ACTUATORS_ON
    };

    BaseType_t seq[] = {pdFALSE, pdTRUE};
    SET_RETURN_SEQ(xSemaphoreTake, seq, 2);

    actuators_ventilation_disable_override(&_hand);
    
    ASSERT_EQ(xSemaphoreTake_fake.call_count, 2);
    ASSERT_EQ(_hand.ventilation_overriden, pdFALSE);
    ASSERT_EQ(xSemaphoreGive_fake.call_count, 1);
    ASSERT_EQ(xSemaphoreGive_fake.arg0_val, xSemaphoreTake_fake.arg0_val);
}

TEST(actuation_handler, aircon_override_state) {
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
    RESET_FAKE(rc_servo_setPosition);
    FFF_RESET_HISTORY();

    struct actuation_handler _hand = {
        NULL,
        NULL,
        0,
        0,
        pdFALSE,
        pdFALSE,
        ACTUATORS_ON
    };

    BaseType_t seq[] = {pdFALSE, pdTRUE};
    SET_RETURN_SEQ(xSemaphoreTake, seq, 2);

    actuators_aircon_override_state(&_hand, AIRCON_COOL);
    
    ASSERT_EQ(xSemaphoreTake_fake.call_count, 2);
    ASSERT_EQ(_hand.aircon_overriden, pdTRUE);
    ASSERT_EQ(rc_servo_setPosition_fake.call_count, 1);
    ASSERT_EQ(rc_servo_setPosition_fake.arg1_val, AIRCON_COOL);
    ASSERT_EQ(xSemaphoreGive_fake.call_count, 1);
    ASSERT_EQ(xSemaphoreGive_fake.arg0_val, xSemaphoreTake_fake.arg0_val);
}

TEST(actuation_handler, aircon_disable_override) {
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
    FFF_RESET_HISTORY();

    struct actuation_handler _hand = {
        NULL,
        NULL,
        0,
        0,
        pdFALSE,
        pdFALSE,
        ACTUATORS_ON
    };

    BaseType_t seq[] = {pdFALSE, pdTRUE};
    SET_RETURN_SEQ(xSemaphoreTake, seq, 2);

    actuators_aircon_disable_override(&_hand);
    
    ASSERT_EQ(xSemaphoreTake_fake.call_count, 2);
    ASSERT_EQ(_hand.aircon_overriden, pdFALSE);
    ASSERT_EQ(xSemaphoreGive_fake.call_count, 1);
    ASSERT_EQ(xSemaphoreGive_fake.arg0_val, xSemaphoreTake_fake.arg0_val);
}