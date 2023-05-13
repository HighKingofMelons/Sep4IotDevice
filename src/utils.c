<<<<<<< HEAD
#include "utils.h"

/*
    Multiplies the input with 10 and casts it to an unsigned 16bit integer
    Returns 0 if cast would cause an overflow 
*/
uint16_t float_to_int_x10(float input) {
    float multiple = input * 10;

    if ( multiple < UINT16_MAX)
        return (uint16_t) multiple;

    return 0;
<<<<<<< HEAD
=======
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
>>>>>>> 8903bb4 (Co2 impelentation, not tests)
}
=======
}
>>>>>>> a1854d6 (Testing prep progress)
