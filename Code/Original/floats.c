#include "floats.h"

/*
    a and b should be integer representations of signed float
    example: 0.2 = 0.2<<15 = 6553 and -0.2 = -6553

    returns integer representation of the float product


    TODO: use high and low registers for multiplication, and the increase FLOAT_SHIFT for smaller units
*/
float_t fMult(float_t a, float_t b)
{
    //32-bit integers (solution 1)
    //int64_t product = (int64_t)a * (int64_t)b;
    //return product >> FLOAT_SHIFT;
    
    //32-bit integers (solution 2)
    //return ( (int64_t)a * (int64_t)b ) >> FLOAT_SHIFT;

    //64-bit integers (solution 3) (fewest assembly instructions)
    int64_t product = a * b;
    return product >> FLOAT_SHIFT;

    //64-bit integers (solution 4)
    //return (a*b) >> FLOAT_SHIFT;
}

