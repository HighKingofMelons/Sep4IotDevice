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
 * It hold a 10 attributes and is needed for accesing and using temperature values.
 */
typedef struct temperature* temperature_t;
/**
 * @brief Create temperature.
 *
 * @param mesureCirclefreequency
 * @return temperature_t
 */
temperature_t temperature_create(TickType_t mesureCircleFreequency);
/**
 * @brief Gets the latest average temperature value.
 *
 * @param self
 * @return int16_t
 */
int16_t temperature_get_latest_average_temperature(temperature_t self);
/**
 * @brief Sets the temperature limits.
 *
 * @param self
 * @param maxLimit
 * @param minLimit
 */
void temperature_set_limits(temperature_t self, int16_t maxLimit, int16_t minLimit);
/**
 * @brief Checks the temperature acceptability status and returns the status.
 *
 * @param self
 * @return int8_t
 */
int8_t temperature_acceptability_status(temperature_t self);
/**
 * @brief Destrys the temperature struct.
 *
 * @param self
 */
void temperature_destroy(temperature_t self);

