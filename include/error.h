#pragma once

#include <stdint.h>
#include <ATMEGA_FreeRTOS.h>

/*
    Error codes recignised by system.
    When adding new error codes remember to add relevant switch statements.
*/ 
typedef enum {
    ERROR_LORAWAN_NO_CONNECT = 001,
} error_code_t;

/*
    Intializes Error handler.
    Needs to be called before the scheduler is activated.
*/
void error_handler_init();

/*
    Queues up an error meassage at a given length.
        `error`: `error_code_t`
            If error code is not recignised 
        
        `length`: `TickType_t`
            The amount of ticks
*/
BaseType_t error_handler_queue_error(error_code_t error, TickType_t length);

error_code_t error_handler_get_payload_error();

