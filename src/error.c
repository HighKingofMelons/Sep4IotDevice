#include <stdlib.h>
#include <stdio.h>

#include <ATMEGA_FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>
#include <queue.h>
#include <display_7seg.h>

#include "error.h"

struct queue_item {
    error_code_t error;
    TickType_t length;
};

int is_initialized = 0;
QueueHandle_t queue_handle;

void error_handler_task(void *pvParameters) {
    
    while (1)
    {
        printf("Error Handler Task Start\n");
        struct queue_item item;
        if (pdFALSE == xQueueReceive(queue_handle, &item, 2000)) {       
            continue;
        }

        display_7seg_powerUp();

        switch (item.error)
        {
            case ERROR_LORAWAN_NO_CONNECT:
                display_7seg_displayHex("E001");
                printf("Error #E%c: LORAWAN_NO_CONNECT\n", item.error);
                break;
            
            default:
                display_7seg_displayErr();
                printf("Error #?: UNKNOWN ERROR\n");
                break;
        }

        vTaskDelay(item.length);
        display_7seg_powerDown();
    }
    
}

void error_handler_init() {
    printf("Error Handler Init");
    display_7seg_initialise(NULL);

    queue_handle = xQueueCreate(16, sizeof(struct queue_item));

    xTaskCreate(
        error_handler_task,
        "Error Handler",
        128,
        NULL,
        tskIDLE_PRIORITY+1,
        NULL
    );
}

BaseType_t error_handler_queue_error(error_code_t error, TickType_t length) {
    printf("Error Handler QUEUE");
    struct queue_item item = {error, length};

    return xQueueSend(queue_handle, &item, 1000);
}