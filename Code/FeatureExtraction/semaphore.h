#ifndef __LANGLET_SEMAPHORE_H
    #define __LANGLET_SEMAPHORE_H
    
    #include "config.h"


    __declspec(imem export aligned(64)) int global_semaphores[SEM_COUNT];

    //function prototypes
    void initSemaphores();

#endif