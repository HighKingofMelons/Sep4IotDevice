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
BaseType_t xTaskDelayUntil(TickType_t *const pxPreviousWakeTime,
                           const TickType_t xTimeIncrement);

void vTaskDelete( TaskHandle_t xTaskToDelete );
