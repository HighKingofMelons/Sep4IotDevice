#include "fakes.h"
DEFINE_FFF_GLOBALS;

// --------------------------
#include "display_7seg.h"
#include "ATMEGA_FreeRTOS.h"
#include "semphr.h"
#include "mh_z19.h"

// void display_7seg_initialise(void (*displayDoneCallBack)(void));
DEFINE_FAKE_VOID_FUNC(display_7seg_initialise, void*);
// void display_7seg_powerUp(void);
DEFINE_FAKE_VOID_FUNC(display_7seg_powerUp);
// void display_7seg_powerDown(void);
DEFINE_FAKE_VOID_FUNC(display_7seg_powerDown);


// void display_7seg_display(float value, uint8_t no_of_decimals);
DEFINE_FAKE_VOID_FUNC(display_7seg_display, float, uint8_t);
// void display_7seg_displayHex(char * hexString);
DEFINE_FAKE_VOID_FUNC(display_7seg_displayHex, char*);
// void display_7seg_displayErr(void);
DEFINE_FAKE_VOID_FUNC(display_7seg_displayErr);

// ------------------------
#include "queue.h"

// QueueHandle_t xQueueCreate( UBaseType_t uxQueueLength, UBaseType_t uxItemSize );
DEFINE_FAKE_VALUE_FUNC(QueueHandle_t, xQueueCreate, UBaseType_t, UBaseType_t);

// BaseType_t xQueueReceive( QueueHandle_t xQueue,
//                            void * const pvBuffer,
//                            TickType_t xTicksToWait );
DEFINE_FAKE_VALUE_FUNC(BaseType_t, xQueueReceive, QueueHandle_t, void * const, TickType_t);

// BaseType_t xQueueSend(  QueueHandle_t xQueue,
//                        const void * pvItemToQueue,
//                        TickType_t xTicksToWait );
DEFINE_FAKE_VALUE_FUNC(BaseType_t, xQueueSend, QueueHandle_t, const void *, TickType_t);


// ----------------------------
#include "task.h"

// BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
//                        const char * const pcName,     /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
//                        const uint16_t usStackDepth,
//                        void * const pvParameters,
//                        UBaseType_t uxPriority,
//                        TaskHandle_t * const pxCreatedTask );
DEFINE_FAKE_VALUE_FUNC(BaseType_t, xTaskCreate, TaskFunction_t, const char * const, const uint16_t, void * const, UBaseType_t, TaskHandle_t * const);

// void vTaskDelay( const TickType_t xTicksToDelay );
DEFINE_FAKE_VOID_FUNC(vTaskDelay, const TickType_t);

//co2
DEFINE_FAKE_VALUE_FUNC(TickType_t, pdMS_TO_TICKS, uint16_t);
//TickType_t pdMS_TO_TICKS(uint16_t numb);
DEFINE_FAKE_VALUE_FUNC(int, xSemaphoreTake, SemaphoreHandle_t, TickType_t)
DEFINE_FAKE_VOID_FUNC(xSemaphoreGive, SemaphoreHandle_t);

DEFINE_FAKE_VALUE_FUNC(SemaphoreHandle_t, xSemaphoreCreateMutex);
DEFINE_FAKE_VALUE_FUNC(TickType_t, xTaskGetTickCount);
DEFINE_FAKE_VALUE_FUNC(BaseType_t, xTaskDelayUntil, TickType_t *const, const TickType_t);
DEFINE_FAKE_VALUE_FUNC(mh_z19_returnCode_t, mh_z19_getCo2Ppm, int*);
//mh_z19_returnCode_t mh_z19_getCo2Ppm(int *ppm);
DEFINE_FAKE_VOID_FUNC(mh_z19_initialise, serial_comPort_t);
DEFINE_FAKE_VALUE_FUNC(mh_z19_returnCode_t, mh_z19_takeMeassuring);
