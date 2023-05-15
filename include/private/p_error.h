#pragma once
#include "error.h"

struct error_handler {
    QueueHandle_t queue;
    SemaphoreHandle_t flag_semaphore;
    error_flags_t flags = 0;
    BaseType_t display_on = pdFALSE;
}

enum request_type {
    ERROR_ENABLE,
    ERROR_DISABLE
};

struct error_item
{
    error_component_t component;
    enum request_type state;
};