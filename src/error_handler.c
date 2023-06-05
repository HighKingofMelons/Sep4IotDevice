#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include <ATMEGA_FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <display_7seg.h>

#include "taskConfig.h"
#include "error_handler.h"

struct error_handler {
    QueueHandle_t queue;
    SemaphoreHandle_t flag_semaphore;
    error_flags_t flags;
    BaseType_t display_on;
    uint8_t current_display;
    TaskHandle_t error_task_h;
};

enum request_type {
    ERROR_ENABLE,
    ERROR_DISABLE
};

struct error_item
{
    uint8_t component;
    enum request_type state;
};

void error_handler_task (void *pvArguments);

error_handler_t error_handler_init () {
    display_7seg_initialise(NULL);

    error_handler_t self = calloc(1, sizeof(struct error_handler));

    *self = (struct error_handler) {
        xQueueCreate(10, sizeof(struct error_item)),
        xSemaphoreCreateMutex(),
        0,
        pdFALSE,
        0,
        NULL
    };

    xSemaphoreGive(self->flag_semaphore);

    xTaskCreate (
        error_handler_task,
        "Error Handler Task",
        TASK_ERROR_STACK,
        self,
        TASK_ERROR_PRIORITY,
        &(self->error_task_h)
    );

    return self;
}

void update_flags(error_handler_t self) {
    struct error_item item;

    if (pdTRUE != xSemaphoreTake(self->flag_semaphore, 0))
        return;

    if (pdTRUE != xQueueReceive(self->queue, &item, 0)) {
        xSemaphoreGive(self->flag_semaphore);
        return;
    }

    if (item.state == ERROR_ENABLE) {
        self->flags = self->flags | item.component;

        if (DEBUG_ERROR) {
            printf("Error: %i flagged. Flags: %i\n", item.component, self->flags);
        }
    }
    else if (item.state == ERROR_DISABLE) {
        self->flags = ( self->flags & ( ~ item.component));

        if (DEBUG_ERROR) {
            printf("Error: %i unflagged. Flags: %i\n", item.component, self->flags);
        }
    }

    xSemaphoreGive(self->flag_semaphore);
}

BaseType_t error_handler_report(error_handler_t self, error_component_t component) {
    struct error_item item = (struct error_item) {
        component,
        ERROR_ENABLE
    };

    return xQueueSend(self->queue, &item, 0);
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
        if (!( (int) round(pow(2, self->current_display)) & self->flags)) {
            self->current_display = (self->current_display + 1) % 5;
            continue;
        }

        break;
    }
    
    if (DEBUG_ERROR) {
        printf("Display Error: %i\n", (int) round(pow(2, self->current_display)) & self->flags);
    }

    display_7seg_display((int) pow(2, self->current_display) & self->flags, 0);
    self->current_display = (self->current_display + 1) % 5;
    xSemaphoreGive(self->flag_semaphore);
}

error_flags_t error_handler_get_flags(error_handler_t self) {
    for (;;) {
        if (pdTRUE == xSemaphoreTake(self->flag_semaphore, 50))
            break;
    }
    error_flags_t _flags = self->flags;
    if (DEBUG_ERROR) {
        printf("Flags: %i\n", _flags);
    }
    xSemaphoreGive(self->flag_semaphore);
    return _flags;
}

void error_handler_task (void *pvArguments) {
    error_handler_t self = (error_handler_t) pvArguments;
    TickType_t lastWake = xTaskGetTickCount();

    for (;;)
    {
        update_flags(self);
        update_display(self);
        xTaskDelayUntil((TickType_t *const) &lastWake, pdMS_TO_TICKS(15000));
        lastWake = xTaskGetTickCount();
    }
}

void error_handler_destroy(error_handler_t self) {
    vTaskDelete(self->error_task_h);
    
    if (self->display_on)
        display_7seg_powerDown();

    vQueueDelete(self->queue);
    vSemaphoreDelete(self->flag_semaphore);
    free(self);
    self = NULL;
}
