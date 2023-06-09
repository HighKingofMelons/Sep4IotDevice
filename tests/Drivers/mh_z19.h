
#pragma once
#include <stdint.h>
#include "serial.h"

typedef enum
{
    MHZ19_OK /**< Everything went well */
    ,
    MHZ19_NO_MEASSURING_AVAILABLE /**< No results are received from the sensor */
    ,
    MHZ19_NO_SERIAL /**< No serial driver is specified */
    ,
    MHZ19_PPM_MUST_BE_GT_999 /**< Span calibration is only allowed from ppm >= 1000 */
} mh_z19_returnCode_t;

mh_z19_returnCode_t mh_z19_getCo2Ppm(uint16_t *ppm);
void mh_z19_initialise(serial_comPort_t com_port);
mh_z19_returnCode_t mh_z19_takeMeassuring(void);