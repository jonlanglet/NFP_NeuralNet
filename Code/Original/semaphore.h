#ifndef __LANGLET_SEMAPHORE_H__
    #define __LANGLET_SEMAPHORE_H__
    

    //__declspec(ctm export aligned(64)) int my_semaphore;


    /*
        Claim a semaphore, or wait for it
    */
    void semaphore_down(volatile __declspec(mem addr40) void * addr);

    /*
        Release the semaphore
    */
    void semaphore_up(volatile __declspec(mem addr40) void * addr);

#endif