/*
 * taskConfig.h
 *
 * Created: 27/04/2023 13.29.57
 *  Author: andre
 */ 


#pragma once

/* Priorities at which the tasks are created. */
#define TASK_MESSURE_TEMP_PRIORITY	( tskIDLE_PRIORITY + 2 )
#define	TASK_MESSURE_HUM_PRIORITY	( tskIDLE_PRIORITY + 2 )
/* Task stack sizes*/
#define TASK_MESSURE_TEMP_STACK		( configMINIMAL_STACK_SIZE * 2 )
#define	TASK_MESSURE_HUM_STACK		( configMINIMAL_STACK_SIZE )
#define configUSE_MUTEXES			1