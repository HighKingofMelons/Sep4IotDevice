#pragma once
#include <ATMEGA_FreeRTOS.h>

#include "temperature.h"
#include "humidity.h"

typedef struct actuation_handler *actuation_handler_t;

actuation_handler_t actuation_handler_init(temperature_t temperature, humidity_t humidity);
void actuation_handler_destroy(actuation_handler_t victim);

typedef enum {
    VENT_ON     =  100,
    VENT_OFF    = -100
} vent_state_t;
/*
    Takes pdTRUE for ON and pdFALSE for OFF
*/
void actuators_ventilation_override_state(actuation_handler_t self, vent_state_t state);
void actuators_ventilation_disable_override(actuation_handler_t self);

typedef enum {
    AIRCON_HEAT =  100,
    AIRCON_COOL = -100,
    AIRCON_OFF  =  50
} aircon_state_t;

void actuators_aircon_override_state(actuation_handler_t self, aircon_state_t state);
void actuators_aircon_disable_override(actuation_handler_t self);