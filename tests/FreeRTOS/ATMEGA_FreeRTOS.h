#include <stdint.h>
#include "FreeRTOSConfig.h"
#define tskIDLE_PRIORITY ((UBaseType_t)0U)
#define pdTRUE ((BaseType_t)1)
#define pdFALSE ((BaseType_t)0)

typedef int TickType_t;
typedef int8_t BaseType_t;
typedef uint8_t UBaseType_t;
typedef struct TaskControlBlock_t *TaskHandle_t;
typedef void (*TaskFunction_t)(void *);

TickType_t pdMS_TO_TICKS(uint16_t numb);
