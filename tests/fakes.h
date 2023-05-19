#pragma once

#include "ATMEGA_FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "display_7seg.h"
#include "queue.h"
#include "semphr.h"
#include "mh_z19.h"
#include "hih8120.h"
#include "rc_servo.h"

#include "fff.h"

// ---------------------------- ~ DISPLAY_7SEG ~ ---------------------------------------

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

// ---------------------------- ~ QUEUE ~ ---------------------------------------

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

// void vQueueDelete( QueueHandle_t xQueue );
DECLARE_FAKE_VOID_FUNC(vQueueDelete, QueueHandle_t);

// ---------------------------- ~ TASK ~ ---------------------------------------

// BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
//                        const char * const pcName,     /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
//                        const uint16_t usStackDepth,
//                        void * const pvParameters,
//                        UBaseType_t uxPriority,
//                        TaskHandle_t * const pxCreatedTask );


DECLARE_FAKE_VALUE_FUNC(BaseType_t, xTaskCreate, TaskFunction_t, const char * const, const uint16_t, void * const, UBaseType_t, TaskHandle_t * const);

// void vTaskDelay( const TickType_t xTicksToDelay );
DECLARE_FAKE_VOID_FUNC(vTaskDelay, const TickType_t);

// BaseType_t xTaskDelayUntil(TickType_t * const pxPreviousWakeTime, const TickType_t xTimeIncrement);
DECLARE_FAKE_VALUE_FUNC(BaseType_t, xTaskDelayUntil, TickType_t * const, const TickType_t);

//TickType_t xTaskGetTickCount( void )
DECLARE_FAKE_VALUE_FUNC(TickType_t, xTaskGetTickCount);

//void vTaskDelete( TaskHandle_t xTaskToDelete )
DECLARE_FAKE_VOID_FUNC(vTaskDelete, TaskHandle_t);

// ---------------------------- ~ SEMPHR ~ ---------------------------------------

// BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait );
DECLARE_FAKE_VALUE_FUNC(BaseType_t, xSemaphoreTake, SemaphoreHandle_t, TickType_t);

// void xSemaphoreGive(SemaphoreHandle_t xSemaphore);
DECLARE_FAKE_VOID_FUNC(xSemaphoreGive, SemaphoreHandle_t);

// void xSemaphoreCreateMutex(void);
DECLARE_FAKE_VALUE_FUNC(SemaphoreHandle_t, xSemaphoreCreateMutex);

//void xSemaphoreDelete(SemaphoreHandle_t xSemaphore);
DECLARE_FAKE_VOID_FUNC(vSemaphoreDelete, SemaphoreHandle_t);

// ---------------------------- ~ HIH8120 ~ ---------------------------------------

// BaseType_t hih8120_isReady(void);
DECLARE_FAKE_VALUE_FUNC(BaseType_t, hih8120_isReady);

// hih8120_driverReturnCode_t hih8120_initialise(void); 
DECLARE_FAKE_VALUE_FUNC(hih8120_driverReturnCode_t, hih8120_initialise);

// hih8120_driverReturnCode_t hih8120_destroy(void);
DECLARE_FAKE_VALUE_FUNC(hih8120_driverReturnCode_t, hih8120_destroy);

// hih8120_driverReturnCode_t hih8120_wakeup(void);
DECLARE_FAKE_VALUE_FUNC(hih8120_driverReturnCode_t, hih8120_wakeup);

// hih8120_driverReturnCode_t hih8120_measure(void);
DECLARE_FAKE_VALUE_FUNC(hih8120_driverReturnCode_t, hih8120_measure);

// int16_t hih8120_getTemperature_x10(void);
DECLARE_FAKE_VALUE_FUNC(int16_t, hih8120_getTemperature_x10);

// uint16_t hih8120_getTemperature_x10(void);
DECLARE_FAKE_VALUE_FUNC(uint16_t, hih8120_getHumidityPercent_x10);


// ---------------------------- ~ MH_Z19 ~ ---------------------------------------

DECLARE_FAKE_VALUE_FUNC(mh_z19_returnCode_t, mh_z19_getCo2Ppm, int *);

DECLARE_FAKE_VOID_FUNC(mh_z19_initialise, serial_comPort_t);

DECLARE_FAKE_VALUE_FUNC(mh_z19_returnCode_t, mh_z19_takeMeassuring);

// ---------------------------- ~ RC_SERVO ~ ---------------------------------------

// void rc_servo_initialise(void);
DECLARE_FAKE_VOID_FUNC(rc_servo_initialise);

// void rc_servo_setPosition(uint8_t servoNo, int8_t percent);
DECLARE_FAKE_VOID_FUNC(rc_servo_setPosition, uint8_t, int8_t);