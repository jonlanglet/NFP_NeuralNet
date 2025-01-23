#ifndef __LANGLET_FLOATS_H_
    #define __LANGLET_FLOATS_H_


    #include <stdint.h>


    /*
        Convert a float to integer representation:
        example 0.2: 0.2*2^15 = 6553 and -0.2: -6553

        storing floats in 64-bit all the way through seems to be the most speed-optimized solution, but the most memory heavy
        Latency test would be nice
    */

    //make sure this is correct! 2<<30 = 2^31
    #define FLOAT_SHIFT 30 //15 //2^15 = 32768 | this results in accuracy 2^-15 ~= 3*10^-5 ~== 0.00003  (acceptable-ish)

    typedef int64_t float_t; //base floats on 64-bit integers (this feels so wrong)

    /*
        A list of 30-bit shifted float values
        Use these for debugging
    */
    #define val0        0           //0
    #define val0_01     10737418    //0.01
    #define val0_1      107374182   //0.1
    #define val0_25     268435456   //0.25
    #define val0_5      536870912   //0.5
    #define val0_75     805306368   //0.75
    #define val1_0      1073741824  //1.0

    
    /*
        Function prototypes
    */
    float_t fMult(float_t a, float_t b);


#endif