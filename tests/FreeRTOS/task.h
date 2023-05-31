#pragma once 

#define pdMS_TO_TICKS( xTimeInMs )   xTimeInMs

#define configSTACK_DEPTH_TYPE uint16_t

BaseType_t xTaskCreate(TaskFunction_t pxTaskCode,
                       const char *const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                       const configSTACK_DEPTH_TYPE usStackDepth,
                       void *const pvParameters,
                       UBaseType_t uxPriority,
                       TaskHandle_t *const pxCreatedTask);
                       
void vTaskDelay( const TickType_t xTicksToDelay );

BaseType_t xTaskDelayUntil(TickType_t * const pxPreviousWakeTime, const TickType_t xTimeIncrement);

TickType_t xTaskGetTickCount( void );

void vTaskDelete( TaskHandle_t xTaskToDelete );

void taskYIELD(void);

UBaseType_t uxTaskGetStackHighWaterMark( TaskHandle_t xTask );
