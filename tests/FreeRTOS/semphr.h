#include "queue.h"
#include "ATMEGA_FreeRTOS.h"
typedef QueueHandle_t SemaphoreHandle_t;
BaseType_t xSemaphoreTake(SemaphoreHandle_t, TickType_t);
void xSemaphoreGive(SemaphoreHandle_t);
void vSemaphoreDelete(SemaphoreHandle_t);
SemaphoreHandle_t xSemaphoreCreateMutex();
