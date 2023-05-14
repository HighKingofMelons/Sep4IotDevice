#pragma once

#include "ATMEGA_FreeRTOS.h"
#include "task.h"
#include "display_7seg.h"
#include "queue.h"

#include "fff.h"

// --------------------------

// void display_7seg_initialise(void (*displayDoneCallBack)(void));
DECLARE_FAKE_VOID_FUNC(display_7seg_initialise, void*);
// void display_7seg_powerUp(void);
DECLARE_FAKE_VOID_FUNC(display_7seg_powerUp);
// void display_7seg_powerDown(void);
DECLARE_FAKE_VOID_FUNC(display_7seg_powerDown);


// void display_7seg_display(float value, uint8_t no_of_decimals);
DECLARE_FAKE_VOID_FUNC(display_7seg_display, float, uint8_t);
// void display_7seg_displayHex(char * hexString);
DECLARE_FAKE_VOID_FUNC(display_7seg_displayHex, char*);
// void display_7seg_displayErr(void);
DECLARE_FAKE_VOID_FUNC(display_7seg_displayErr)

// ------------------------

// QueueHandle_t xQueueCreate( UBaseType_t uxQueueLength, UBaseType_t uxItemSize );
DECLARE_FAKE_VALUE_FUNC(QueueHandle_t, xQueueCreate, UBaseType_t, UBaseType_t);

// BaseType_t xQueueReceive( QueueHandle_t xQueue,
//                            void * const pvBuffer,
//                            TickType_t xTicksToWait );
DECLARE_FAKE_VALUE_FUNC(BaseType_t, xQueueReceive, QueueHandle_t, void * const, TickType_t);

// BaseType_t xQueueSend(  QueueHandle_t xQueue,
//                        const void * pvItemToQueue,
//                        TickType_t xTicksToWait );
DECLARE_FAKE_VALUE_FUNC(BaseType_t, xQueueSend, QueueHandle_t, const void *, TickType_t);


// ----------------------------

// BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
//                        const char * const pcName,     /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
//                        const uint16_t usStackDepth,
//                        void * const pvParameters,
//                        UBaseType_t uxPriority,
//                        TaskHandle_t * const pxCreatedTask );
DECLARE_FAKE_VALUE_FUNC(BaseType_t, xTaskCreate, TaskFunction_t, const char * const, const uint16_t, void * const, UBaseType_t, TaskHandle_t * const);

// void vTaskDelay( const TickType_t xTicksToDelay );
DECLARE_FAKE_VOID_FUNC(vTaskDelay, const TickType_t);