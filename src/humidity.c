/*
 * humidity.c
 *
 * Created: 17/05/2023 18.02.18
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
#include "../include/humidity.h"
#include <hih8120.h>

void initializeHumidityDriver();
void wakeUpHumiditySensor();
humidity_t initializeHumidity(TickType_t freequency);
void humidity_mesure(void *pvParameters);
void humidity_resetArray(humidity_t self);
void humidity_makeOneMesurment(humidity_t self);
void humidity_addMeassurmentToArray(humidity_t self, uint16_t humidity);
void humidity_calculateAvg(humidity_t self);
uint8_t humidity_getMaxLimit(humidity_t self);
uint8_t humidity_getMinLimit(humidity_t self);
void humidity_setMaxLimit(humidity_t self, uint8_t maxLimit);
void humidity_setMinLimit(humidity_t self, uint8_t minLimit);
void humidity_recordMeasurment(humidity_t self);

static TaskHandle_t mesureHumidityTask = NULL;

typedef struct humidity {
	uint16_t humidityArray[10];
	uint8_t nextToReadIdx;
	uint8_t latestAvgHumidity;
	SemaphoreHandle_t latestAvgHumidityMutex;
	SemaphoreHandle_t maxLimitMutex;
	SemaphoreHandle_t minLimitMutex;
	TickType_t mesureCircleFrequency;
	TickType_t lastMessureCircleTime;
	uint8_t maxLimit;
	uint8_t minLimit;
} humidity_st;


humidity_t humidity_create(TickType_t freequency) {
	humidity_t _newHumidity = initializeHumidity(freequency);
	
	initializeHumidityDriver();
	
	/* Create the task, not storing the handle. */
	xTaskCreate(humidity_mesure,				/* Function that implements the task. */
				"messureHumidity",           /* Text name for the task. */
				TASK_MESSURE_HUM_STACK,		/* Stack size in words, not bytes. */
				(void*) _newHumidity,		/* Parameter passed into the task. */
				TASK_MESSURE_HUM_PRIORITY,		/* Priority at which the task is created. */
				&mesureHumidityTask			/* Used to pass out the created task's handle. */
	);

	return _newHumidity;			
}
	
void humidity_mesure(void* pvParameters) {
	humidity_t self = (humidity_t) pvParameters; // TODO: IS THIS CORRECT?
	for(;;) {
		humidity_makeOneMesurment(self);
        //printf("Hum hw: %i\n", uxTaskGetStackHighWaterMark(mesureHumidityTask));
		vTaskDelay(pdMS_TO_TICKS(27000UL)); // 27s delay, to make ~10 measurements in 4.5-5min
	}
}

void humidity_addMeassurmentToArray(humidity_t self, uint16_t humidity) {
	self->humidityArray[self->nextToReadIdx++] = humidity;
}

void humidity_resetArray(humidity_t self) {
	memset(self->humidityArray, 0, sizeof self->humidityArray);
	self->nextToReadIdx = 0;
}

void humidity_calculateAvg(humidity_t self) {
	uint16_t humidityx10Sum = 0;
	for (int i = 0; i < 10; i++) {
		humidityx10Sum += self->humidityArray[i];
	}

	while (1) {
		if (xSemaphoreTake(self->latestAvgHumidityMutex, pdMS_TO_TICKS(200)) == pdTRUE ) { // wait maximum 200ms
			self->latestAvgHumidity = (uint8_t) round(humidityx10Sum / 100); // TODO: something smarter
			xSemaphoreGive(self->latestAvgHumidityMutex);
			break;
		} else {
			/* We timed out and could not obtain the mutex and cant therefore not access the shared resource safely. */
		}
	}
}

uint8_t humidity_get_latest_average_humidity(humidity_t self) {
	uint8_t tmpHumidity = 0;
	while (1) {
		if (xSemaphoreTake(self->latestAvgHumidityMutex, pdMS_TO_TICKS(200)) == pdTRUE ) { // wait maximum 200ms
			tmpHumidity = self->latestAvgHumidity;
			xSemaphoreGive(self->latestAvgHumidityMutex);
			break;
		} else {
			/* We timed out and could not obtain the mutex and cant therefore not access the shared resource safely. */
		}
	}

	return tmpHumidity;	 // TODO: something smarter
}

void humidity_set_limits(humidity_t self, uint8_t maxLimit, uint8_t minLimit) {
	humidity_setMaxLimit(self, maxLimit);
	humidity_setMinLimit(self, minLimit);
}

int8_t humidity_acceptability_status(humidity_t self) {
	int8_t returnValue = 0;
	int16_t tempLatestAvgHumidity = humidity_get_latest_average_humidity(self);

	if (tempLatestAvgHumidity == 0) {
		returnValue = 0;
	} else if (tempLatestAvgHumidity > humidity_getMaxLimit(self)) {
		returnValue = 1;
	} else if (tempLatestAvgHumidity < humidity_getMinLimit(self)) {
		returnValue = -1;
	}

	return returnValue;
}
	
void humidity_destroy(humidity_t self) {
	if (self != NULL) {
		free(self);
	}
	
	if (mesureHumidityTask != NULL) {
		vTaskDelete(mesureHumidityTask);
		mesureHumidityTask = NULL;
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

humidity_t initializeHumidity(TickType_t freequency) {
	humidity_t _newHumidity = calloc(sizeof(humidity_st), 1);
	humidity_resetArray(_newHumidity);
	_newHumidity->latestAvgHumidity = 0;
	_newHumidity->latestAvgHumidityMutex = xSemaphoreCreateMutex();
	_newHumidity->minLimitMutex = xSemaphoreCreateMutex();
	_newHumidity->maxLimitMutex = xSemaphoreCreateMutex();
	_newHumidity->mesureCircleFrequency = freequency;
	_newHumidity->lastMessureCircleTime = xTaskGetTickCount();
	_newHumidity->maxLimit = HUM_MAX_LIMIT;
	_newHumidity->minLimit = HUM_MIN_LIMIT;
	return _newHumidity;
}

void initializeHumidityDriver() {
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

void wakeUpHumiditySensor() {
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

void humidity_makeOneMesurment(humidity_t self) {
	wakeUpHumiditySensor();

	switch(hih8120_measure()) {
		case HIH8120_OK:
			humidity_recordMeasurment(self);
			break;
		case HIH8120_TWI_BUSY:
			// TODO:
			break;
		case HIH8120_DRIVER_NOT_INITIALISED:
			// TODO:
			break;
		default:
			if (DEBUG) {
				humidity_recordMeasurment(self);
			}
			// TODO:
			break;
	}
	
	if (self->nextToReadIdx >= 10) {
		humidity_calculateAvg(self);
		humidity_resetArray(self);
		xTaskDelayUntil(&(self->lastMessureCircleTime), self->mesureCircleFrequency);
	}
}

uint8_t humidity_getMaxLimit(humidity_t self) {
	uint8_t limit = -100;
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

uint8_t humidity_getMinLimit(humidity_t self) {
	uint8_t limit = 0;
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

void humidity_setMaxLimit(humidity_t self, uint8_t maxLimit) {
	while (1) {
		if (xSemaphoreTake(self->maxLimitMutex, pdMS_TO_TICKS(200)) == pdTRUE ) { // wait maximum 200ms
			self->maxLimit = maxLimit;
			xSemaphoreGive(self->maxLimitMutex);
        	printf("Hum MAX LIM: %i \n", maxLimit);
			break;
		} else {
			/* We timed out and could not obtain the mutex and cant therefore not access the shared resource safely. */
		}
	}
}

void humidity_setMinLimit(humidity_t self, uint8_t minLimit) {
	while (1) {
		if (xSemaphoreTake(self->minLimitMutex, pdMS_TO_TICKS(200)) == pdTRUE ) { // wait maximum 200ms
			self->minLimit = minLimit;
			xSemaphoreGive(self->minLimitMutex);
        	printf("Hum MIN LIM: %i \n", minLimit);
			break;
		} else {
			/* We timed out and could not obtain the mutex and cant therefore not access the shared resource safely. */
		}
	}
}

void humidity_recordMeasurment(humidity_t self) {
	while(!hih8120_isReady()) {
		vTaskDelay(pdMS_TO_TICKS(500)); //wait 0.5s
	}

	uint16_t currentHumidity =  hih8120_getHumidityPercent_x10();

	if (DEBUG) {
		printf("Hum Measurement #%i: %i\n", self->nextToReadIdx + 1, currentHumidity);
	}

	humidity_addMeassurmentToArray(self, currentHumidity);
}

