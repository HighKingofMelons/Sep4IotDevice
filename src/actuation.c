#include <stdlib.h>
#include <stdio.h>

#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <rc_servo.h>

#include "taskConfig.h"
#include "temperature_handler.h"
#include "humidity_handler.h"
#include "co2_handler.h"

#include "actuation.h"

TaskHandle_t actuation_task_h;

struct actuation_handler {
    temperature_t temp_handler;
    humidity_t humid_handler;

    SemaphoreHandle_t override_sema;
    SemaphoreHandle_t actuator_state_sema;
    BaseType_t ventilation_overriden;
    BaseType_t aircon_overriden;
    actuator_state_t actuator_state;
};

enum actuator {
    VENTILATION = 0,
    AIRCON      = 1
};

actuator_state_t actuators_get_state(actuation_handler_t self);

void update_vent(actuation_handler_t self) {
    if (pdTRUE != xSemaphoreTake(self->override_sema, pdMS_TO_TICKS(50)))
        return;

    if (self->ventilation_overriden) {
        xSemaphoreGive(self->override_sema);
        return;
    }

    switch (humidity_get_acceptability_status(self->humid_handler))
    {
    case  1:
        rc_servo_setPosition(VENTILATION, VENT_ON);
        printf("VENT_ON\n");
        break;
    case  0:
    case -1:
        rc_servo_setPosition(VENTILATION, VENT_OFF);
        printf("VENT_OFF\n");
        break;
    }

    xSemaphoreGive(self->override_sema);
}                                                                                                                               

void update_aircon(actuation_handler_t self) {
    if (pdTRUE != xSemaphoreTake(self->override_sema, pdMS_TO_TICKS(50)))
        return;
    
    if (self->aircon_overriden) {
        xSemaphoreGive(self->override_sema);
        return;
    }

    switch (temperature_get_acceptability_status(self->temp_handler))
    {
    case -1:
        rc_servo_setPosition(AIRCON, AIRCON_HEAT);
        printf("AIRCON_HEAT\n");
        break;
    case 1:
        rc_servo_setPosition(AIRCON, AIRCON_COOL);
        printf("AIRCON_COOL\n");
        break;
    case 0:
        rc_servo_setPosition(AIRCON, AIRCON_OFF);
        printf("AIRCON_OFF\n");
        break;
    }

    xSemaphoreGive(self->override_sema);
}

void actuation_task(void *pvParameters) {
    actuation_handler_t self = (actuation_handler_t) pvParameters;
    TickType_t lastDelay = xTaskGetTickCount();
    
    for(;;) {
        xTaskDelayUntil(&lastDelay, pdMS_TO_TICKS(300000));
        if (actuators_get_state(self) == ACTUATORS_OFF) {
            printf("ACTUATORS_OFF\n");
            continue;
        }

        update_vent(self);
        update_aircon(self);
        lastDelay = xTaskGetTickCount();
        printf("Ac hw: %i\n", uxTaskGetStackHighWaterMark(actuation_task_h));
    }
}

actuation_handler_t actuation_handler_init(temperature_t temperature, humidity_t humidity) {
    rc_servo_initialise();

    actuation_handler_t _handler = calloc(1, sizeof(struct actuation_handler));

    *_handler = (struct actuation_handler) {
        temperature,
        humidity,

        xSemaphoreCreateMutex(),
        xSemaphoreCreateMutex(),
        pdFALSE,
        pdFALSE,
        ACTUATORS_OFF
    };

    rc_servo_setPosition(VENTILATION, VENT_OFF);
    rc_servo_setPosition(AIRCON, AIRCON_OFF);

    xTaskCreate(
        actuation_task,
        "Actuator Task",
        TASK_ACTUATOR_STACK,
        _handler,
        TASK_ACTUATOR_PRIORITY,
        &actuation_task_h
    );

    return _handler;
}

void actuation_handler_destroy(actuation_handler_t victim) {
    vTaskDelete(actuation_task_h);
    vSemaphoreDelete(victim->override_sema);
    vSemaphoreDelete(victim->actuator_state_sema);
    free(victim);
    victim = NULL;
}

void actuators_ventilation_override_state(actuation_handler_t self, vent_state_t state) {
    for(;;) {
        if (pdTRUE == xSemaphoreTake(self->override_sema, pdMS_TO_TICKS(50)))
            break;
    }

    self->ventilation_overriden = pdTRUE;
    rc_servo_setPosition(VENTILATION, state);

    xSemaphoreGive(self->override_sema);
}

void actuators_ventilation_disable_override(actuation_handler_t self) {
    for(;;) {
        if (pdTRUE == xSemaphoreTake(self->override_sema, pdMS_TO_TICKS(50)))
            break;
    }

    self->ventilation_overriden = pdFALSE;

    xSemaphoreGive(self->override_sema);
}

void actuators_aircon_override_state(actuation_handler_t self, aircon_state_t state) {
    for(;;) {
        if (pdTRUE == xSemaphoreTake(self->override_sema, pdMS_TO_TICKS(50)))
            break;
    }

    self->aircon_overriden = pdTRUE;
    rc_servo_setPosition(AIRCON, state);

    xSemaphoreGive(self->override_sema);
}

void actuators_aircon_disable_override(actuation_handler_t self) {
    for(;;) {
        if (pdTRUE == xSemaphoreTake(self->override_sema, pdMS_TO_TICKS(50)))
            break;
    }

    self->aircon_overriden = pdFALSE;

    xSemaphoreGive(self->override_sema);
}

void actuators_turn_on_off(actuation_handler_t self, actuator_state_t state) {
    while(1) {
        if (pdTRUE == xSemaphoreTake(self->actuator_state_sema, pdMS_TO_TICKS(50))) {
           break;
        }
    }

    if ((state == ACTUATORS_OFF) && (state != self->actuator_state)) {
        actuators_ventilation_override_state(self, VENT_OFF);
        actuators_aircon_override_state(self, AIRCON_OFF);
        printf("ACTUATORS_OFF\n");
    } else {
        printf("ACTUATORS_ON\n");
    }

    self->actuator_state = state;
    xSemaphoreGive(self->actuator_state_sema);
}

actuator_state_t actuators_get_state(actuation_handler_t self) {
    actuator_state_t tmp = ACTUATORS_OFF;
    while(1) {
        if (pdTRUE == xSemaphoreTake(self->actuator_state_sema, pdMS_TO_TICKS(100))) {
           tmp = self->actuator_state;
           break;
        }
    }

    xSemaphoreGive(self->actuator_state_sema);
    return tmp;
}