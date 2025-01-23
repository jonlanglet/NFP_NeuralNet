#include "misc.h"

uint32_t rand32()
{
    //use built-in function
    return local_csr_read(local_csr_pseudo_random_number);


    /*
        TODO: critical section? Shouldn't really be a big deal here

    */
    /*
    uint32_t randVal;

    randVal = ( rand32_multiplier * rand32_seed + rand32_increment ) % rand32_modulus;

    rand32_seed = randVal;

    return randVal;
    */
}

void srand32(uint32_t seed)
{
    //use built-in function
    local_csr_write( local_csr_pseudo_random_number, (local_csr_read(local_csr_timestamp_low) & 0xffffffffffff) +1 );
    return;
    
    /*
    rand32_seed = seed;
    rand32_multiplier = 1103515245;
    rand32_increment = 12345;
    rand32_modulus = 2<<31;
    */
}