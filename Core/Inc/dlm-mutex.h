// dlm-mutex.h
//  Header file for dlm-mutex.c


#ifndef DLM_MUTEX_H
#define DLM_MUTEX_H

#include "base_types.h"


// the mutex struct
typedef enum
{
	MUTEX_UNLOCKED = 0,
	MUTEX_LOCKED = 1
} DLM_MUTEX;


// externs for the different mutexes
extern volatile DLM_MUTEX ram_data_mutex;


// functions for locking and unlocking mutexes
boolean get_mutex_lock(volatile DLM_MUTEX* mutex);
void release_mutex(volatile DLM_MUTEX* mutex);


#endif // DLM_MUTEX_H


// End of dlm-mutex.h
