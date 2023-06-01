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
#include "../include/error_handler.h"
#include "../include/humidity_handler.h"
#include <hih8120.h>

humidity_t humidity_initialize(error_handler_t error_handler, TickType_t lastMessureCircleTime);
void humidity_init_driver(humidity_t self);
void humidity_wake_up_sensor(humidity_t self);
void humidity_mesure(void *pvParameters);
void humidity_resetArray(humidity_t self);
void humidity_makeOneMesurment(humidity_t self);
void humidity_addMeasurmentToArray(humidity_t self, uint16_t humidity);
void humidity_calculateAvg(humidity_t self);
uint8_t humidity_getMaxLimit(humidity_t self);
uint8_t humidity_getMinLimit(humidity_t self);
void humidity_setMaxLimit(humidity_t self, uint8_t maxLimit);
void humidity_setMinLimit(humidity_t self, uint8_t minLimit);
void humidity_recordMeasurment(humidity_t self); // TODO: remove

typedef struct humidity {
	uint16_t humidityArray[10];
	uint8_t nextToReadIdx;
	uint8_t latestAvgHumidity;
	SemaphoreHandle_t latestAvgHumidityMutex;
	SemaphoreHandle_t maxLimitMutex;
	SemaphoreHandle_t minLimitMutex;
	TickType_t lastMessureCircleTime;
	uint8_t maxLimit;
	uint8_t minLimit;
	error_handler_t error_handler;
	TaskHandle_t meassure_task_h;
} humidity_st;


humidity_t humidity_create(error_handler_t error_handler, TickType_t lastMessureCircleTime) {
	humidity_t _newHumidity = humidity_initialize(error_handler, lastMessureCircleTime);
	
	humidity_init_driver(_newHumidity);
	
	/* Create the task, not storing the handle. */
	xTaskCreate(humidity_mesure,				/* Function that implements the task. */
				"messureHumidity",           /* Text name for the task. */
				TASK_MESSURE_HUM_STACK,		/* Stack size in words, not bytes. */
				(void*) _newHumidity,		/* Parameter passed into the task. */
				TASK_MESSURE_HUM_PRIORITY,		/* Priority at which the task is created. */
				&(_newHumidity->meassure_task_h)			/* Used to pass out the created task's handle. */
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

void humidity_addMeasurmentToArray(humidity_t self, uint16_t humidity) {
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

int8_t humidity_get_acceptability_status(humidity_t self) {
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
	if (self->meassure_task_h != NULL) {
		vTaskDelete(self->meassure_task_h);
		self->meassure_task_h = NULL;
	}
	
	
	hih8120_driverReturnCode_t returnCode;
		
	if((returnCode = hih8120_destroy()) != HIH8120_OK) {
		error_handler_report(self->error_handler, ERROR_HUMI);
	}

	vSemaphoreDelete(self->latestAvgHumidityMutex);
	vSemaphoreDelete(self->maxLimitMutex);
	vSemaphoreDelete(self->minLimitMutex);

	if (self != NULL) {
		free(self);
	}
}

humidity_t humidity_initialize(error_handler_t error_handler, TickType_t lastMessureCircleTime) {
	humidity_t _newHumidity = calloc(sizeof(humidity_st), 1);
	humidity_resetArray(_newHumidity);
	_newHumidity->latestAvgHumidity = 0;
	_newHumidity->latestAvgHumidityMutex = xSemaphoreCreateMutex();
	_newHumidity->minLimitMutex = xSemaphoreCreateMutex();
	_newHumidity->maxLimitMutex = xSemaphoreCreateMutex();
	_newHumidity->lastMessureCircleTime = lastMessureCircleTime;
	_newHumidity->maxLimit = HUM_MAX_LIMIT;
	_newHumidity->minLimit = HUM_MIN_LIMIT;
	_newHumidity->error_handler = error_handler;
	return _newHumidity;
}

void humidity_init_driver(humidity_t self) {
	hih8120_driverReturnCode_t returnCode;
	switch (returnCode = hih8120_initialise()) {
		case HIH8120_OK:
			// Driver initialized OK
			// Always check what hih8120_initialise() returns
			break;
		case HIH8120_OUT_OF_HEAP:
		case HIH8120_TWI_BUSY:
		case HIH8120_DRIVER_NOT_INITIALISED:
			error_handler_report(self->error_handler, ERROR_HUMI);
			break;
		default:
			// TODO:
			break;
	}
}

void humidity_wake_up_sensor(humidity_t self) {
	switch (hih8120_wakeup()) {
		case HIH8120_OK:
			vTaskDelay(pdMS_TO_TICKS(100));
			// Driver initialized OK
			// Always check what hih8120_initialise() returns
			break;
		case HIH8120_TWI_BUSY:
		case HIH8120_DRIVER_NOT_INITIALISED:
			error_handler_report(self->error_handler, ERROR_HUMI);
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
	humidity_wake_up_sensor(self);

	switch(hih8120_measure()) {
		case HIH8120_OK:
			humidity_recordMeasurment(self);
			break;
		case HIH8120_TWI_BUSY:
		case HIH8120_DRIVER_NOT_INITIALISED:
			error_handler_report(self->error_handler, ERROR_HUMI);
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
		xTaskDelayUntil(&(self->lastMessureCircleTime), pdMS_TO_TICKS(MESURE_CIRCLE_FREAQUENCY));
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

	humidity_addMeasurmentToArray(self, currentHumidity);
}

