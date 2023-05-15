#include <stdlib.h>
#include <stdio.h>

#include <ATMEGA_FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <display_7seg.h>

#include "error.h"
#include "private/p_error.h"

void error_handler_task (void *pvArguments);

error_handler_t error_handler_init () {
    display_7seg_initialise(NULL);

    error_handler_t _error_handler = {
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
        NULL 
    );
}

void update_flags(error_handler_t self) {
    struct error_item item = { NULL, NULL };

    if (pdTRUE != xSemaphoreTake(self->flag_semaphore, 0))
        return;

    if (pdTRUE != xQueueReceive(self->queue, &item, 0)) {
        xSemaphoreGive(self->flag_semaphore);
        return;
    }

    if (item.state == ERROR_ENABLE) {
        printf("Flags: %i, Component: %i, Result: %i\n", self->flags, item.component, self->flags & (error_flags_t) item.component);
        self->flags = self->flags & (error_flags_t) item.component;
    }
    else if (item.state == ERROR_DISABLE) {
        printf("Flags: %i, Component: %i, Result: %i\n", self->flags, item.component, self->flags ^ (error_flags_t) item.component);
        self->flags = self->flags ^ (error_flags_t) item.component;
    }

    xSemaphoreGive(self->flag_semaphore);
}

BaseType_t error_handler_report(error_handler_t self, error_component_t component) {
    struct error_item item = {
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
    if (!self->flags) {
        if (!self->display_on)
            return;
        
        display_7seg_powerDown();
        self->display_on = pdFALSE;
        return;
    }
        
    if (!self->display_on) {
        display_7seg_powerUp();
        self->display_on = pdTRUE;
    }
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
    error_handler_t self = pvArguments;

    for (;;)
    {
        void update_flags(self);


    }
    
}