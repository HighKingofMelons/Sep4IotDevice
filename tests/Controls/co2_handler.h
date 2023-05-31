#pragma once

#include <../include/co2_handler.h>

typedef struct co2 {
	uint16_t co2Array[10];
	uint8_t nextCo2ToReadIdx;
	uint16_t latestAvgCo2;
	SemaphoreHandle_t latestAvgCo2Mutex;
	SemaphoreHandle_t maxLimitMutex;
	SemaphoreHandle_t minLimitMutex;
	TickType_t xLastMessureCircleTime;
	uint16_t maxCo2Limit;
	uint16_t minCo2Limit;
	error_handler_t error_handler;
} co2_st;

int makeOneCo2Mesurment(co2_handler_t self);
void calculateCo2(co2_handler_t self);
void addCo2(co2_handler_t self, uint16_t co2);
void resetCo2Array(co2_handler_t self);
void co2_makeOneMesuremnt(co2_handler_t self, uint16_t ppm);