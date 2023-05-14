#include "queue.h"
#include "ATMEGA_FreeRTOS.h"
typedef QueueHandle_t SemaphoreHandle_t;
int xSemaphoreTake(SemaphoreHandle_t, TickType_t);
void xSemaphoreGive(SemaphoreHandle_t);
SemaphoreHandle_t xSemaphoreCreateMutex();
