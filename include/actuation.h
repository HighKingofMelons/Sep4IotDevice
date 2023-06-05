/** @file
 * @brief This is a actuation header file.
 *
 * It handles the actuation.
 */
#pragma once
/**
 * @brief Structure to store actuation information.
 *
 * It hold a 5 attributes and is needed for accesing and using actuation.
 */
typedef struct actuation_handler *actuation_handler_t;

/**
 * @brief Initiatizes the actuation handler.
 *
 * @param temperature
 * @param humidity
 * @return actuation_handler_t
 */
actuation_handler_t actuation_handler_init(temperature_t temperature, humidity_t humidity, co2_handler_t co2);
/**
 * @brief Destroys the actuation handler.
 *
 * @param victim
 */
void actuation_handler_destroy(actuation_handler_t victim);

typedef enum {
    VENT_ON     =  100,
    VENT_OFF    = -100
} vent_state_t;
/*
    Takes pdTRUE for ON and pdFALSE for OFF
*/

/**
 * @brief Ventilation actuators overide state.
 *
 * @param self
 * @param state
 */
void actuators_ventilation_override_state(actuation_handler_t self, vent_state_t state);
/**
 * @brief Ventilation actuators disable state.
 *
 * @param self
 */
void actuators_ventilation_disable_override(actuation_handler_t self);

typedef enum {
    AIRCON_HEAT =  100,
    AIRCON_COOL = -100,
    AIRCON_OFF  =  0
} aircon_state_t;

typedef enum {
    ACTUATORS_ON =  1,
    ACTUATORS_OFF = 0,
} actuator_state_t;
/**
 * @brief Ventilation actuators overide state.
 *
 * @param self
 * @param state
 */
void actuators_aircon_override_state(actuation_handler_t self, aircon_state_t state);
/**
 * @brief Ventilation actuators disable state.
 *
 * @param self
 */
void actuators_aircon_disable_override(actuation_handler_t self);

void actuators_turn_on_off(actuation_handler_t self, actuator_state_t state);