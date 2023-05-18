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


#pragma once

typedef struct humidity* humidity_t;
humidity_t humidity_create(TickType_t mesureCircleFreequency);
uint8_t humidity_get_latest_average_humidity(humidity_t self);
void humidity_set_limits(humidity_t self, uint8_t maxLimit, uint8_t minLimit);
int8_t humidity_acceptability_status(humidity_t self);
void humidity_destroy(humidity_t self);

