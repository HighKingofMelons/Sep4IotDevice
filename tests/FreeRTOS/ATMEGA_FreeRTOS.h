#pragma once
#include <stdint.h>
#include <FreeRTOSConfig.h>

typedef uint8_t UBaseType_t;
typedef int8_t BaseType_t;

typedef uint32_t TickType_t;

#define pdFALSE                                  ( ( BaseType_t ) 0 )
#define pdTRUE                                   ( ( BaseType_t ) 1 )

#define pdPASS                                   ( pdTRUE )
#define pdFAIL                                   ( pdFALSE )