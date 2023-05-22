#pragma once
#include <stdint.h>

void rc_servo_initialise(void);
void rc_servo_setPosition(uint8_t servoNo, int8_t percent);