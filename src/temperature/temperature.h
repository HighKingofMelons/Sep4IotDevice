/*
 * temperature.h
 *
 * Created: 27/04/2023 11.42.59
 *  Author: andre
 */ 


#pragma once

typedef struct temperature* temperature_t;
temperature_t temperature_create(uint8_t portNo, TickType_t mesureCircleFreequency);
void temperature_mesure(void *pvParameters);
int16_t temperature_get_latest_average_temperature(temperature_t self);
int temaperature_destroy(temperature_t self);
