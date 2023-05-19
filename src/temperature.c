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
#include <semphr.h>
#include <task.h>
#include "../include/taskConfig.h"
#include "../include/temperature.h"
#include <hih8120.h>

void initializeTemperatureDriver();
void wakeUpTemperatureSensor();
temperature_t initializeTemperature(TickType_t freequency);
void temperature_mesure(void *pvParameters);
void temperature_resetArray(temperature_t self);
void temperature_makeOneMesurment(temperature_t self);
void temperature_addMeassurmentToArray(temperature_t self, int16_t temperature);
void temperature_calculateAvg(temperature_t self);
int16_t temperature_getMaxLimit(temperature_t self);
int16_t temperature_getMinLimit(temperature_t self);
void temperature_setMaxLimit(temperature_t self, int16_t maxLimit);
void temperature_setMinLimit(temperature_t self, int16_t minLimit);
void temperature_recordMeasurment(temperature_t self);

static TaskHandle_t mesureTemperatureTask = NULL;

typedef struct temperature {
	int16_t temperatureArray[10];
	uint8_t nextToReadIdx;
	int16_t latestAvgTemperature;
	SemaphoreHandle_t latestAvgTemperatureMutex;
	SemaphoreHandle_t maxLimitMutex;
	SemaphoreHandle_t minLimitMutex;
	TickType_t mesureCircleFrequency;
	TickType_t lastMessureCircleTime;
	int16_t maxLimit;
	int16_t minLimit;
} temperature_st;


temperature_t temperature_create(TickType_t freequency) {
	temperature_t _newTemperature = initializeTemperature(freequency);
	
	initializeTemperatureDriver();
	
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
	for(;;) {
		temperature_makeOneMesurment(self);
		vTaskDelay(pdMS_TO_TICKS(27000UL)); // 27s delay, to make ~10 measurements in 4.5-5min
	}
}

void temperature_addMeassurmentToArray(temperature_t self, int16_t temperature) {
	self->temperatureArray[self->nextToReadIdx++] = temperature;
}

void temperature_resetArray(temperature_t self) {
	memset(self->temperatureArray, 0, sizeof self->temperatureArray);
	self->nextToReadIdx = 0;
}

void temperature_calculateAvg(temperature_t self) {
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
			/* We timed out and could not obtain the mutex and cant therefore not access the shared resource safely. */
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

// TODO: use semaphores
void temperature_set_limits(temperature_t self, int16_t maxLimit, int16_t minLimit) {
	temperature_setMaxLimit(self, maxLimit);
	temperature_setMinLimit(self, minLimit);
}

int8_t temperature_acceptability_status(temperature_t self) {
	int8_t returnValue = 0;
	int16_t tempLatestAvgTemperature = temperature_get_latest_average_temperature(self);

	if (tempLatestAvgTemperature == 0) {
		returnValue = 0;
	} else if (tempLatestAvgTemperature > temperature_getMaxLimit(self)) {
		returnValue = 1;
	} else if (tempLatestAvgTemperature < temperature_getMinLimit(self)) {
		returnValue = -1;
	}

	return returnValue;
}
	
void temperature_destroy(temperature_t self) {
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
		// TODO:
	} else {
		//HIH8120_OUT_OF_HEAP
		// TODO:
	}
}

temperature_t initializeTemperature(TickType_t freequency) {
	temperature_t _newTemperature = calloc(sizeof(temperature_st), 1);
	temperature_resetArray(_newTemperature);
	_newTemperature->latestAvgTemperature = 0;
	_newTemperature->latestAvgTemperatureMutex = xSemaphoreCreateMutex();
	_newTemperature->minLimitMutex = xSemaphoreCreateMutex();
	_newTemperature->maxLimitMutex = xSemaphoreCreateMutex();
	_newTemperature->mesureCircleFrequency = freequency;
	_newTemperature->lastMessureCircleTime = xTaskGetTickCount();
	_newTemperature->maxLimit = TEMP_MAX_LIMIT;
	_newTemperature->minLimit = TEMP_MIN_LIMIT;
	return _newTemperature;
}

void initializeTemperatureDriver() {
	hih8120_driverReturnCode_t returnCode;
	switch (returnCode = hih8120_initialise()) {
		case HIH8120_OK:
			// Driver initialized OK
			// Always check what hih8120_initialise() returns
			break;
		case HIH8120_OUT_OF_HEAP:
			// TODO:
			break;
		case HIH8120_TWI_BUSY:
			// TODO:
			break;
		case HIH8120_DRIVER_NOT_INITIALISED:
			// TODO:
			break;
		default:
			// TODO:
			break;
	}
}

void wakeUpTemperatureSensor() {
	switch (hih8120_wakeup()) {
		case HIH8120_OK:
			vTaskDelay(pdMS_TO_TICKS(100));
			// Driver initialized OK
			// Always check what hih8120_initialise() returns
			break;
		case HIH8120_TWI_BUSY:
			// TODO:
			break;
		case HIH8120_DRIVER_NOT_INITIALISED:
			// TODO:
			break;
		default:
			if (DEBUG) {
				vTaskDelay(pdMS_TO_TICKS(100));
			}
			// TODO:
			break;
	}
}

void temperature_makeOneMesurment(temperature_t self) {
	wakeUpTemperatureSensor();
	
	switch(hih8120_measure()) {
		case HIH8120_OK:
			temperature_recordMeasurment(self);
			break;
		case HIH8120_TWI_BUSY:
			// TODO:
			break;
		case HIH8120_DRIVER_NOT_INITIALISED:
			// TODO:
			break;
		default:
			if (DEBUG) {
				temperature_recordMeasurment(self);
			}
			// TODO:
			break;
	}
	
	if (self->nextToReadIdx >= 10) {
		temperature_calculateAvg(self);
		temperature_resetArray(self);
		xTaskDelayUntil(&(self->lastMessureCircleTime), self->mesureCircleFrequency);
	}
}

int16_t temperature_getMaxLimit(temperature_t self) {
	int16_t limit = -100;
	while (1) {
		if (xSemaphoreTake(self->maxLimitMutex, pdMS_TO_TICKS(200)) == pdTRUE ) { // wait maximum 200ms
			limit = self->maxLimit;
			xSemaphoreGive(self->maxLimitMutex);
			break;
		} else {
			/* We timed out and could not obtain the mutex and cant therefore not access the shared resource safely. */
		}
	}

	return limit;
}

int16_t temperature_getMinLimit(temperature_t self) {
	int16_t limit = -100;
	while (1) {
		if (xSemaphoreTake(self->minLimitMutex, pdMS_TO_TICKS(200)) == pdTRUE ) { // wait maximum 200ms
			limit = self->minLimit;
			xSemaphoreGive(self->minLimitMutex);
			break;
		} else {
			/* We timed out and could not obtain the mutex and cant therefore not access the shared resource safely. */
		}
	}

	return limit;
}

void temperature_setMaxLimit(temperature_t self, int16_t maxLimit) {
	while (1) {
		if (xSemaphoreTake(self->maxLimitMutex, pdMS_TO_TICKS(200)) == pdTRUE ) { // wait maximum 200ms
			self->maxLimit = maxLimit;
			xSemaphoreGive(self->maxLimitMutex);
			break;
		} else {
			/* We timed out and could not obtain the mutex and cant therefore not access the shared resource safely. */
		}
	}
}

void temperature_setMinLimit(temperature_t self, int16_t minLimit) {
	while (1) {
		if (xSemaphoreTake(self->minLimitMutex, pdMS_TO_TICKS(200)) == pdTRUE ) { // wait maximum 200ms
			self->minLimit = minLimit;
			xSemaphoreGive(self->minLimitMutex);
			break;
		} else {
			/* We timed out and could not obtain the mutex and cant therefore not access the shared resource safely. */
		}
	}
}

void temperature_recordMeasurment(temperature_t self) {
	while(!hih8120_isReady()) {
		vTaskDelay(pdMS_TO_TICKS(500)); //wait 0.5s
	}

	int16_t currentTemperature =  hih8120_getTemperature_x10();

	if (DEBUG) {
		printf("Temp Measurement #%i: %i\n", self->nextToReadIdx + 1, currentTemperature);
	}

	temperature_addMeassurmentToArray(self, currentTemperature);
}

