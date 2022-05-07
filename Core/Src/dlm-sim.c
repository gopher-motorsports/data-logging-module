/*
 * dlm-sim.c
 *
 *  Created on: Apr 3, 2022
 *      Author: jonathan
 */

// functions for running the DLM in simulation mode

#include "dlm-sim.h"
#include <stdlib.h>
#include "cmsis_os2.h"
#include "base_types.h"
#include "dlm-storage_structs.h"
#include "GopherCAN.h"
#include "dlm-error_handling.h"
#include "dlm-high_level_functions.h"
#include "main.h"
#include "dlm-util.h"

// this function is called every 1ms, so keep that in mind when doing timings
void sim_generate_data(PPBuff* sd_buffer, PPBuff* telem_buffer)
{
    static U32 packet_count = 0;
    static U32 last_gen = 0;

    // check if it the right time to add data to the buffer
    if (HAL_GetTick() - last_gen < DATA_GEN_MS_BETWEEN)
    {
    	return;
    }
	last_gen = HAL_GetTick();

	DLM_ERRORS_t error;

	// create data for each of the parameters required
	for (U16 pid = FIRST_GCAN_ID; pid < FIRST_GCAN_ID + NUM_CHANNELS; pid++)
	{
		U32 time = HAL_GetTick();

		// append a fake packet to storage buffer
		if (osMutexAcquire(mutex_storage_bufferHandle, MUTEX_GET_TIMEOUT_ms) != osOK)
		{
			set_error_state(DLM_ERR_MUTEX);
			return;
		}
		error = append_packet(sd_buffer, STORAGE_BUFFER_SIZE, time, pid, &packet_count, sizeof(packet_count));
		if (osMutexRelease(mutex_storage_bufferHandle) != osOK)
		{
			set_error_state(DLM_ERR_MUTEX);
			return;
		}
		if (error != DLM_ERR_NO_ERR) {
			set_error_state(error);
			return;
		}

		// append the same fake packet to telemetry buffer
		if (osMutexAcquire(mutex_broadcast_bufferHandle, MUTEX_GET_TIMEOUT_ms) != osOK)
		{
			set_error_state(DLM_ERR_MUTEX);
			return;
		}
		error = append_packet(telem_buffer, BROADCAST_BUFFER_SIZE, time, pid, &packet_count, sizeof(packet_count));
		if (osMutexRelease(mutex_broadcast_bufferHandle) != osOK)
		{
			set_error_state(DLM_ERR_MUTEX);
			return;
		}
		if (error != DLM_ERR_NO_ERR) {
			set_error_state(error);
			return;
		}

		packet_count++;
	}
}

void sim_swap_sd_buffer(PPBuff* sd_buffer)
{
	// get the mutex and ping pong the buffer
	if (osMutexAcquire(mutex_storage_bufferHandle, MUTEX_GET_TIMEOUT_ms) != osOK)
	{
		set_error_state(DLM_ERR_MUTEX);
		return;
	}

	sd_buffer->fill = 0;
	sd_buffer->write = !sd_buffer->write;

	if (osMutexRelease(mutex_storage_bufferHandle) != osOK)
	{
		set_error_state(DLM_ERR_MUTEX);
		return;
	}
}
