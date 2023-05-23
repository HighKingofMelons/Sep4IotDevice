/*
 * LoRaWANHandler.h
 *
 * Created: 19/05/2023 21.37.43
 *  Author: andre
 */ 

#include <ATMEGA_FreeRTOS.h>
#include "handlers.h"

typedef struct lorawan_handler* lorawan_handler_t;
lorawan_handler_t lorawan_handler_create(handlers_t handlers, TickType_t last_messure_circle_time);
