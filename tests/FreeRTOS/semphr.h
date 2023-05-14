#pragma once

#include "queue.h"

typedef QueueHandle_t SemaphoreHandle_t;

BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait );

void xSemaphoreGive(SemaphoreHandle_t xSemaphore);

SemaphoreHandle_t xSemaphoreCreateMutex(void);