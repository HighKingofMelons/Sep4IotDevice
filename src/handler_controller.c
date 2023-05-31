#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ATMEGA_FreeRTOS.h>
#include <task.h>

#include <stdio_driver.h>
#include <serial.h>

#include <status_leds.h>

#include "taskConfig.h"
#include "temperature.h"
#include "humidity.h"
#include "co2_handler.h"
#include "error_handler.h"
#include "actuation.h"
#include "LoRaWANHandler.h"
#include "handler_controller.h"

typedef struct handler_controller {
	TickType_t last_messure_circle_time;
	temperature_t temperature_handler;
    humidity_t humidity_handler;
	co2_handler_t co2_handler;
	error_handler_t error_handler;
	actuation_handler_t actuation_handler;
    lorawan_handler_t lorawan_handler;
    TaskHandle_t handler_controller_h;
} handler_controller_st;


handler_controller_t initialise_handler_controller();
void handler_controller_task(void *pvParameters);
void create_uplink_payload(handler_controller_t self);
void parse_downlink_payload(handler_controller_t self);

void init_and_run() {
    handler_controller_t self = initialise_handler_controller();
	xTaskCreate(
		handler_controller_task,
		"handler_controller",  // A name just for humans
		TASK_HANDLER_CONTROLLER_STACK,  // This stack size can be checked & adjusted by reading the Stack Highwater
		self, 
		TASK_HANDLER_CONTROLLER_PRIORITY,    // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		&(self->handler_controller_h));
}

void hander_controller_destroy(handler_controller_t self) {
	if (self->handler_controller_h != NULL) {
		vTaskDelete(self->handler_controller_h);
		self->handler_controller_h = NULL;
	}

	if (self != NULL) {
		free(self);
	}
}

void handler_controller_task(void *pvParameters) {
	handler_controller_t self = (handler_controller_t) pvParameters;
    
    // TODO: implement PIR, SOUND, LIGHT handlers
	error_handler_report(self->error_handler, ERROR_PIR);
	error_handler_report(self->error_handler, ERROR_SOUND);
	error_handler_report(self->error_handler, ERROR_LIGHT);

    for( ;; ) {
        xTaskDelayUntil(&(self->last_messure_circle_time), pdMS_TO_TICKS(MESURE_CIRCLE_FREAQUENCY));
        create_uplink_payload(self);;
		vTaskDelay(pdMS_TO_TICKS(30000UL));
        parse_downlink_payload(self);
		printf("HC hw: %i\n", uxTaskGetStackHighWaterMark(self->handler_controller_h));
    } 
}

handler_controller_t initialise_handler_controller() {
	// Make it possible to use stdio on COM port 0 (USB) on Arduino board - Setting 57600,8,N,1
	stdio_initialise(ser_USART0);
	
	status_leds_initialise(5); // Priority 5 for internal task

	const TickType_t measureCircleFreaquency = pdMS_TO_TICKS(MESURE_CIRCLE_FREAQUENCY); // Upload message every 5 minutes (300000 ms)
    TickType_t last_messure_circle_time = xTaskGetTickCount();
    handler_controller_t _new_handler_controller = calloc(sizeof(handler_controller_st), 1);
	_new_handler_controller->last_messure_circle_time = last_messure_circle_time;
	_new_handler_controller->temperature_handler = temperature_create(measureCircleFreaquency);
	_new_handler_controller->humidity_handler = humidity_create(measureCircleFreaquency);
	_new_handler_controller->error_handler = error_handler_init();
	_new_handler_controller->co2_handler = co2_create(_new_handler_controller->error_handler, last_messure_circle_time);
	_new_handler_controller->actuation_handler = actuation_handler_init(_new_handler_controller->temperature_handler, _new_handler_controller->humidity_handler);
	_new_handler_controller->lorawan_handler = lorawan_handler_create(last_messure_circle_time);
	_new_handler_controller->handler_controller_h = NULL;
    return _new_handler_controller;
}

void create_uplink_payload(handler_controller_t self) {
    // uint8_t pir = pir_handler_latest_average_pir(self->pir_handler); 
    // uint8_t error = error_handler_get_flags(self->error_handler); 
	uint8_t flags = error_handler_get_flags(self->error_handler); // uint8_t flags = pir | error;
	int16_t temp = temperature_get_latest_average_temperature(self->temperature_handler);
	uint8_t humidity = humidity_get_latest_average_humidity(self->humidity_handler);
	uint16_t co2_ppm = co2_get_latest_average_co2(self->co2_handler);
	uint16_t sound = 0; // uint16_t sound = sound_handler_latest_average_sound(self->light_handler); 
	uint16_t light = 0; // uint16_t light = light_handler_latest_average_light(self->sound_handler);
    uint8_t bytes[10];
	bytes[0] = flags;
	bytes[1] = temp >> 8;
	bytes[2] = temp & 0xFF;
	bytes[3] = humidity;
	bytes[4] = co2_ppm >> 8;
	bytes[5] = co2_ppm & 0xFF;
	bytes[6] = sound >> 8;
	bytes[7] = sound & 0xFF;
	bytes[8] = light >> 8;
	bytes[9] = light & 0xFF;
    lorawan_handler_set_uplink(self->lorawan_handler, bytes);
}

void parse_downlink_payload(handler_controller_t self) {
    uint8_t bytes[10];
	memset(bytes, 0, sizeof(uint8_t) * 10);
    if(!lorawan_handler_get_downlink(self->lorawan_handler, bytes)) {
		return; // no limits;
	}

	vTaskDelay(pdMS_TO_TICKS(5000));
	printf("Limits are being changed.\n");
	uint8_t flags = bytes[0];
	actuators_turn_on_off(self->actuation_handler, (flags & 0x80) == 0x80);
	vTaskDelay(pdMS_TO_TICKS(5000));
	uint8_t max_temp_limit = ((int16_t) bytes[1]) * 10;
	uint8_t min_temp_limit = ((int16_t) bytes[2]) * 10;
	temperature_set_limits(self->temperature_handler, max_temp_limit, min_temp_limit);
	vTaskDelay(pdMS_TO_TICKS(5000));
	uint8_t max_hum_limit = bytes[3];
	uint8_t min_hum_limit = bytes[4];
	humidity_set_limits(self->humidity_handler, max_hum_limit, min_hum_limit);
	vTaskDelay(pdMS_TO_TICKS(5000));
    // uint16_t max_co2_limit = (bytes[5] << 8) + bytes[6];
	// co2_set_limits(self->co2_handler, max_co2_limit, 0); TODO: this breaks down
	printf("Limits changed.\n");
}