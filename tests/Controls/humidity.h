#pragma once
#include <humidity.h>

void humidity_makeOneMesurment(humidity_t self);

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