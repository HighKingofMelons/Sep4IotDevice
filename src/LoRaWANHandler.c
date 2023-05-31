/*
* loraWANHandler.c
*
* Created: 12/04/2019 10:09:05
*  Author: IHA
*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <lora_driver.h>
#include <status_leds.h>
#include <message_buffer.h>
#include <semphr.h>

#include "LoRaWAN.h"
#include "taskConfig.h"
#include "LoRaWANHandler.h"

typedef struct lorawan_handler {
	MessageBufferHandle_t downlink_message_buffer_handle;
	lora_driver_payload_t uplink_payload;
	uint8_t downlink_payload_bytes[7];
	SemaphoreHandle_t uplink_payload_mut;
	SemaphoreHandle_t downlink_payload_mut;
	TickType_t last_messure_circle_time_uplink;
	TaskHandle_t lora_uplink_task_h;
} lorawan_handler_st;

lorawan_handler_t initialize_lorawan_handler(TickType_t last_messure_circle_time);
void _lora_setup(void);
void lora_uplink_task(void *pvParameters );
void lora_uplink_task_(uint8_t test, void *pvParameters );
lora_driver_returnCode_t lora_upload_uplink(lorawan_handler_t self);
void lora_download_downlink(lorawan_handler_t self);

lorawan_handler_t lorawan_handler_create(TickType_t last_messure_circle_time)
{
	lorawan_handler_t self = initialize_lorawan_handler(last_messure_circle_time);
	xSemaphoreTake(self->uplink_payload_mut, pdMS_TO_TICKS(10));
	xSemaphoreTake(self->downlink_payload_mut, pdMS_TO_TICKS(10));
	lora_driver_initialise(ser_USART1, self->downlink_message_buffer_handle);

	xTaskCreate(
		lora_uplink_task,
		"LoRaWAN_uplink",  // A name just for humans
		TASK_LORA_UPLINK_STACK,  // This stack size can be checked & adjusted by reading the Stack Highwater
		self, 
		TASK_LORA_UPLINK_PRIORITY,    // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
		&(self->lora_uplink_task_h));

	return self;
}

void lorawan_handler_set_uplink(lorawan_handler_t self, uint8_t bytes[10]) {
	memcpy((self->uplink_payload).bytes, bytes, sizeof(uint8_t) * 10);
	printf("lorawan_handler_set_uplink byte[0]: %u\n", (self->uplink_payload).bytes[0]);
	xSemaphoreGive(self->uplink_payload_mut);
}

uint8_t lorawan_handler_get_downlink(lorawan_handler_t self, uint8_t* bytes) {
	if (xSemaphoreTake(self->downlink_payload_mut, pdMS_TO_TICKS(60000)) == pdTRUE ) {
		memcpy(bytes, self->downlink_payload_bytes, sizeof(uint8_t) * 7);
		return 1;
	}

	return 0;
}

void lorawan_handler_destroy(lorawan_handler_t self) {
	if (self->lora_uplink_task_h != NULL) {
		vTaskDelete(self->lora_uplink_task_h);
		self->lora_uplink_task_h = NULL;
	}

	if (self != NULL) {
		free(self);
	}
}

lorawan_handler_t initialize_lorawan_handler(TickType_t last_messure_circle_time) {
	lorawan_handler_t _new_lorawan_handler = calloc(sizeof(lorawan_handler_st), 1);
	_new_lorawan_handler->downlink_message_buffer_handle = xMessageBufferCreate(sizeof(lora_driver_payload_t) * 2); // Here I make room for two downlink messages in the message buffer
	_new_lorawan_handler->uplink_payload.portNo = 1;
	_new_lorawan_handler->uplink_payload.len = 10;
	memset((_new_lorawan_handler->uplink_payload).bytes, 0, sizeof((_new_lorawan_handler->uplink_payload).bytes));
	memset(_new_lorawan_handler->downlink_payload_bytes, 0, sizeof(_new_lorawan_handler->downlink_payload_bytes));
	_new_lorawan_handler->uplink_payload_mut = xSemaphoreCreateMutex();
	_new_lorawan_handler->downlink_payload_mut = xSemaphoreCreateMutex();
	_new_lorawan_handler->last_messure_circle_time_uplink = last_messure_circle_time;
	_new_lorawan_handler->lora_uplink_task_h = NULL;
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
void lora_uplink_task(void *pvParameters ) {
	lora_uplink_task_(0, pvParameters);
}

void lora_uplink_task_(uint8_t test, void *pvParameters )
{
	lorawan_handler_t self = (lorawan_handler_t) pvParameters;

	// Hardware reset of LoRaWAN transceiver
	lora_driver_resetRn2483(1);
	vTaskDelay(2);
	lora_driver_resetRn2483(0);
	// Give it a chance to wakeup
	vTaskDelay(150);

	lora_driver_flushBuffers(); // get rid of first version string from module after reset!

	_lora_setup();

	const TickType_t xFrequency = pdMS_TO_TICKS(MESURE_CIRCLE_FREAQUENCY); // Upload message every 5 minutes (300000 ms)
	for(;;) {
		xTaskDelayUntil(&(self->last_messure_circle_time_uplink), xFrequency);
		lora_driver_returnCode_t rc = lora_upload_uplink(self);
		vTaskDelay(pdMS_TO_TICKS(5000));
		if (rc == LORA_MAC_TX_OK) {
			// The uplink message is sent - Do nothing
        	printf("Lor hw: %i\n", uxTaskGetStackHighWaterMark(self->lora_uplink_task_h));
		} else if (rc == LORA_MAC_RX) {
			// The uplink message is sent and a downlink message is received
        	lora_download_downlink(self);
		}

		vTaskDelay(pdMS_TO_TICKS(5000));
		printf("Upload Message >%s<\n", lora_driver_mapReturnCodeToText(rc));
		if (test) {
			break;
		}
	}
}

lora_driver_returnCode_t lora_upload_uplink(lorawan_handler_t self) {
	lora_driver_returnCode_t rc;
	while (1) {
		if (xSemaphoreTake(self->uplink_payload_mut, pdMS_TO_TICKS(60000UL)) == pdTRUE ) {
			printf("Payload: ");
			for (int i = 0; i < (self->uplink_payload).len; i++)
			{
				printf("%i ", (self->uplink_payload).bytes[i]);
			}

			printf("\n");

			status_leds_shortPuls(led_ST4);  // OPTIONAL
			rc = lora_driver_sendUploadMessage(false, &(self->uplink_payload));
			break;
		}
	}

	return rc;
}

void lora_download_downlink(lorawan_handler_t self) {
	// The uplink message is sent and a downlink message is received
    printf("Lor hw: %i\n", uxTaskGetStackHighWaterMark(self->lora_uplink_task_h));
	lora_driver_payload_t _downlink_payload;
	size_t downlink_size = xMessageBufferReceive(self->downlink_message_buffer_handle, &_downlink_payload, sizeof(lora_driver_payload_t), pdMS_TO_TICKS(180000)); // wait maximum 0.5m
    printf("Lor hw: %i\n", uxTaskGetStackHighWaterMark(self->lora_uplink_task_h));
	vTaskDelay(pdMS_TO_TICKS(5000));
	printf("downlink_size: %i\n", downlink_size);
	if(downlink_size > 0 && _downlink_payload.len == 7) {
		printf("\n Downlink payload received\n");
		printf("Payload: ");
		for (int i = 0; i < 7; i++)
		{
			printf("%u ", _downlink_payload.bytes[i]);
		}

		printf("\n");
    	printf("Lor hw: %i\n", uxTaskGetStackHighWaterMark(self->lora_uplink_task_h));
		vTaskDelay(pdMS_TO_TICKS(5000));
		memcpy(self->downlink_payload_bytes, _downlink_payload.bytes, sizeof(uint8_t) * 7);
    	printf("Lor hw: %i\n", uxTaskGetStackHighWaterMark(self->lora_uplink_task_h));
		xSemaphoreGive(self->downlink_payload_mut);
	}
}
