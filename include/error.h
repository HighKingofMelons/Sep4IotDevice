#pragma once

#include <stdint.h>
#include <ATMEGA_FreeRTOS.h>

typedef enum {
    ERROR_TEMP  = 128,
    ERROR_CO2   = 64,
    ERROR_HUMI  = 32,
    ERROR_SOUND = 16,
    ERROR_PIR   = 8,
} error_component_t;

typedef uint8_t error_flags_t;
typedef struct error_handler *error_handler_t;

error_handler_t error_handler_init();

BaseType_t error_handler_report(error_handler_t self, error_component_t component);
BaseType_t error_handler_revoke(error_handler_t self, error_component_t component);

BaseType_t error_handler_get_component_state(error_handler_t self, error_component_t component);

/* The first 3 bits of the return are always 0 */
error_flags_t error_handler_get_flags(error_handler_t self);