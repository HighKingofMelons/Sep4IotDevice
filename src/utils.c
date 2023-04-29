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
}