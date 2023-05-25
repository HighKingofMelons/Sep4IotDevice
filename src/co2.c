/*
 * co2.c
 *
 * Created: 5/11/2023 3:16:45 PM
 *  Author: takat
 */ 
#include <ATMEGA_FreeRTOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <semphr.h>
#include <message_buffer.h>
#include <task.h>
#include "../include/taskConfig.h"
#include "../include/co2.h"
#include <mh_z19.h>

int wakeUpCo2Sensos();

co2_t initializeCo2(TickType_t freequency);
void calculateCo2(co2_t self);
void resetCo2Array(co2_t self);
int16_t getMaxCo2Limit(co2_t self);
int16_t getMinCo2Limit(co2_t self);
void setMaxCo2Limit(co2_t self, int16_t maxCo2Limit);
void setMinCo2Limit(co2_t self, int16_t minCO2Limit);

static TaskHandle_t mesureCo2Task = NULL;

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
	
co2_t co2_create(TickType_t freequency){
	co2_t _newCo2 = initializeCo2(freequency);
	initializeCo2Driver();
	
	xTaskCreate(co2_mesure,	
				"messureCo2",
				TASK_MESSURE_CO2_STACK,
				(void*) _newCo2,
				TASK_MESSURE_CO2_PRIORITY,
				&mesureCo2Task);
	return _newCo2;
}

void co2_mesure(void* pvParameters) {
	co2_t self = (co2_t) pvParameters;
	
	self->xLastMessureCircleTime = xTaskGetTickCount();
	for(;;) {
		makeOneCo2Mesurment(self);
		vTaskDelay(pdMS_TO_TICKS(27000UL));
	}
}
void addCo2(co2_t self, int16_t co2) {
	self->co2Array[self->nextCo2ToReadIdx++] = co2;
}

void co2_addMessurementToArray(co2_t self, int16_t co2){
	self->co2Array[self->nextCo2ToReadIdx++] = co2;
}

void resetCo2Array(co2_t self) {
	memset(self->co2Array, 0, sizeof self->co2Array);
	self->nextCo2ToReadIdx = 0;
}

void calculateCo2(co2_t self) {
	int16_t co2x10Sum = 0;
	for (int i = 0; i < 10; i++) {
		co2x10Sum += self->co2Array[i];
	}
	while (1) {
		if (xSemaphoreTake(self->latestAvgCo2Mutex, pdMS_TO_TICKS(200)) == pdTRUE ) {
			self->latestAvgCo2 = (int16_t) (co2x10Sum / 10);
			xSemaphoreGive(self->latestAvgCo2Mutex);
			break;
			} else {

		}
	}
}

int16_t co2_get_latest_average_co2(co2_t self) {
	int16_t tmpCo2 = -100;
	while (1) {
		if (xSemaphoreTake(self->latestAvgCo2Mutex, pdMS_TO_TICKS(200)) == pdTRUE ) {
			tmpCo2 = self->latestAvgCo2;
			xSemaphoreGive(self->latestAvgCo2Mutex);
			break;
			} else {
		}
	}
	return tmpCo2;
}

co2_t initializeCo2(TickType_t freequency){
	co2_t _newCo2 = calloc(sizeof(co2_st), 1);
	resetCo2Array(_newCo2);
	_newCo2->latestAvgCo2 = 0;
	_newCo2->latestAvgCo2Mutex = xSemaphoreCreateMutex();
	_newCo2->xMesureCircleFrequency = freequency;
	_newCo2->xLastMessureCircleTime = xTaskGetTickCount();
	
	return _newCo2;	
}

int initializeCo2Driver() {
	mh_z19_initialise((ser_USART3));
	mh_z19_returnCode_t returnCode;
	returnCode = mh_z19_takeMeassuring();
	switch (returnCode){
		case MHZ19_NO_MEASSURING_AVAILABLE:
		return 2;
		case MHZ19_NO_SERIAL:
		return 3;
		case MHZ19_PPM_MUST_BE_GT_999:
		return 4;
		default:
		return 0;
	}
}
// void co2_makeOneMesuremnt(co2_t self){
	
// 	switch (mh_z19_takeMeassuring())
// 	{
// 	case MHZ19_NO_MEASSURING_AVAILABLE:
// 		return 3;
// 	case MHZ19_NO_SERIAL:
// 		return 4;
// 	case MHZ19_PPM_MUST_BE_GT_999:
// 		return 5;
// 	default:
// 		mh_z19_getCo2Ppm(&ppm);
// 		printf("Co2 Measurement #%i: %i\n", self->nextCo2ToReadIdx + 1, ppm);
// 		addCo2(self, ppm);
// 		return 0;
// 	}
// 	}
// }
void co2_recordMeasurement(co2_t self){
	uint16_t ppm;

	int16_t currentCo2 = mh_z19_getCo2Ppm(&ppm);
	if(DEBUG){
		printf("Co2 Measurement #%i: %i \n", self->nextCo2ToReadIdx + 1, currentCo2);
	}
	co2_addMessurementToArray(self, currentCo2);
}

int makeOneCo2Mesurment(co2_t self)
{
	if (self->nextCo2ToReadIdx >= 10) {
		calculateCo2(self);
		resetCo2Array(self);
		xTaskDelayUntil(&(self->xLastMessureCircleTime), self->xMesureCircleFrequency);
	}	
	
	uint16_t ppm;
	mh_z19_returnCode_t rc;
	rc = mh_z19_takeMeassuring();
	vTaskDelay(pdMS_TO_TICKS(500UL));
	switch (rc)
	{
	case MHZ19_NO_MEASSURING_AVAILABLE:
		return 3;
	case MHZ19_NO_SERIAL:
		return 4;
	case MHZ19_PPM_MUST_BE_GT_999:
		return 5;
	default:
		mh_z19_getCo2Ppm(&ppm);
		printf("Co2 Measurement #%i: %i\n", self->nextCo2ToReadIdx + 1, ppm);
		addCo2(self, ppm);
		return 0;
		}
}
void co2_set_limits(co2_t self, int16_t maxLimit, int16_t minLimit){
	setMaxCo2Limit(self, maxLimit);
	setMinCo2Limit(self, minLimit);
}
int16_t getMaxCo2Limit(co2_t self){
	int16_t limit = -100;
	while(1){
		if (xSemaphoreTake(self->maxLimitMutex, pdMS_TO_TICKS(200)) == pdTRUE){
			limit = self->maxCo2Limit;
			xSemaphoreGive(self->maxLimitMutex);
			break;
		} else {

		}
	}
	return limit;
}
int16_t getMinCo2Limit(co2_t self){
	int16_t limit = -100;
	while(1){
		if(xSemaphoreTake(self->minLimitMutex, pdMS_TO_TICKS(200)) == pdTRUE){
			limit = self->minCo2Limit;
			xSemaphoreGive(self->minLimitMutex);
			break;
		} else {

		}
	}
	return limit;
}
void setMaxCo2Limit(co2_t self, int16_t maxCo2Limit){
	while(1){
		if (xSemaphoreTake(self->maxLimitMutex, pdMS_TO_TICKS(200)) == pdTRUE)
		{ 
			self->maxCo2Limit = maxCo2Limit;
			xSemaphoreGive(self->maxLimitMutex);
			break;
		}
		else
		{

		}
	}
}
void setMinCo2Limit(co2_t self, int16_t minCo2Limit){
	while(1){
		if(xSemaphoreTake(self->maxLimitMutex, pdMS_TO_TICKS(200)) == pdTRUE){
			self->minCo2Limit = minCo2Limit;
			xSemaphoreGive(self->minLimitMutex);
			break;
		} else {
		
		}
	}
}

int8_t co2_acceptability_status(co2_t self)
{
	int8_t returnValue = 0;
	int16_t tempLatestAvgCo2 = co2_get_latest_average_co2(self);
	printf("Avg co2: %d", tempLatestAvgCo2);

	if (tempLatestAvgCo2 == 0)
	{
		returnValue = 0;
	}
	else if (tempLatestAvgCo2 > getMaxCo2Limit(self))
	{
		returnValue = 1;
	}
	else if (tempLatestAvgCo2 < getMinCo2Limit(self))
	{
		returnValue = -1;
	}

	return returnValue;
}
