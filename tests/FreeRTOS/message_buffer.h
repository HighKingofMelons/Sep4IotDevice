#pragma once

#include <stddef.h>

typedef void * MessageBufferHandle_t;

MessageBufferHandle_t xMessageBufferCreate(long size);

size_t xMessageBufferReceive(MessageBufferHandle_t handle, void* payload, size_t size, TickType_t wait);
