/*
 * handlers.h
 *
 * Created: 19/05/2023 18.28.59
 *  Author: andre
 */ 


#pragma once

typedef struct handlers* handlers_t;

handlers_t handlers_create(temperature_t temperature, humidity_t humidity, co2_t co2, error_handler_t error_handler);

temperature_t get_temperature_handler(handlers_t self);

humidity_t get_humidity_handler(handlers_t self);

co2_t get_co2_handler(handlers_t self);

error_handler_t get_error_handler(handlers_t self);

