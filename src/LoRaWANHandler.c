/*
* loraWANHandler.c
*
* Created: 12/04/2019 10:09:05
*  Author: IHA
*/
#include <stddef.h>
#include <stdio.h>

// co2 driver import
#include <mh_z19.h>

#include <ATMEGA_FreeRTOS.h>

#include <lora_driver.h>
#include <status_leds.h>

#include "co2/co2.h"

// Parameters for OTAA join - You have got these in a mail from IHA
#define LORA_appEUI "49B360EEE16A8D4C"
#define LORA_appKEY "E0597BF885F1F18CF896B91F8E211814"

void lora_handler_task( void *pvParameters );

static lora_driver_payload_t _uplink_payload;

void lora_handler_initialise(UBaseType_t lora_handler_task_priority, co2_c co2)
{
	xTaskCreate(
	lora_handler_task
	,  "LRHand"  // A name just for humans
	,  configMINIMAL_STACK_SIZE+200  // This stack size can be checked & adjusted by reading the Stack Highwater
	,  co2
	,  lora_handler_task_priority  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
	,  NULL );
}

static void _lora_setup(void)
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
	
<<<<<<< HEAD:src/LoRaWANHandler.c
	//mh_z19_initialise(ser_USART3); 
=======
	mh_z19_initialise(ser_USART3); 
>>>>>>> a0b62f3 (CO2 sensor progress):LoRaWANHandler.c
	
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
void lora_handler_task( void *pvParameters )
{
	co2_c co2 = (co2_c) pvParameters;
	// Hardware reset of LoRaWAN transceiver
	lora_driver_resetRn2483(1);
	vTaskDelay(2);
	lora_driver_resetRn2483(0);
	// Give it a chance to wakeup
	vTaskDelay(150);

	lora_driver_flushBuffers(); // get rid of first version string from module after reset!

	_lora_setup();

	_uplink_payload.len = 6;
	_uplink_payload.portNo = 2;

	TickType_t xLastWakeTime;
	const TickType_t xFrequency = pdMS_TO_TICKS(300000UL); // Upload message every 5 minutes (300000 ms)
	xLastWakeTime = xTaskGetTickCount();
	
	for(;;)
	{
		xTaskDelayUntil( &xLastWakeTime, xFrequency );
<<<<<<< HEAD:src/LoRaWANHandler.c
		
		// Some dummy payload
		uint8_t tmp = 255;
		// Flag bits
		uint8_t open_bit = tmp | 0x1;
		uint8_t battery_bit = 0;
		uint8_t temp_bit = 0;
		uint8_t hum_bit= 0;
		uint8_t co2_bit = 0;
		uint8_t sound_bit = 0;
		uint8_t light_bit = 0;
		uint8_t pir_bit = tmp | 0x128;

		// co2 actual mesurement
		//uint16_t ppm;
		//mh_z19_returnCode_t rc;
		//rc = mh_z19_takeMeassuring();
		
		uint8_t flags = 0;//open_bit | battery_bit | temp_bit | hum_bit | co2_bit | sound_bit | light_bit | pir_bit; // dummy flags
		int16_t temp = 0; // Dummy temp
		uint8_t hum = 0; // Dummy humidity
		uint16_t co2_ppm = co2_get_latest_average_co2(co2);
		printf("Payload.co2: %i\n", co2);
		uint16_t sound = 0; // Dummy sound
		uint16_t light = 0; // Dummy lux
		// Some dummy payload
		//uint16_t hum = 12345; // Dummy humidity
		//int16_t temp = 675; // Dummy temp
		//uint16_t co2_ppm = 1050; // Dummy CO2

		_uplink_payload.bytes[0] = flags;
		_uplink_payload.bytes[1] = temp >> 8;
		_uplink_payload.bytes[2] = temp & 0xFF;
		_uplink_payload.bytes[3] = hum;
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
=======

		// co2 actual messurement
		//uint16_t ppm;
		//mh_z19_returnCode_t rc;
		//rc = mh_z19_takeMeassuring();
		

		// Some dummy payload
		uint16_t hum = 12345; // Dummy humidity
		int16_t temp = 675; // Dummy temp
		//uint16_t co2_ppm = 1050; // Dummy CO2

		_uplink_payload.bytes[0] = hum >> 8;
		_uplink_payload.bytes[1] = hum & 0xFF;
		_uplink_payload.bytes[2] = temp >> 8;
		_uplink_payload.bytes[3] = temp & 0xFF;
		//_uplink_payload.bytes[4] = ppm >> 8;
		//_uplink_payload.bytes[5] = ppm & 0xFF;
>>>>>>> a0b62f3 (CO2 sensor progress):LoRaWANHandler.c

		status_leds_shortPuls(led_ST4);  // OPTIONAL
		printf("Upload Message >%s<\n", lora_driver_mapReturnCodeToText(lora_driver_sendUploadMessage(false, &_uplink_payload)));
	}
}
