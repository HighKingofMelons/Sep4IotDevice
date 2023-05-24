/*
* loraWANHandler.c
*
* Created: 12/04/2019 10:09:05
*  Author: IHA
*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <ATMEGA_FreeRTOS.h>
#include <lora_driver.h>
#include <status_leds.h>
#include <message_buffer.h>
#include <semphr.h>

#include "LoRaWAN.h"
#include "taskConfig.h"
#include "temperature.h"
#include "humidity.h"
#include "co2.h"
#include "error.h"
#include "handlers.h"
#include "LoRaWANHandler.h"

lorawan_handler_t initialize_lorawan_handler(handlers_t handlers, TickType_t last_messure_circle_time);
void _lora_setup(void);
void lora_uplink_task(void *pvParameters );
void lora_downlink_task(void *pvParameters);

static MessageBufferHandle_t downlink_message_buffer_handle;

static TaskHandle_t loraUplinkTask = NULL;
static TaskHandle_t loraDownlinkTask = NULL;

typedef struct lorawan_handler {
	handlers_t handlers;
	TickType_t last_messure_circle_time_uplink;
	TickType_t last_messure_circle_time_downlink;
} lorawan_handler_st;

lorawan_handler_t lorawan_handler_create(handlers_t handlers, TickType_t last_messure_circle_time)
{
	downlink_message_buffer_handle = xMessageBufferCreate(sizeof(lora_driver_payload_t) * 2); // Here I make room for two downlink messages in the message buffer
	lora_driver_initialise(1, downlink_message_buffer_handle);

	lorawan_handler_t _new_lorawan_handler = initialize_lorawan_handler(handlers, last_messure_circle_time);

	xTaskCreate(
		lora_uplink_task,
		"LoRaWAN_uplink",  // A name just for humans
		TASK_LORA_UPLINK_STACK,  // This stack size can be checked & adjusted by reading the Stack Highwater
		_new_lorawan_handler, // TODO: test if this works
		TASK_LORA_UPLINK_PRIORITY,    // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		&loraUplinkTask);

	return _new_lorawan_handler;
}

lorawan_handler_t initialize_lorawan_handler(handlers_t handlers, TickType_t last_messure_circle_time) {
	lorawan_handler_t _new_lorawan_handler = calloc(sizeof(lorawan_handler_st), 1);
	_new_lorawan_handler->handlers = handlers;
	_new_lorawan_handler->last_messure_circle_time_uplink = last_messure_circle_time;
	_new_lorawan_handler->last_messure_circle_time_downlink = last_messure_circle_time;
	return _new_lorawan_handler;
}

void _lora_setup(void)
{	
	char _out_buf[20];
	lora_driver_returnCode_t rc;
	status_leds_slowBlink(led_ST2); // OPTIONAL: Led the green led blink slowly while we are setting up LoRa

	// Factory reset the transceiver
	printf("FactoryReset >%s<\n", lora_driver_mapReturnCodeToText(lora_driver_rn2483FactoryReset()));
	
	// Configure to EU868 LoRaWAN standards
	printf("Configure to EU868 >%s<\n", lora_driver_mapReturnCodeToText(lora_driver_configureToEu868()));

	// Get the transceivers HW EUI
	rc = lora_driver_getRn2483Hweui(_out_buf);
	printf("Get HWEUI >%s<: %s\n",lora_driver_mapReturnCodeToText(rc), _out_buf);

	// Set the HWEUI as DevEUI in the LoRaWAN software stack in the transceiver
	printf("Set DevEUI: %s >%s<\n", _out_buf, lora_driver_mapReturnCodeToText(lora_driver_setDeviceIdentifier(_out_buf)));

	// Set Over The Air Activation parameters to be ready to join the LoRaWAN
	printf("Set OTAA Identity appEUI:%s appKEY:%s devEUI:%s >%s<\n", LORA_appEUI, LORA_appKEY, _out_buf, lora_driver_mapReturnCodeToText(lora_driver_setOtaaIdentity(LORA_appEUI,LORA_appKEY,_out_buf)));

	// Save all the MAC settings in the transceiver
	printf("Save mac >%s<\n",lora_driver_mapReturnCodeToText(lora_driver_saveMac()));

	// Enable Adaptive Data Rate
	printf("Set Adaptive Data Rate: ON >%s<\n", lora_driver_mapReturnCodeToText(lora_driver_setAdaptiveDataRate(LORA_ON)));

	// Set receiver window1 delay to 500 ms - this is needed if down-link messages will be used
	printf("Set Receiver Delay: %d ms >%s<\n", 500, lora_driver_mapReturnCodeToText(lora_driver_setReceiveDelay(500)));

	// Join the LoRaWAN
	uint8_t maxJoinTriesLeft = 10;
	
	do {
		rc = lora_driver_join(LORA_OTAA);
		printf("Join Network TriesLeft:%d >%s<\n", maxJoinTriesLeft, lora_driver_mapReturnCodeToText(rc));

		if ( rc != LORA_ACCEPTED)
		{
			// Make the red led pulse to tell something went wrong
			status_leds_longPuls(led_ST1); // OPTIONAL
			// Wait 5 sec and lets try again
			vTaskDelay(pdMS_TO_TICKS(5000UL));
		}
		else
		{
			break;
		}
	} while (--maxJoinTriesLeft);

	if (rc == LORA_ACCEPTED)
	{
		// Connected to LoRaWAN :-)
		// Make the green led steady
		status_leds_ledOn(led_ST2); // OPTIONAL
	}
	else
	{
		// Something went wrong
		// Turn off the green led
		status_leds_ledOff(led_ST2); // OPTIONAL
		// Make the red led blink fast to tell something went wrong
		status_leds_fastBlink(led_ST1); // OPTIONAL

		// Lets stay here
		while (1)
		{
			taskYIELD();
		}
	}
}

/*-----------------------------------------------------------*/
void lora_uplink_task( void *pvParameters )
{
	lorawan_handler_t self = (lorawan_handler_t) pvParameters;
	handlers_t handlers = self->handlers;
	error_handler_t error_handler = get_error_handler(handlers);
	temperature_t temperature_handler = get_temperature_handler(handlers);
	humidity_t humidity_handler = get_humidity_handler(handlers);
	co2_t co2_handler = get_co2_handler(handlers);

	error_handler_report(error_handler, ERROR_PIR);
	error_handler_report(error_handler, ERROR_SOUND);

	// Hardware reset of LoRaWAN transceiver
	lora_driver_resetRn2483(1);
	vTaskDelay(2);
	lora_driver_resetRn2483(0);
	// Give it a chance to wakeup
	vTaskDelay(150);

	lora_driver_flushBuffers(); // get rid of first version string from module after reset!

	_lora_setup();
	
	lora_driver_payload_t _uplink_payload;
	_uplink_payload.len = 10;
	_uplink_payload.portNo = 1;

	uint8_t flags;
	uint8_t max_temp_limit;
	uint8_t min_temp_limit;
	uint8_t max_hum_limit;
	uint8_t min_hum_limit;
	uint16_t max_co2_limit;
	lora_driver_payload_t _downlink_payload;
	const TickType_t xFrequency = pdMS_TO_TICKS(MESURE_CIRCLE_FREAQUENCY); // Upload message every 5 minutes (300000 ms)

	for(;;) {	
    	UBaseType_t uxHighWaterMark;
		uxHighWaterMark = uxTaskGetStackHighWaterMark( loraUplinkTask );
		printf("Lora uplink uxHighWaterMark1: %i\n", uxHighWaterMark);
		xTaskDelayUntil(&(self->last_messure_circle_time_uplink), xFrequency);
		flags = error_handler_get_flags(error_handler); 
		int16_t temp = temperature_get_latest_average_temperature(temperature_handler);
		uint8_t humidity = humidity_get_latest_average_humidity(humidity_handler);
		uint16_t co2_ppm = co2_get_latest_average_co2(co2_handler);
		uint16_t sound = 0; // Dummy sound
		uint16_t light = 0; // Dummy lux
		_uplink_payload.bytes[0] = flags;
		_uplink_payload.bytes[1] = temp >> 8;
		_uplink_payload.bytes[2] = temp & 0xFF;
		_uplink_payload.bytes[3] = humidity;
		_uplink_payload.bytes[4] = co2_ppm >> 8;
		_uplink_payload.bytes[5] = co2_ppm & 0xFF;
		_uplink_payload.bytes[6] = sound >> 8;
		_uplink_payload.bytes[7] = sound & 0xFF;
		_uplink_payload.bytes[8] = light >> 8;
		_uplink_payload.bytes[9] = light & 0xFF;

		printf("Payload: ");
		for (int i = 0; i < _uplink_payload.len; i++)
		{
			printf("%i ", _uplink_payload.bytes[i]);
		}

		printf("\n");

		status_leds_shortPuls(led_ST4);  // OPTIONAL
		lora_driver_returnCode_t rc;
		if ((rc = lora_driver_sendUploadMessage(false, &_uplink_payload)) == LORA_MAC_TX_OK ) {
			// Do nothing
		}	else if (rc == LORA_MAC_RX)
		{
		// The uplink message is sent and a downlink message is received
			flags = 0;
			max_temp_limit = 0;
			min_temp_limit = 0;
			max_hum_limit = 0;
			min_hum_limit = 0;
			max_co2_limit = 0;
			_downlink_payload.portNo = 1;
			xMessageBufferReceive(downlink_message_buffer_handle, &_downlink_payload, sizeof(lora_driver_payload_t), pdMS_TO_TICKS(30000)); // wait maximum 0.5m
			printf("DOWN LINK: from port: %i with %i bytes received!\n", _downlink_payload.portNo, _downlink_payload.len); // Just for Debug
			printf("Payload: ");
			for (int i = 0; i < _downlink_payload.len; i++)
			{
				printf("%u ", _downlink_payload.bytes[i]);
			}

			printf("\n");

			if (7 == _downlink_payload.len) // Check that we have got the expected 7 bytes
			{
      			// decode the payload into our variales
				flags = _downlink_payload.bytes[0];
				// TODO: set open, etc. on actuators.
				max_temp_limit = ((int16_t) _downlink_payload.bytes[1]) * 10;
				min_temp_limit = ((int16_t) _downlink_payload.bytes[2]) * 10;
				temperature_set_limits(temperature_handler, max_temp_limit, min_temp_limit);
				max_hum_limit = _downlink_payload.bytes[3];
				min_hum_limit = _downlink_payload.bytes[4];
				humidity_set_limits(humidity_handler, max_hum_limit, min_hum_limit);
      			max_co2_limit = (_downlink_payload.bytes[5] << 8) + _downlink_payload.bytes[6];
				// TODO: set co2 limits
				break;
			} else {
				printf("\n Downlink payload was not received\n");
			} 
		}

		printf("Upload Message >%s<\n", lora_driver_mapReturnCodeToText(rc));
		uxHighWaterMark = uxTaskGetStackHighWaterMark( loraUplinkTask );
		printf("Lora uplink uxHighWaterMark2: %i\n", uxHighWaterMark);
	}
}

void lorawan_handler_destroy(lorawan_handler_t self) {
	if (self != NULL) {
		free(self);
	}
	
	if (loraUplinkTask != NULL) {
		vTaskDelete(loraUplinkTask);
		loraUplinkTask = NULL;
	}
	
	if (loraDownlinkTask != NULL) {
		vTaskDelete(loraDownlinkTask);
		loraDownlinkTask = NULL;
	}
}
