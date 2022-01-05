// dlm-mutex.c
//  File to hold and handle all mutexes on the DLM


// self include
#include "dlm-mutex.h"


// all of the needed mutexes
volatile DLM_MUTEX ram_data_mutex;


// get_mutex_lock
//  Returns true and sets the mutex as not available if the mutex was previously available
boolean get_mutex_lock(volatile DLM_MUTEX* mutex)
{
	if(*mutex == MUTEX_UNLOCKED)
	{
		// set the mutex state to locked
		*mutex = MUTEX_LOCKED;
		return TRUE;
	}

	// the mutex is not available
	return FALSE;
}


// release_muted
//  releases the mutex so other functions can use it
void release_mutex(volatile DLM_MUTEX* mutex)
{
	*mutex = MUTEX_UNLOCKED;
}


// End of dlm-mutex.c
