/*
 * temperature_handler.h
 *
 * Created: 27/04/2023 11.42.59
 *  Author: andre
 */ 

#pragma once
#include "error_handler.h"

typedef struct temperature* temperature_t;
temperature_t temperature_create(error_handler_t error_handler, TickType_t last_meassure_circle_time);
int16_t temperature_get_latest_average_temperature(temperature_t self);
void temperature_set_limits(temperature_t self, int16_t maxLimit, int16_t minLimit);
int8_t temperature_get_acceptability_status(temperature_t self);
void temperature_destroy(temperature_t self);
