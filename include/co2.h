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

/**
 * @brief Structure to store co2 information
 *  
 * It hold a 10 attributes and is needed for accesing and using co2 values.
 */
typedef struct co2 *co2_t;

/**
 * @brief Create co2.
 *
 * @param freequency
 * @return co2_t
 */
co2_t co2_create(TickType_t freequency);
/**
 * @brief Messure co2.
 *
 * @param pvParameters
 */
void co2_mesure(void *pvParameters);
/**
 * @brief Get the latest average co2 value.
 *
 * @param self
 * @return int16_t
 */
int16_t co2_get_latest_average_co2(co2_t self);
/**
 * @brief Initializes the co2 driver.
 *
 * @return int
 */
int initializeCo2Driver();
/**
 * @brief Makes one co2 measurement.
 *
 * @param self
 * @return int
 */
int makeOneCo2Mesurment(co2_t self);
/**
 * @brief Add co2 value to the co2.
 *
 * @param self
 * @param co2
 */
void addCo2(co2_t self, int16_t co2);
/**
 * @brief Resets the co2 array.
 *
 * @param self
 */
void resetCo2Array(co2_t self);
/**
 * @brief Calculates co2.
 *
 * @param self
 */
void calculateCo2(co2_t self);
/**
 * @brief Sets the co2 limits.
 *
 * @param self
 * @param maxLimit
 * @param minLimit
 */
void co2_set_limits(co2_t self, int16_t maxLimit, int16_t minLimit);
/**
 * @brief Checks co2 acceptability statys and returns the status.
 *
 * @param self
 * @return int8_t
 */
int8_t co2_acceptability_status(co2_t self);
/**
 * @brief Records the co2 measurement.
 *
 * @param self
 * @param ppm
 */
void co2_recordMeasurement(co2_t self, uint16_t ppm);
/**
 * @brief Makes one co2 measurement.
 *
 * @param self
 * @param ppm
 */
void co2_makeOneMesuremnt(co2_t self, uint16_t ppm);