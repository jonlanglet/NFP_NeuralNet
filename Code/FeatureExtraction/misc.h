#ifndef __LANGLET_MISC__
    #define __LANGLET_MISC__

    #include <nfp.h>

    #include "floats.h"

    /*
        Random-seeds and modifiers
    */
    volatile _declspec(emem export scope(global)) uint64_t rand32_seed;
    volatile _declspec(emem export scope(global)) uint64_t rand32_modulus;
    volatile _declspec(emem export scope(global)) uint32_t rand32_multiplier;
    volatile _declspec(emem export scope(global)) uint32_t rand32_increment;

    /*
        Function prototypes
    */

    /*
        Generates a 64-bit psuedo-random integer between 0 and rand64_modulus-1
        Based on a linear congruential generator, with a global seed.
        Modifiers are the same as from glibc's implementation
    */
    uint32_t rand32();
    
    /*
        Set a seed for rand32
        Also initializes modifiers, so it is essential to run this at least once
    */
    void srand32(uint32_t seed);

#endif