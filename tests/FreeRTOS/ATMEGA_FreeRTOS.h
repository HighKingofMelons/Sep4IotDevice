#include <stdint.h>
#define tskIDLE_PRIORITY ((UBaseType_t)0U)
#define configMINIMAL_STACK_SIZE (192)
#define pdTRUE ((BaseType_t)1)

typedef int TickType_t;


TickType_t pdMS_TO_TICKS(uint16_t numb);