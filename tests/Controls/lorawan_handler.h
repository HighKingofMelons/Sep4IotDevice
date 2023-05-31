#pragma once
#include <lorawan_handler.h>

void lora_uplink_task_(uint8_t test, void *pvParameters);
lora_driver_returnCode_t lora_upload_uplink(lorawan_handler_t self);