#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <ATMEGA_FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <display_7seg.h>

#include "error.h"

struct error_handler {
    QueueHandle_t queue;
    SemaphoreHandle_t flag_semaphore;
    error_flags_t flags;
    BaseType_t display_on;
    uint8_t current_display;
};

enum request_type {
    ERROR_ENABLE,
    ERROR_DISABLE
};

struct error_item
{
    error_component_t component;
    enum request_type state;
};

TaskHandle_t task_handle;

void error_handler_task (void *pvArguments);

error_handler_t error_handler_init () {
    display_7seg_initialise(NULL);

    error_handler_t _error_handler = calloc(1, sizeof(struct error_handler));

    *_error_handler = (struct error_handler) {
        xQueueCreate(10, sizeof(struct error_item)),
        xSemaphoreCreateMutex(),
        0,
        pdFALSE
    };

    xSemaphoreGive(_error_handler->flag_semaphore);

    xTaskCreate (
        error_handler_task,
        "Error Handler Task",
        configTIMER_TASK_STACK_DEPTH,
        _error_handler,
        tskIDLE_PRIORITY + 1,
        task_handle
    );

    return _error_handler;
}

void update_flags(error_handler_t self) {
    struct error_item *item = calloc(1, sizeof(struct error_item));

    if (pdTRUE != xSemaphoreTake(self->flag_semaphore, 0))
        return;

    if (pdTRUE != xQueueReceive(self->queue, item, 0)) {
        xSemaphoreGive(self->flag_semaphore);
        return;
    }

    if (item->state == ERROR_ENABLE) {
        //printf("Flags: %i, Component: %i, Result: %i\n", self->flags, item->component, self->flags | (error_flags_t) item->component);
        self->flags = self->flags | item->component;
    }
    else if (item->state == ERROR_DISABLE) {
        //printf("Flags: %i, Component: %i, Result: %i\n", self->flags, item->component, self->flags & ( ~ item->component));
        self->flags = ( self->flags & ( ~ item->component));
    }

    xSemaphoreGive(self->flag_semaphore);
}

BaseType_t error_handler_report(error_handler_t self, error_component_t component) {
    struct error_item *item = calloc(1, sizeof(struct error_item));
    *item = (struct error_item) {
        component,
        ERROR_ENABLE
    };

    return xQueueSend(self->queue, item, 0);
}

BaseType_t error_handler_revoke(error_handler_t self, error_component_t component) {
    struct error_item item = {
        component,
        ERROR_DISABLE
    };

    return xQueueSend(self->queue, &item, 0);
}

void update_display(error_handler_t self) {
    if (pdTRUE != xSemaphoreTake(self->flag_semaphore, pdMS_TO_TICKS(50)))
        return;
    
    if (!self->flags) {
        if (self->display_on)
            display_7seg_powerDown();
                    
        xSemaphoreGive(self->flag_semaphore);
        self->display_on = pdFALSE;
        return;
    }
        
    if (!self->display_on) {
        display_7seg_powerUp();
        self->display_on = pdTRUE;
    }

    for (;;)
    {
        // printf("%i %i %i\n", (int) pow(2, self->current_display), self->flags, (((int) pow(2, self->current_display)) & self->flags));

        if (!( (int) pow(2,  self->current_display) & self->flags)) {
            self->current_display = (self->current_display + 1) % 5;
            continue;
        }

        break;
    }
    
    display_7seg_display((int) pow(2, self->current_display) & self->flags, 0);
    self->current_display = (self->current_display + 1) % 5;
    xSemaphoreGive(self->flag_semaphore);
}

error_flags_t error_handler_get_flags(error_handler_t self) {
    for (;;) {
        if (pdTRUE == xSemaphoreTake(self->queue, 50))
            break;
    }
    error_flags_t _flags = self->flags;
    xSemaphoreGive(self->flag_semaphore);
    return _flags;
}

void error_handler_task (void *pvArguments) {
    struct error_handler *handler = pvArguments;
    TickType_t lastWake = xTaskGetTickCount();

    for (;;)
    {
        update_flags(handler);
        update_display(handler);
        xTaskDelayUntil((TickType_t *const) &lastWake, pdMS_TO_TICKS(500));
        lastWake = xTaskGetTickCount();
    }
}

void error_handler_destroy(error_handler_t self) {
    vTaskDelete(task_handle);
    
    if (self->display_on)
        display_7seg_powerDown();

    vQueueDelete(self->queue);
    vSemaphoreDelete(self->flag_semaphore);
    free(self);
    self = NULL;
}