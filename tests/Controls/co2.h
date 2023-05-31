#pragma once

#include <../include/co2.h>

typedef struct co2 {
	int16_t co2Array[10];
	int16_t nextCo2ToReadIdx;
	int16_t latestAvgCo2;
	SemaphoreHandle_t latestAvgCo2Mutex;
	SemaphoreHandle_t maxLimitMutex;
	SemaphoreHandle_t minLimitMutex;
	TickType_t xMesureCircleFrequency;
	TickType_t xLastMessureCircleTime;
	int16_t maxCo2Limit;
	int16_t minCo2Limit;
} co2_st;