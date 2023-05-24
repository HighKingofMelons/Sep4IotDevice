/*
 * co2.h
 *
 * Created: 5/11/2023 3:16:53 PM
 *  Author: takat
 */ 

#pragma once

typedef struct co2 *co2_c;
co2_c co2_create(TickType_t freequency);
void co2_mesure(void *pvParameters);
int16_t co2_get_latest_average_co2(co2_c self);
int initializeCo2Driver();
int makeOneCo2Mesurment(co2_c self);
void addCo2(co2_c self, int16_t co2);
void resetCo2Array(co2_c self);
void calculateCo2(co2_c self);
void co2_set_limits(co2_c self, int16_t maxLimit, int16_t minLimit);
int8_t co2_acceptability_status(co2_c self);