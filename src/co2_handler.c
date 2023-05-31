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
#include "../include/error_handler.h"
#include "../include/co2_handler.h"
#include <mh_z19.h>

int wakeUpCo2Sensos();

co2_handler_t initializeCo2(error_handler_t error_handler, TickType_t last_messure_circle_time);
void initializeCo2Driver(co2_handler_t self);
void co2_mesure(void *pvParameters);
void makeOneCo2Mesurment(co2_handler_t self);
void calculateCo2(co2_handler_t self);
void addCo2(co2_handler_t self, uint16_t co2);
void resetCo2Array(co2_handler_t self);
uint16_t getMaxCo2Limit(co2_handler_t self);
uint16_t getMinCo2Limit(co2_handler_t self);
void setMaxCo2Limit(co2_handler_t self, uint16_t maxCo2Limit);
void setMinCo2Limit(co2_handler_t self, uint16_t minCO2Limit);
void co2_makeOneMesuremnt(co2_handler_t self, uint16_t ppm); // TODO: delete

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
	TaskHandle_t mesure_co2_task_h;
} co2_st;
	
co2_handler_t co2_create(error_handler_t error_handler, TickType_t last_messure_circle_time){
	co2_handler_t _newCo2 = initializeCo2(error_handler, last_messure_circle_time);
	initializeCo2Driver(_newCo2);
	
	xTaskCreate(co2_mesure,	
				"messureCo2",
				TASK_MESSURE_CO2_STACK,
				(void*) _newCo2,
				TASK_MESSURE_CO2_PRIORITY,
				&(_newCo2->mesure_co2_task_h));
	return _newCo2;
}

void co2_mesure(void* pvParameters) {
	co2_handler_t self = (co2_handler_t) pvParameters;
	
	self->xLastMessureCircleTime = xTaskGetTickCount();
	for(;;) {
		makeOneCo2Mesurment(self);
		vTaskDelay(pdMS_TO_TICKS(27000UL));
	}
}
void addCo2(co2_handler_t self, uint16_t co2) {
	self->co2Array[self->nextCo2ToReadIdx++] = co2;
}

void co2_addMessurementToArray(co2_handler_t self, uint16_t co2){
	if(DEBUG){
		printf("Co2 Measurement #%i: %i \n", self->nextCo2ToReadIdx + 1, co2);
	}
	self->co2Array[self->nextCo2ToReadIdx++] = co2;
}

void resetCo2Array(co2_handler_t self) {
	memset(self->co2Array, 0, sizeof self->co2Array);
	self->nextCo2ToReadIdx = 0;
}

void calculateCo2(co2_handler_t self) {
	uint16_t co2x10Sum = 0;
	for (int i = 0; i < 10; i++) {
		co2x10Sum += self->co2Array[i];
	}
	while (1) {
		if (xSemaphoreTake(self->latestAvgCo2Mutex, pdMS_TO_TICKS(200)) == pdTRUE ) {
			self->latestAvgCo2 = (uint16_t) (co2x10Sum / 10);
			xSemaphoreGive(self->latestAvgCo2Mutex);
			break;
			} else {

		}
	}
}

uint16_t co2_get_latest_average_co2(co2_handler_t self) {
	uint16_t tmpCo2 = 0;
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

co2_handler_t initializeCo2(error_handler_t error_handler, TickType_t last_messure_circle_time){
	co2_handler_t _newCo2 = calloc(sizeof(co2_st), 1);
	resetCo2Array(_newCo2);
	_newCo2->latestAvgCo2 = 0;
	_newCo2->latestAvgCo2Mutex = xSemaphoreCreateMutex();
	_newCo2->xLastMessureCircleTime = last_messure_circle_time;
	_newCo2->error_handler = error_handler;
	return _newCo2;	
}

void initializeCo2Driver(co2_handler_t self) {
	mh_z19_initialise((ser_USART3));
	mh_z19_returnCode_t returnCode;
	returnCode = mh_z19_takeMeassuring();
	switch (returnCode) {
		case MHZ19_NO_MEASSURING_AVAILABLE:
		case MHZ19_NO_SERIAL:
		case MHZ19_PPM_MUST_BE_GT_999:
			error_handler_report(self->error_handler, ERROR_CO2);
			break;
		default:
			break;
			// Do nothing;
	}
}

// TODO: remove this method and change tests.
void co2_makeOneMesuremnt(co2_handler_t self, uint16_t ppm)
{
	switch (mh_z19_takeMeassuring())
	{
	case MHZ19_NO_MEASSURING_AVAILABLE:
		break;
	case MHZ19_NO_SERIAL:
		break;
	case MHZ19_PPM_MUST_BE_GT_999:
		break;
	default:
		if(DEBUG){
			co2_addMessurementToArray(self, ppm);
		}
		break;
	}

	if(self->nextCo2ToReadIdx >= 10){
		calculateCo2(self);
		resetCo2Array(self);
		xTaskDelayUntil(&(self->xLastMessureCircleTime), pdMS_TO_TICKS(MESURE_CIRCLE_FREAQUENCY));
	}
}

void makeOneCo2Mesurment(co2_handler_t self)
{
	if (self->nextCo2ToReadIdx >= 10) {
		calculateCo2(self);
		resetCo2Array(self);
		xTaskDelayUntil(&(self->xLastMessureCircleTime), pdMS_TO_TICKS(MESURE_CIRCLE_FREAQUENCY));
	}	
	
	uint16_t ppm;
	mh_z19_returnCode_t rc;
	rc = mh_z19_takeMeassuring();
	vTaskDelay(pdMS_TO_TICKS(500UL));
	switch (rc)
	{
		case MHZ19_NO_MEASSURING_AVAILABLE:
		case MHZ19_NO_SERIAL:
		case MHZ19_PPM_MUST_BE_GT_999:
			error_handler_report(self->error_handler, ERROR_CO2);
			break;
		default:
			mh_z19_getCo2Ppm(&ppm);
			printf("Co2 Measurement #%i: %i\n", self->nextCo2ToReadIdx + 1, ppm);
			addCo2(self, ppm);
	}
}

void co2_set_limits(co2_handler_t self, uint16_t maxLimit, uint16_t minLimit){
	setMaxCo2Limit(self, maxLimit);
	setMinCo2Limit(self, minLimit);
}
uint16_t getMaxCo2Limit(co2_handler_t self){
	uint16_t limit = 0;
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

uint16_t getMinCo2Limit(co2_handler_t self){
	uint16_t limit = 0;
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
void setMaxCo2Limit(co2_handler_t self, uint16_t maxCo2Limit){
	while(1){
		if (xSemaphoreTake(self->maxLimitMutex, pdMS_TO_TICKS(200)) == pdTRUE)
		{ 
			self->maxCo2Limit = maxCo2Limit;
			xSemaphoreGive(self->maxLimitMutex);
        	printf("CO2 MAX LIM: %i \n", maxCo2Limit);
			break;
		}
		else
		{

		}
	}
}
void setMinCo2Limit(co2_handler_t self, uint16_t minCo2Limit){
	while(1){
		if(xSemaphoreTake(self->maxLimitMutex, pdMS_TO_TICKS(200)) == pdTRUE){
			self->minCo2Limit = minCo2Limit;
			xSemaphoreGive(self->minLimitMutex);
        	printf("CO2 MAX LIM: %i \n", minCo2Limit);
			break;
		} else {
		
		}
	}
}

int8_t co2_acceptability_status(co2_handler_t self)
{
	int8_t returnValue = 0;
	uint16_t tempLatestAvgCo2 = co2_get_latest_average_co2(self);
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

void co2_handler_destroy(co2_handler_t self) {
	if(self->mesure_co2_task_h != NULL) {
		vTaskDelete(self->mesure_co2_task_h);
		self->mesure_co2_task_h = NULL;
	}

	vSemaphoreDelete(self->latestAvgCo2Mutex);
	vSemaphoreDelete(self->maxLimitMutex);
	vSemaphoreDelete(self->minLimitMutex);

	free(self);
	self = NULL;
}
