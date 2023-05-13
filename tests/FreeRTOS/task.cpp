#include "../fff.h"
#include "task.h"
DEFINE_FFF_GLOBALS;
FAKE_VOID_FUNC(vTaskDelay, const TickType_t);
FAKE_VALUE_FUNC(TickType_t, xTaskGetTickCount );