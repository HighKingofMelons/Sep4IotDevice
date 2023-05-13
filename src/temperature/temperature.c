/*
 * CFile1.c
 *
 * Created: 27/04/2023 11.42.18
 *  Author: andre
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ATMEGA_FreeRTOS.h>
#include "semphr.h"
#include "message_buffer.h"
#include "task.h"
#include "../taskConfig.h"
#include "temperature.h"
#include <hih8120.h>

int initializeTemperatureDriver();
int wakeUpTemperatureSensor();

temperature_t initializeTemperature(uint8_t port, TickType_t freequency);
void temperature_mesure(void *pvParameters);
void resetTemperatureArray(temperature_t self);
int makeOneTemperatueMesurment(temperature_t self);
void addTemperature(temperature_t self, int16_t temperature);
void calculateTemperature(temperature_t self);

static TaskHandle_t mesureTemperatureTask = NULL;

typedef struct temperature {
	int16_t temperatureArray[10];
	uint8_t nextTemperatureToReadIdx;
	int16_t latestAvgTemperature;
	SemaphoreHandle_t latestAvgTemperatureMutex;
	uint8_t portNo;
	TickType_t xMesureCircleFrequency;
	TickType_t xLastMessureCircleTime;
} temperature_st;


temperature_t temperature_create(uint8_t port, TickType_t freequency) {
	temperature_t _newTemperature = initializeTemperature(port, freequency);
	
	int dirverInitIdx = initializeTemperatureDriver(); // TODO: resolve error codes
	
	/* Create the task, not storing the handle. */
	xTaskCreate(temperature_mesure,				/* Function that implements the task. */
				"messureTemperature",           /* Text name for the task. */
				TASK_MESSURE_TEMP_STACK,		/* Stack size in words, not bytes. */
				(void*) _newTemperature,		/* Parameter passed into the task. */
				TASK_MESSURE_TEMP_PRIORITY,		/* Priority at which the task is created. */
				&mesureTemperatureTask			/* Used to pass out the created task's handle. */
	);	
	
	return _newTemperature;			
}
	
void temperature_mesure(void* pvParameters) {
	temperature_t self = (temperature_t) pvParameters; // TODO: IS THIS CORRECT?
	
	self->xLastMessureCircleTime = xTaskGetTickCount();
	for(;;) {
		makeOneTemperatueMesurment(self);
		vTaskDelay(pdMS_TO_TICKS(27000UL)); // 27s delay, to make ~10 measurements in 4.5-5min
	}
}

void addTemperature(temperature_t self, int16_t temperature) {
	self->temperatureArray[self->nextTemperatureToReadIdx++] = temperature;
}

void resetTemperatureArray(temperature_t self) {
	memset(self->temperatureArray, 0, sizeof self->temperatureArray);
	self->nextTemperatureToReadIdx = 0;
}

void calculateTemperature(temperature_t self) {
	int16_t temperaturex10Sum = 0;
	for (int i = 0; i < 10; i++) {
		temperaturex10Sum += self->temperatureArray[i];
	}

	while (1) {
		if (xSemaphoreTake(self->latestAvgTemperatureMutex, pdMS_TO_TICKS(200)) == pdTRUE ) { // wait maximum 200ms
			self->latestAvgTemperature = (int16_t) (temperaturex10Sum / 10); // TODO: something smarter
			xSemaphoreGive(self->latestAvgTemperatureMutex);
			break;
		} else {

		}
	}
}

int16_t temperature_get_latest_average_temperature(temperature_t self) {
	int16_t tmpTemperature = -100;
	while (1) {
		if (xSemaphoreTake(self->latestAvgTemperatureMutex, pdMS_TO_TICKS(200)) == pdTRUE ) { // wait maximum 200ms
			tmpTemperature = self->latestAvgTemperature;
			xSemaphoreGive(self->latestAvgTemperatureMutex);
			break;
		} else {
			/* We timed out and could not obtain the mutex and cant therefore not access the shared resource safely. */
		}
	}

	return tmpTemperature;	 // TODO: something smarter
}
	
int temaperature_destroy(temperature_t self) {
	if (self != NULL) {
		free(self);
	}
	
	if (mesureTemperatureTask != NULL) {
		vTaskDelete(mesureTemperatureTask);
		mesureTemperatureTask = NULL;
	}
	
	
	hih8120_driverReturnCode_t returnCode;
		
	if((returnCode = hih8120_destroy()) == HIH8120_OK) {
		//Destroyed successfully
		return 1;
	} else {
		//HIH8120_OUT_OF_HEAP
		return 2;
	}
}

temperature_t initializeTemperature(uint8_t port, TickType_t freequency) {
	temperature_t _newTemperature = calloc(sizeof(temperature_st), 1);
	resetTemperatureArray(_newTemperature);
	_newTemperature->latestAvgTemperature = 0;
	_newTemperature->latestAvgTemperatureMutex = xSemaphoreCreateMutex();
	_newTemperature->portNo = port;
	_newTemperature->xMesureCircleFrequency = freequency;
	
	return _newTemperature;
}

int initializeTemperatureDriver() {
	hih8120_driverReturnCode_t returnCode;
	switch (returnCode = hih8120_initialise()) {
		case HIH8120_OK:
			// Driver initialized OK
			// Always check what hih8120_initialise() returns
			return 1;
		case HIH8120_OUT_OF_HEAP:
			return 2;
		case HIH8120_TWI_BUSY:
			return 3;
		case HIH8120_DRIVER_NOT_INITIALISED:
			return 4;
		default:
			return 0;
	}
}

int wakeUpTemperatureSensor() {
	switch (hih8120_wakeup()) {
		case HIH8120_OK:
			vTaskDelay(pdMS_TO_TICKS(100));
			// Driver initialized OK
			// Always check what hih8120_initialise() returns
			return 1;
		case HIH8120_TWI_BUSY:
			return 3;
		case HIH8120_DRIVER_NOT_INITIALISED:
			return 4;
		default:
			return 0;
	}
}

int makeOneTemperatueMesurment(temperature_t self) {
	if (self->nextTemperatureToReadIdx >= 10) {
		calculateTemperature(self);
		resetTemperatureArray(self);
		xTaskDelayUntil(&(self->xLastMessureCircleTime), self->xMesureCircleFrequency);
	}
	
	wakeUpTemperatureSensor();
	
	switch(hih8120_measure()) {
		case HIH8120_OK:
			while(!hih8120_isReady()) {
				vTaskDelay(pdMS_TO_TICKS(500)); //wait 0.5s
			}
			printf("Temp Measurement #%i: %i\n", self->nextTemperatureToReadIdx + 1, hih8120_getTemperature_x10());
			addTemperature(self, hih8120_getTemperature_x10());
			return 1;
		case HIH8120_TWI_BUSY:
			return 3;
		case HIH8120_DRIVER_NOT_INITIALISED:
			return 4;
		default:
			return 0;
	}
}


