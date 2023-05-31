#pragma once

typedef enum {
    ERROR_LIGHT  = 32,
    ERROR_TEMP  = 16,
    ERROR_HUMI  = 8,
    ERROR_CO2   = 4,
    ERROR_SOUND = 2,
    ERROR_PIR   = 1,
} error_component_t;

typedef uint8_t error_flags_t;
typedef struct error_handler *error_handler_t;

error_handler_t error_handler_init();

BaseType_t error_handler_report(error_handler_t self, error_component_t component);
BaseType_t error_handler_revoke(error_handler_t self, error_component_t component);

/* The first 3 bits of the return are always 0 */
error_flags_t error_handler_get_flags(error_handler_t self);

void error_handler_destroy(error_handler_t self);