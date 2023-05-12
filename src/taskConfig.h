/*
 * taskConfig.h
 *
 * Created: 5/11/2023 3:26:10 PM
 *  Author: takat
 */ 


#pragma once

/* Priorities at which the tasks are created. */
#define TASK_MESSURE_CO2_PRIORITY	( tskIDLE_PRIORITY + 2 )
/* Task stack sizes*/
#define TASK_MESSURE_CO2_STACK		( configMINIMAL_STACK_SIZE * 2 )
//#define configUSE_MUTEXES			1