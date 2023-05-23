/*
 * sensorHandlers.c
 *
 * Created: 19/05/2023 18.30.59
 *  Author: andre
 */ 

#include <stdlib.h>

#include <ATMEGA_FreeRTOS.h>

#include "temperature.h"
#include "humidity.h"
#include "co2.h"
#include "error.h"
#include "handlers.h"

typedef struct handlers {
	temperature_t temperature;
    humidity_t humidity;
    co2_t co2;
    error_handler_t error_handler;
} handlers_st;

handlers_t handlers_create(temperature_t temperature, humidity_t humidity, co2_t co2, error_handler_t error_handler) {
    handlers_t new_handlers = calloc(sizeof(handlers_st), 1);
    new_handlers->temperature = temperature;
    new_handlers->humidity = humidity;
    new_handlers->co2 = co2;
    new_handlers->error_handler = error_handler;
    return new_handlers;
}

temperature_t get_temperature_handler(handlers_t self) {
    return self->temperature;
}

humidity_t get_humidity_handler(handlers_t self) {
    return self->humidity;
}

co2_t get_co2_handler(handlers_t self) {
    return self->co2;
}

error_handler_t get_error_handler(handlers_t self) {
    return self->error_handler;
}



