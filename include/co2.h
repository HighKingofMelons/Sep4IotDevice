/*
 * co2.h
 *
 * Created: 5/11/2023 3:16:53 PM
 *  Author: takat
 */

/** @file
 * @brief This is a co2 header file.
 *
 * It uses the mh_z19 driver in order to messure and use the data.
 */

#pragma once

typedef struct co2 *co2_t;
co2_t co2_create(TickType_t freequency);
void co2_mesure(void *pvParameters);
int16_t co2_get_latest_average_co2(co2_t self);
int initializeCo2Driver();
int makeOneCo2Mesurment(co2_t self);
void addCo2(co2_t self, int16_t co2);
void resetCo2Array(co2_t self);
void calculateCo2(co2_t self);
void co2_set_limits(co2_t self, int16_t maxLimit, int16_t minLimit);
int8_t co2_acceptability_status(co2_t self);
void co2_recordMeasurement(co2_t self, uint16_t ppm);
void co2_makeOneMesuremnt(co2_t self, uint16_t ppm);