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

// co2 driver import
//#include <mh_z19.h>

 // Needed for LoRaWAN
#include <lora_driver.h>
#include <status_leds.h>

#include "co2/co2.h"
#include "temperature.h"
#include "error.h"


// Prototype for LoRaWAN handler
void lora_handler_initialise(UBaseType_t lora_handler_task_priority, co2_c co2, temperature_t temperature, error_handler_t error);

/*-----------------------------------------------------------*/
void initialiseSystem()
{
	// Set output ports for leds used in the example
	DDRA |= _BV(DDA0) | _BV(DDA7);

	// Make it possible to use stdio on COM port 0 (USB) on Arduino board - Setting 57600,8,N,1
	stdio_initialise(ser_USART0);

	// vvvvvvvvvvvvvvvvv BELOW IS LoRaWAN initialisation vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	// Status Leds driver
	status_leds_initialise(5); // Priority 5 for internal task
	// Initialise the LoRaWAN driver without down-link buffer
	lora_driver_initialise(1, NULL);
	
	// Create temp TODO: restructure files to more logical and easier to draw class diagram 
	const TickType_t measureCircleFreaquency = pdMS_TO_TICKS(300000UL); // Upload message every 5 minutes (300000 ms)
	
	error_handler_t error = error_handler_init();
	temperature_t temperature = temperature_create(measureCircleFreaquency); //TODO: change port number
	co2_c co2 = co2_create(22, measureCircleFreaquency);
	
	lora_handler_initialise(3, temperature, co2, error);
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

