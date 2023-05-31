#pragma once
/*
 * LoRaWANHandler.h
 *
 * Created: 19/05/2023 21.37.43
 *  Author: andre
 */ 

typedef struct lorawan_handler* lorawan_handler_t;
lorawan_handler_t lorawan_handler_create(TickType_t last_messure_circle_time);
void lorawan_handler_set_uplink(lorawan_handler_t self, uint8_t bytes[10]);
uint8_t lorawan_handler_get_downlink(lorawan_handler_t self, uint8_t* bytes);
void lorawan_handler_destroy(lorawan_handler_t self);
