/*
 * humidity_handler.h
 *
 * Created: 27/04/2023 11.42.59
 *  Author: andre
 */

/** @file
 * @brief This is a humidity header file.
 *
 * It uses the HIH8120 driver in order to messure and use the data.
 */
#pragma once
#include "error_handler.h"
/**
 * @brief Structure to store humidity information.
 *
 * It hold a 10 attributes and is needed for accesing and using humidity values.
 */
typedef struct humidity* humidity_t;
/**
 * @brief Create humidity.
 *
 * @param mesureCirclefreequency
 * @param lastMessureCircleTime
 * @return humidity_t
 */
humidity_t humidity_create(error_handler_t error_handler, TickType_t lastMessureCircleTime);
/**
 * @brief Gets the latest average humidity value.
 *
 * @param self
 * @return uint8_t
 */
uint8_t humidity_get_latest_average_humidity(humidity_t self);
/**
 * @brief Sets the humidity limits.
 *
 * @param self
 * @param maxLimit
 * @param minLimit
 */
void humidity_set_limits(humidity_t self, uint8_t maxLimit, uint8_t minLimit);
/**
 * @brief Checks the humidity acceptability status and returns the status.
 *
 * @param self
 * @return int8_t
 */
int8_t humidity_get_acceptability_status(humidity_t self);
/**
 * @brief Destrys the humidity struct.
 *
 * @param self
 */
void humidity_destroy(humidity_t self);

