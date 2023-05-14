<<<<<<< HEAD
#include <stdint.h>
#define tskIDLE_PRIORITY ((UBaseType_t)0U)
#define configMINIMAL_STACK_SIZE (192)
#define pdTRUE ((BaseType_t)1)

typedef int TickType_t;


TickType_t pdMS_TO_TICKS(uint16_t numb);
=======
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
>>>>>>> 44e643662981084d7ba63ff8159fc0b32291d218
