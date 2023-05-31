/*
 * co2_handler.h
 *
 * Created: 5/11/2023 3:16:53 PM
 *  Author: takat
 */ 

#pragma once
#include "error.h"

typedef struct co2 *co2_handler_t;
co2_handler_t co2_create(error_handler_t error_handler, TickType_t freequency);
uint16_t co2_get_latest_average_co2(co2_handler_t self);
void co2_set_limits(co2_handler_t self, uint16_t maxLimit, uint16_t minLimit);
int8_t co2_acceptability_status(co2_handler_t self);
void co2_handler_destroy(co2_handler_t self);