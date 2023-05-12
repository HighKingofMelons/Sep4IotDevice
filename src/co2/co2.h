/*
 * co2.h
 *
 * Created: 5/11/2023 3:16:53 PM
 *  Author: takat
 */ 


#pragma once

typedef struct co2* co2_c;
co2_c co2_create(uint8_t portNo, TickType_t mesureCircleFreequency);
void co2_mesure(void *pvParameters);
int16_t co2_get_latest_average_co2(co2_c self);