/*
 * co2_handler.h
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
#include "error_handler.h"
/**
 * @brief Structure to store co2 information.
 *
 * It hold a 10 attributes and is needed for accesing and using co2 values.
 */
typedef struct co2 *co2_handler_t;
/**
 * @brief Create co2.
 *
 * @param freequency
 * @param lastMessureCircleTime
 * @return co2_t
 */
co2_handler_t co2_create(error_handler_t error_handler, TickType_t lastMessureCircleTime);
/**
 * @brief Get the latest average co2 value.
 *
 * @param self
 * @return int16_t
 */
uint16_t co2_get_latest_average_co2(co2_handler_t self);
/**
 * @brief Sets the co2 limits.
 *
 * @param self
 * @param maxLimit
 * @param minLimit
 */
void co2_set_limits(co2_handler_t self, uint16_t maxLimit, uint16_t minLimit);
/**
 * @brief Checks co2 acceptability statys and returns the status.
 *
 * @param self
 * @return int8_t
 */
int8_t co2_acceptability_status(co2_handler_t self);
/**
 * @brief Destroys co2.
 *
 * @param self
 */
void co2_handler_destroy(co2_handler_t self);