/*
 * humidity.h
 *
 * Created: 27/04/2023 11.42.59
 *  Author: andre
 */ 

/*
 * humidity.h
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

/**
 * @brief Structure to store humidity information
 *
 * It hold a 10 attributes and is needed for accesing and using co2 values.
 */
typedef struct humidity* humidity_t;
humidity_t humidity_create(TickType_t mesureCircleFreequency);
uint8_t humidity_get_latest_average_humidity(humidity_t self);
void humidity_set_limits(humidity_t self, uint8_t maxLimit, uint8_t minLimit);
int8_t humidity_acceptability_status(humidity_t self);
void humidity_destroy(humidity_t self);

