<<<<<<< HEAD
#include "ATMEGA_FreeRTOS.h"
#include <stdint.h>


typedef int8_t BaseType_t;
typedef uint8_t UBaseType_t;
typedef struct TaskControlBlock_t *TaskHandle_t;
typedef void (*TaskFunction_t)(void *);
#define configSTACK_DEPTH_TYPE uint16_t

void vTaskDelay(const TickType_t xTicksToDelay);
TickType_t xTaskGetTickCount(void);
BaseType_t xTaskDelayUntil(TickType_t *const pxPreviousWakeTime,
                           const TickType_t xTimeIncrement);
BaseType_t xTaskCreate(TaskFunction_t pxTaskCode,
                       const char *const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                       const configSTACK_DEPTH_TYPE usStackDepth,
                       void *const pvParameters,
                       UBaseType_t uxPriority,
                       TaskHandle_t *const pxCreatedTask);
=======
#pragma once
#include <ATMEGA_FreeRTOS.h>

#define tskIDLE_PRIORITY    ( ( UBaseType_t ) 0U )

struct TaskControlBlock_t;
typedef struct TaskControlBlock_t* TaskHandle_t;
typedef void (* TaskFunction_t)( void * );

BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                        const char * const pcName,     /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                        const uint16_t usStackDepth,
                        void * const pvParameters,
                        UBaseType_t uxPriority,
                        TaskHandle_t * const pxCreatedTask );

void vTaskDelay( const TickType_t xTicksToDelay );
>>>>>>> 44e643662981084d7ba63ff8159fc0b32291d218
