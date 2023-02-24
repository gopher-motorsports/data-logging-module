// dlm-manage_data_aquisition.c
//  Code for managing data requests and storage from the DAMs


// includes
#include "dlm-manage_data_aquisition.h"
#include "dlm-error_handling.h"
#include <stdlib.h>
#include "dlm-storage_structs.h"
#include "base_types.h"
#include "GopherCAN.h"
#include "cmsis_os.h"
#include "main.h"
#include "dlm-high_level_functions.h"
#include "dlm-util.h"


// array to store the last ms each of the parameters was added
// to memory
static U32 last_log[NUM_OF_PARAMETERS] = {0};


// store_new_data
//  Run through each GCAN parameter, log it if there has been a new value sent by
//  comparing last_rx on the GCAN variable and the stored array last_log
void store_new_data(PPBuff* sd_buffer, PPBuff* telem_buffer)
{
	CAN_INFO_STRUCT* param_info;

	// NOTE: start at param ID 1 because param ID 0 is reserved
	for (U8 c = 1; c < NUM_OF_PARAMETERS; c++)
	{
		// get the CAN_INFO_STRUCT related to this parameter. Data stores the parameter ID in the node struct
		param_info = (CAN_INFO_STRUCT*)(PARAMETERS[c]);

		// check if this param was received again since the last time it was logged
		if (param_info->last_rx > last_log[c])
		{
			// add the param data to RAM
			DLM_ERRORS_t error = add_param_to_ram(param_info, sd_buffer, telem_buffer);
			if (error != DLM_ERR_NO_ERR)
			{
				set_error_state(error);
				return;
			}

			// successfully added the data point to ram
			clear_error_state(DLM_ERR_RAM_FAIL);

			// set the last log time for this parameter
			last_log[c] = param_info->last_rx;
		}
    }
}


// add_param_to_ram
//  Function to add the data of a specific parameter to the RAM buffer
DLM_ERRORS_t add_param_to_ram(CAN_INFO_STRUCT* param, PPBuff* sd_buffer,
		                      PPBuff* telem_buffer)
{
	// add the data to the PPBuffs for both the SD write and telem buffers
    U8 data_size;
    DLM_ERRORS_t error;
    void* data_ptr;
    // get the size of the parameter based on the data type
    switch (param->TYPE)
	{
	case UNSIGNED8:
		data_size = sizeof(U8);
		data_ptr = &((U8_CAN_STRUCT*)param)->data;
		break;
	case UNSIGNED16:
			data_size = sizeof(U16);
			data_ptr = &((U16_CAN_STRUCT*)param)->data;
			break;
	case UNSIGNED32:
			data_size = sizeof(U32);
			data_ptr = &((U32_CAN_STRUCT*)param)->data;
			break;
	case UNSIGNED64:
			data_size = sizeof(U64);
			data_ptr = &((U64_CAN_STRUCT*)param)->data;
			break;
	case SIGNED8:
			data_size = sizeof(S8);
			data_ptr = &((S8_CAN_STRUCT*)param)->data;
			break;
	case SIGNED16:
			data_size = sizeof(S16);
			data_ptr = &((S16_CAN_STRUCT*)param)->data;
			break;
	case SIGNED32:
			data_size = sizeof(S32);
			data_ptr = &((S32_CAN_STRUCT*)param)->data;
			break;
	case SIGNED64:
			data_size = sizeof(S64);
			data_ptr = &((S64_CAN_STRUCT*)param)->data;
			break;
	case FLOATING:
			data_size = sizeof(float);
			data_ptr = &((FLOAT_CAN_STRUCT*)param)->data;
			break;
	default:
		// the datatype is not found for some reason
        return DLM_ERR_DATATYPE;
	}

    // set the time the data was taken as the time is was requested, as there is less
    // TX delay than RX delay
    if (osMutexAcquire(mutex_storage_bufferHandle,
    				   MUTEX_GET_TIMEOUT_ms) != osOK) return DLM_ERR_MUTEX;
    error = append_packet(sd_buffer, STORAGE_BUFFER_SIZE, param->last_rx,
		      (U16)param->ID, data_ptr, data_size);
    if (osMutexRelease(mutex_storage_bufferHandle) != osOK) return DLM_ERR_MUTEX;
    if (error != DLM_ERR_NO_ERR) return error;


	// TODO: only append whitelisted packets to telem buffer
	if (osMutexAcquire(mutex_broadcast_bufferHandle,
					   MUTEX_GET_TIMEOUT_ms) != osOK) return DLM_ERR_MUTEX;
	error = append_packet(telem_buffer, BROADCAST_BUFFER_SIZE, param->last_rx,
				(U16)param->ID, data_ptr, data_size);
	if (osMutexRelease(mutex_broadcast_bufferHandle) != osOK) return DLM_ERR_MUTEX;
	if (error != DLM_ERR_NO_ERR) return error;

    return DLM_SUCCESS;
}


// End of dlm-manage_data_aquisition.c
