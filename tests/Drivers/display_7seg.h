#pragma once
#include <stdint.h>

void display_7seg_initialise(void*);
void display_7seg_display(float value, uint8_t no_of_decimals);
void display_7seg_displayHex(char * hexString);
void display_7seg_displayErr(void);
void display_7seg_powerUp(void);
void display_7seg_powerDown(void);
