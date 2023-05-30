/** @file
 * @brief This is a error header file.
 *
 * It handles the actuation.
 */

#pragma once

#include <stdint.h>
#include <ATMEGA_FreeRTOS.h>

typedef enum {
    ERROR_TEMP  = 16,
    ERROR_HUMI  = 8,
    ERROR_CO2   = 4,
    ERROR_SOUND = 2,
    ERROR_PIR   = 1,
} error_component_t;

typedef uint8_t error_flags_t;
/**
 * @brief Structure to store error information.
 *
 * It hold a 5 attributes and is needed for accesing and using error.
 */
typedef struct error_handler *error_handler_t;
/**
 * @brief Initiatizes the error handler.
 *
 * @return error_handler_t
 */
error_handler_t error_handler_init();
/**
 * @brief The error handler report function.
 *
 * @param self
 * @param componenet
 * @return BaseType_t
 */
BaseType_t error_handler_report(error_handler_t self, error_component_t component);
/**
 * @brief The error handler revoke function.
 *
 * @param self
 * @param componenet
 * @return BaseType_t
 */
BaseType_t error_handler_revoke(error_handler_t self, error_component_t component);

/* The first 3 bits of the return are always 0 */
/**
 * @brief The error handler get flags function.
 *
 * @param self
 * @return error_flags_t
 */
error_flags_t error_handler_get_flags(error_handler_t self);
/**
 * @brief Destroys the error handler.
 *
 * @param self
 */
void error_handler_destroy(error_handler_t self);