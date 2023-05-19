#pragma once

#include "queue.h"
#include "ATMEGA_FreeRTOS.h"

typedef QueueHandle_t SemaphoreHandle_t;

BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait );

void xSemaphoreGive(SemaphoreHandle_t xSemaphore);

SemaphoreHandle_t xSemaphoreCreateMutex(void);

void vSemaphoreDelete(SemaphoreHandle_t xSemaphore);
