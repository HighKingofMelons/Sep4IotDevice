#include <stdlib.h>

#include <ATMEGA_FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <rc_servo.h>

#include "taskConfig.h"
#include "temperature.h"
#include "humidity.h"
#include "co2.h"

#include "actuation.h"

TaskHandle_t actuation_task_h;

struct actuation_handler {
    temperature_t temp_handler;
    humidity_t humid_handler;

    SemaphoreHandle_t override_sema;
    BaseType_t ventilation_overriden;
    BaseType_t aircon_overriden;
};

enum actuator {
    VENTILATION = 0,
    AIRCON      = 1
};

void update_vent(actuation_handler_t self) {
    if (pdTRUE != xSemaphoreTake(self->override_sema, pdMS_TO_TICKS(50)))
        return;

    if (self->ventilation_overriden) {
        xSemaphoreGive(self->override_sema);
        return;
    }

    switch (humidity_acceptability_status(self->humid_handler))
    {
    case  1:
        rc_servo_setPosition(VENTILATION, VENT_ON);
        break;
    case  0:
    case -1:
        rc_servo_setPosition(VENTILATION, VENT_OFF);
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

    switch (temperature_acceptability_status(self->temp_handler))
    {
    case -1:
        rc_servo_setPosition(AIRCON, AIRCON_HEAT);
        break;
    case 1:
        rc_servo_setPosition(AIRCON, AIRCON_COOL);
        break;
    case 0:
        rc_servo_setPosition(AIRCON, AIRCON_OFF);
        break;
    }

    xSemaphoreGive(self->override_sema);
}

void actuation_task(void *pvParameters) {
    actuation_handler_t self = (actuation_handler_t) pvParameters;
    TickType_t lastDelay = xTaskGetTickCount();
    
    for(;;) {
        update_vent(self);
        update_aircon(self);

        xTaskDelayUntil(&lastDelay, pdMS_TO_TICKS(300000));
        lastDelay = xTaskGetTickCount();
    }
}

actuation_handler_t actuation_handler_init(temperature_t temperature, humidity_t humidity) {
    rc_servo_initialise();

    actuation_handler_t _handler = calloc(1, sizeof(struct actuation_handler));

    *_handler = (struct actuation_handler) {
        temperature,
        humidity,

        xSemaphoreCreateMutex(),
        pdFALSE,
        pdFALSE
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