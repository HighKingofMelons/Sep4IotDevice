/*
 * taskConfig.h

 * Created: 27/04/2023 13.29.57
 *  Author: andre
 */

/** @file
 * @brief This is a taskConfig header file.
 *
 * It handles the tasks.
 */
#pragma once
#include <FreeRTOSConfig.h>
/* Priorities at which the tasks are created. */
#define TASK_MESSURE_TEMP_PRIORITY	( tskIDLE_PRIORITY + 2 )
#define	TASK_MESSURE_HUM_PRIORITY	( tskIDLE_PRIORITY + 2 )
#define TASK_MESSURE_CO2_PRIORITY	( tskIDLE_PRIORITY + 2 )
#define TASK_ERROR_PRIORITY         ( tskIDLE_PRIORITY + 1 )
#define	TASK_ACTUATOR_PRIORITY	    ( tskIDLE_PRIORITY + 1 )
/* Task stack sizes*/
#define TASK_MESSURE_TEMP_STACK		( configMINIMAL_STACK_SIZE )
#define	TASK_MESSURE_HUM_STACK		( configMINIMAL_STACK_SIZE )
#define TASK_MESSURE_CO2_STACK		( configMINIMAL_STACK_SIZE )
#define TASK_ERROR_STACK            ( configMINIMAL_STACK_SIZE - 80 )
#define	TASK_ACTUATOR_STACK		    ( configMINIMAL_STACK_SIZE - 100 )
#define configUSE_MUTEXES			1
/* Initial Limits */
#define TEMP_MAX_LIMIT 25
#define TEMP_MIN_LIMIT 18
#define HUM_MAX_LIMIT 60
#define HUM_MIN_LIMIT 30
#define CO2_MAX_LIMIT 900
#define CO2_MIN_LIMIT 100

#define DEBUG                       1
#define DEBUG_ERROR                 0
