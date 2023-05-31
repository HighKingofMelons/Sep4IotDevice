/*
 * humidity_handler.h
 *
 * Created: 27/04/2023 11.42.59
 *  Author: andre
 */ 

#pragma once
#include "error_handler.h"

typedef struct humidity* humidity_t;
humidity_t humidity_create(error_handler_t error_handler, TickType_t lastMessureCircleTime);
uint8_t humidity_get_latest_average_humidity(humidity_t self);
void humidity_set_limits(humidity_t self, uint8_t maxLimit, uint8_t minLimit);
int8_t humidity_get_acceptability_status(humidity_t self);
void humidity_destroy(humidity_t self);

