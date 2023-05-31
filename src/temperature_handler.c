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
#include "../include/error_handler.h"
#include "../include/temperature_handler.h"
#include <hih8120.h>

temperature_t temperature_initialize(error_handler_t error_handler, TickType_t last_meassure_circle_time);
void temperature_initialize_driver();
void temperature_wake_up_sensor();
void temperature_mesure(void *pvParameters);
void temperature_resetArray(temperature_t self);
void temperature_make_one_measurement(temperature_t self);
void temperature_add_measurment_to_array(temperature_t self, int16_t temperature);
void temperature_calculateAvg(temperature_t self);
int16_t temperature_get_max_limit(temperature_t self);
int16_t temperature_get_min_limit(temperature_t self);
void temperature_set_max_limit(temperature_t self, int16_t maxLimit);
void temperature_set_min_limit(temperature_t self, int16_t minLimit);
void temperature_record_measurment(temperature_t self);

typedef struct temperature {
	int16_t temperatureArray[10];
	uint8_t nextToReadIdx;
	int16_t latestAvgTemperature;
	SemaphoreHandle_t latestAvgTemperatureMutex;
	SemaphoreHandle_t maxLimitMutex;
	SemaphoreHandle_t minLimitMutex;
	TickType_t last_meassure_circle_time;
	int16_t maxLimit;
	int16_t minLimit;
	error_handler_t error_handler;
	TaskHandle_t temperature_measure_h;
} temperature_st;

temperature_t temperature_create(error_handler_t error_handler, TickType_t last_meassure_circle_time) {
	temperature_t _newTemperature = temperature_initialize(error_handler, last_meassure_circle_time);
	
	temperature_initialize_driver(_newTemperature);
	
	/* Create the task, not storing the handle. */
	xTaskCreate(temperature_mesure,				/* Function that implements the task. */
				"messureTemperature",           /* Text name for the task. */
				TASK_MESSURE_TEMP_STACK,		/* Stack size in words, not bytes. */
				_newTemperature,		/* Parameter passed into the task. */
				TASK_MESSURE_TEMP_PRIORITY,		/* Priority at which the task is created. */
				&(_newTemperature->temperature_measure_h)			/* Used to pass out the created task's handle. */
	);
	
	return _newTemperature;			
}
	
void temperature_mesure(void* pvParameters) {
	temperature_t self = (temperature_t) pvParameters; // TODO: IS THIS CORRECT?
	for(;;) {
		temperature_make_one_measurement(self);
        //printf("Tem hw: %i\n", uxTaskGetStackHighWaterMark(mesureTemperatureTask));
		vTaskDelay(pdMS_TO_TICKS(27000UL)); // 27s delay, to make ~10 measurements in 4.5-5min
	}
}

void temperature_add_measurment_to_array(temperature_t self, int16_t temperature) {
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
	temperature_set_max_limit(self, maxLimit);
	temperature_set_min_limit(self, minLimit);
}

int8_t temperature_get_acceptability_status(temperature_t self) {
	int8_t returnValue = 0;
	int16_t tempLatestAvgTemperature = temperature_get_latest_average_temperature(self);

	if (tempLatestAvgTemperature == 0) {
		returnValue = 0;
	} else if (tempLatestAvgTemperature > temperature_get_max_limit(self)) {
		returnValue = 1;
	} else if (tempLatestAvgTemperature < temperature_get_min_limit(self)) {
		returnValue = -1;
	}

	return returnValue;
}
	
void temperature_destroy(temperature_t self) {	
	if (self->temperature_measure_h != NULL) {
		vTaskDelete(self->temperature_measure_h);
		self->temperature_measure_h = NULL;
	}
		
	if (hih8120_destroy() != HIH8120_OK) {
		error_handler_report(self->error_handler, ERROR_TEMP);
	}

	vSemaphoreDelete(self->latestAvgTemperatureMutex);
	vSemaphoreDelete(self->maxLimitMutex);
	vSemaphoreDelete(self->minLimitMutex);

	if (self != NULL) {
		free(self);
		self = NULL;
	}
}

temperature_t temperature_initialize(error_handler_t error_handler, TickType_t last_meassure_circle_time) {
	temperature_t _newTemperature = calloc(sizeof(temperature_st), 1);
	temperature_resetArray(_newTemperature);
	_newTemperature->latestAvgTemperature = 0;
	_newTemperature->latestAvgTemperatureMutex = xSemaphoreCreateMutex();
	_newTemperature->minLimitMutex = xSemaphoreCreateMutex();
	_newTemperature->maxLimitMutex = xSemaphoreCreateMutex();
	_newTemperature->last_meassure_circle_time = last_meassure_circle_time;
	_newTemperature->maxLimit = TEMP_MAX_LIMIT;
	_newTemperature->minLimit = TEMP_MIN_LIMIT;
	_newTemperature->error_handler = error_handler;
	return _newTemperature;
}

void temperature_initialize_driver(temperature_t self) {
	hih8120_driverReturnCode_t returnCode;
	switch (returnCode = hih8120_initialise()) {
		case HIH8120_OUT_OF_HEAP:
		case HIH8120_TWI_BUSY:
		case HIH8120_DRIVER_NOT_INITIALISED:
			error_handler_report(self->error_handler, ERROR_TEMP);
			break;
		case HIH8120_OK:
		default:
			// everithing is well
			break;
	}
}

void temperature_wake_up_sensor(temperature_t self) {
	switch (hih8120_wakeup()) {
		case HIH8120_OK:
			vTaskDelay(pdMS_TO_TICKS(100));
			// Driver initialized OK
			// Always check what hih8120_initialise() returns
			break;
		case HIH8120_TWI_BUSY:
		case HIH8120_DRIVER_NOT_INITIALISED:
			error_handler_report(self->error_handler, ERROR_TEMP);
			break;
		default:
			if (DEBUG) {
				vTaskDelay(pdMS_TO_TICKS(100));
			}
			break;
	}
}

void temperature_make_one_measurement(temperature_t self) {
	temperature_wake_up_sensor(self);
	
	switch(hih8120_measure()) {
		case HIH8120_OK:
			temperature_record_measurment(self);
			break;
		case HIH8120_TWI_BUSY:
		case HIH8120_DRIVER_NOT_INITIALISED:
			error_handler_report(self->error_handler, ERROR_TEMP);
			break;
		default:
			if (DEBUG) {
				temperature_record_measurment(self);
			}

			break;
	}
	
	if (self->nextToReadIdx >= 10) {
		temperature_calculateAvg(self);
		temperature_resetArray(self);
		xTaskDelayUntil(&(self->last_meassure_circle_time), pdMS_TO_TICKS(MESURE_CIRCLE_FREAQUENCY));
	}
}

int16_t temperature_get_max_limit(temperature_t self) {
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

int16_t temperature_get_min_limit(temperature_t self) {
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

void temperature_set_max_limit(temperature_t self, int16_t maxLimit) {
	while (1) {
		if (xSemaphoreTake(self->maxLimitMutex, pdMS_TO_TICKS(200)) == pdTRUE ) { // wait maximum 200ms
			self->maxLimit = maxLimit;
			xSemaphoreGive(self->maxLimitMutex);
        	printf("Temp MAX LIM: %i \n", maxLimit);
			break;
		} else {
			/* We timed out and could not obtain the mutex and cant therefore not access the shared resource safely. */
		}
	}
}

void temperature_set_min_limit(temperature_t self, int16_t minLimit) {
	while (1) {
		if (xSemaphoreTake(self->minLimitMutex, pdMS_TO_TICKS(200)) == pdTRUE ) { // wait maximum 200ms
			self->minLimit = minLimit;
			xSemaphoreGive(self->minLimitMutex);
        	printf("Temp MIN LIM: %i \n", minLimit);
			break;
		} else {
			/* We timed out and could not obtain the mutex and cant therefore not access the shared resource safely. */
		}
	}
}

void temperature_record_measurment(temperature_t self) {
	while(!hih8120_isReady()) {
		vTaskDelay(pdMS_TO_TICKS(500)); //wait 0.5s
	}

	int16_t currentTemperature =  hih8120_getTemperature_x10();

	if (DEBUG) {
		printf("Temp Measurement #%i: %i\n", self->nextToReadIdx + 1, currentTemperature);
	}

	temperature_add_measurment_to_array(self, currentTemperature);
}

