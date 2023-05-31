#pragma once
#include <actuation.h>
#include <co2_handler.h>
#include <temperature_handler.h>
#include <humidity_handler.h>

void update_aircon(actuation_handler_t self);
void update_vent(actuation_handler_t self);

struct actuation_handler {
    temperature_t temp_handler;
    humidity_t humid_handler;
    co2_handler_t co2_handler;

    SemaphoreHandle_t override_sema;
    SemaphoreHandle_t actuator_state_sema;
    BaseType_t ventilation_overriden;
    BaseType_t aircon_overriden;
    actuator_state_t actuator_state;
};