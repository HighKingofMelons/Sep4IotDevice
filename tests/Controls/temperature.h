#pragma once
#include <temperature.h>

void temperature_makeOneMesurment(temperature_t self);

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