/*
 * temperature.h
 *
 * Created: 27/04/2023 11.42.59
 *  Author: andre
 */

/** @file
 * @brief This is a temperature header file.
 *
 * It uses the HIH8120 driver in order to messure and use the data.
 */
#pragma once

/**
 * @brief Structure to store humidity information
 *
 * It hold a 10 attributes and is needed for accesing and using co2 values.
 */
typedef struct temperature* temperature_t;
temperature_t temperature_create(TickType_t mesureCircleFreequency);
int16_t temperature_get_latest_average_temperature(temperature_t self);
void temperature_set_limits(temperature_t self, int16_t maxLimit, int16_t minLimit);
int8_t temperature_acceptability_status(temperature_t self);
void temperature_destroy(temperature_t self);

