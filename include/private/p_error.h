#pragma once
#include <semphr.h>

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

void update_flags(error_handler_t self);
void update_display(error_handler_t self);