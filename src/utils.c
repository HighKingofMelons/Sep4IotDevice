/*
 * utils.c
 *
 * Created: 5/11/2023 3:15:39 PM
 *  Author: takat
 */ 
#include "include/utils.h"

uint16_t float_to_int_x10(float input) {
	float multiple = input * 10;

	if ( multiple < UINT16_MAX)
	return (uint16_t) multiple;

	return 0;
}
