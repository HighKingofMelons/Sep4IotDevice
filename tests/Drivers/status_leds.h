#pragma once

typedef enum {
	led_ST1 = 0		/**< Status led ST1 (RED)*/
	,led_ST2		/**< Status led ST2 (GREEN)*/
	,led_ST3		/**< Status led ST3 (YELLOW)*/
	,led_ST4		/**< Status led ST4 (BLUE)*/
} status_leds_t;


void status_leds_initialise(UBaseType_t ledTaskPriority);

void status_leds_longPuls(status_leds_t led);

void status_leds_shortPuls(status_leds_t led);

void status_leds_slowBlink(status_leds_t led);

void status_leds_fastBlink(status_leds_t led);

void status_leds_ledOn(status_leds_t led);

void status_leds_ledOff(status_leds_t led);