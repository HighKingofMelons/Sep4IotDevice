/*
* main.c
* Author : IHA
*
* Example main file including LoRaWAN setup
* Just for inspiration :)
*/

#include <stdio.h>
#include <avr/io.h>

#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <stdio_driver.h>
#include <serial.h>

 // Needed for LoRaWAN
#include <lora_driver.h>
#include <status_leds.h>

#include "temperature.h"
#include "humidity.h"
#include "co2.h"
#include "error.h"
#include "handlers.h"
#include "LoRaWANHandler.h"
#include "actuation.h"

void initialiseSystem();

/*-----------------------------------------------------------*/
void initialiseSystem()
{
	// Set output ports for leds used in the example
	DDRA |= _BV(DDA0) | _BV(DDA7);

	// Make it possible to use stdio on COM port 0 (USB) on Arduino board - Setting 57600,8,N,1
	stdio_initialise(ser_USART0);
	
	status_leds_initialise(5); // Priority 5 for internal task

	// Create temp TODO: restructure files to more logical and easier to draw class diagram 
	const TickType_t measureCircleFreaquency = pdMS_TO_TICKS(300000UL); // Upload message every 5 minutes (300000 ms)
	
	TickType_t last_messure_circle_time = xTaskGetTickCount();
	temperature_t temperature = temperature_create(measureCircleFreaquency);
	humidity_t humidity = humidity_create(measureCircleFreaquency);
	co2_t co2 = co2_create(measureCircleFreaquency);
	error_handler_t error = error_handler_init();
	handlers_t handlers = handlers_create(temperature, humidity, co2, error);
	actuation_handler_t act = actuation_handler_init(temperature, humidity);
	lorawan_handler_create(handlers, last_messure_circle_time);
}

/*-----------------------------------------------------------*/
int main(void)
{
	initialiseSystem(); // Must be done as the very first thing!!
	printf("Program Started!!\n");
	vTaskStartScheduler(); // Initialise and run the freeRTOS scheduler. Execution should never return from here.

	/* Replace with your application code */
	while (1)
	{
	}
}

