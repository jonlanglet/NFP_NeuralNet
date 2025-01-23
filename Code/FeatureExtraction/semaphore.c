#include <nfp.h>
#include <pif_plugin.h>
#include <pif_plugin_metadata.h>
#include <pif_registers.h>
#include <pif_pkt.h>

#include "semaphore.h"







void semaphore_down(volatile __declspec(mem addr40) void * addr) {

	/* semaphore "DOWN" = claim = wait */

	unsigned int addr_hi, addr_lo;

	__declspec(read_write_reg) int xfer;

	SIGNAL_PAIR my_signal_pair;

	addr_hi = ((unsigned long long int)addr >> 8) & 0xff000000;

	addr_lo = (unsigned long long int)addr & 0xffffffff;

	do {

		xfer = 1;

		__asm {

			mem[test_subsat, xfer, addr_hi, <<8, addr_lo, 1], sig_done[my_signal_pair];
			ctx_arb[my_signal_pair]

		}

	} while (xfer == 0);

}



void semaphore_up(volatile __declspec(mem addr40) void * addr) {

	/* semaphore "UP" = release = signal */

	unsigned int addr_hi, addr_lo;

	__declspec(read_write_reg) int xfer;

	addr_hi = ((unsigned long long int)addr >> 8) & 0xff000000;

	addr_lo = (unsigned long long int)addr & 0xffffffff;



	__asm {

		mem[incr, --, addr_hi, <<8, addr_lo, 1];

	}



}





// lock a semaphore based on an ID in the metadata.

int pif_plugin_semaphore_lock(EXTRACTED_HEADERS_T *headers, MATCH_DATA_T *data){

	// Get the ID of the semaphore to lock from the packet header.

	__declspec(local_mem) int sem_sid;

	//one set of semaphores per flow
	sem_sid = (int)pif_plugin_meta_get__scalars__metadata__flowIndex(headers)*SEM_LOCKS_PER_FLOW + (int)pif_plugin_meta_get__scalars__metadata__semaphore_id(headers);

	

	// Lock that semaphore.

	semaphore_down( &global_semaphores[sem_sid]);

	

	return PIF_PLUGIN_RETURN_FORWARD;

}

// Release a semaphore based on an ID in the metadata.

int pif_plugin_semaphore_release(EXTRACTED_HEADERS_T *headers, MATCH_DATA_T *data){

	// Get the ID of the semaphore to lock from the packet header.

	// Would like to store this in metadata, but..

	__declspec(local_mem) int sem_sid;

	//one set of semaphores per flow
	//sem_sid = (int)pif_plugin_meta_get__scalars__metadata__semaphore_id(headers);
	sem_sid = (int)pif_plugin_meta_get__scalars__metadata__flowIndex(headers)*SEM_LOCKS_PER_FLOW + (int)pif_plugin_meta_get__scalars__metadata__semaphore_id(headers);

	

	// Release that semaphore.

	semaphore_up( &global_semaphores[sem_sid]);



	return PIF_PLUGIN_RETURN_FORWARD;

}



void initSemaphores()
{
	int i;

	for(i = 0; i < SEM_COUNT; i++)
		global_semaphores[i] = 1;
}

