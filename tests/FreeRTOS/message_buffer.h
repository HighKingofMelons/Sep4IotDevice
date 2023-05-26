#pragma once

typedef void * MessageBufferHandle_t;


MessageBufferHandle_t xMessageBufferCreate(long size);

void xMessageBufferReceive(MessageBufferHandle_t handle, void* payload, long size, TickType_t wait)
