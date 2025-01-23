#include <nfp.h>
#include "semaphore.h"

void semaphore_down(volatile __declspec(mem addr40) void * addr)
{
    /* Claim semaphore /wait for semaphore */

    unsigned int addr_hi, addr_lo;
    __declspec(read_write_reg) int xfer;
    SIGNAL_PAIR my_signal_pair;

    addr_hi = ((unsigned long long int)addr >> 8) & 0xff000000;
    addr_lo = (unsigned long long int)addr & 0xffffffff;

    do
    {
        
        xfer = 1;
        __asm
        {
            mem[test_subsat, xfer, addr_hi, <<8, addr_lo, 1], \
                sig_done[my_signal_pair];
            ctx_arb[my_signal_pair]
        }
    } while(xfer==0);
}

void semaphore_up(volatile __declspec(mem addr40) void * addr)
{
    /* release semaphore */
    unsigned int addr_hi, addr_lo;
    __declspec(read_write_reg) int xfer;

    addr_hi = ((unsigned long long int)addr >> 8) & 0xff000000;
    addr_lo = (unsigned long long int)addr & 0xffffffff;

    __asm
    {
        mem[incr, --, addr_hi, <<8, addr_lo, 1];
    }
}