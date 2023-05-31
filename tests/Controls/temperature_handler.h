#pragma once
#include <temperature_handler.h>

void temperature_make_one_measurement(temperature_t self);

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