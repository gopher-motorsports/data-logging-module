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

// The head node for the linked list of all of the buckets.
//  A linked list is a good canidate for storing all of the buckets because they will need
//  to be squentially run through in order to send the correct request, and the DLM
//  must be able to handle a general amount of them.
BUCKET_NODE bucket_list_head = {{0, 0, 0, 0, 0, 0, NULL}, NULL};


// manage_data_aquisition_init
//  Assign the pointer to the head node, set up the CAN commands, and tell the DAMs to start
//  defining their buckets
void manage_data_aquisition_init(void)
{
    // Add the correct CAN command functions
    add_custom_can_func(SET_BUCKET_SIZE, &set_bucket_size, TRUE, NULL);
    add_custom_can_func(ADD_PARAM_TO_BUCKET, &add_param_to_bucket, TRUE, NULL);
    add_custom_can_func(ASSIGN_BUCKET_TO_FRQ, &assign_bucket_to_frq, TRUE, NULL);

    // Send CAN commands to all modules (specifically to the DAMs) that
    // this module is ready to be interacted with to add buckets
    send_can_command(PRIO_HIGH, ALL_MODULES_ID, SEND_BUCKET_PARAMS, 0, 0, 0, 0);
}


// manage_data_aquisition_deinit
//  Called if the DLM needs to delete all of the buckets. Will free all associated memory
void manage_data_aquisition_deinit()
{
    // TODO
}


// set_bucket_size
//  function that will handle the SET_BUCKET_SIZE command from the DAM. This
//  will search for the bucket in the LL with the correct ID, or create a new
//  one if it does not exsist
void set_bucket_size(U8 sending_dam, void* UNUSED,
    U8 bucket_id, U8 num_of_params, U8 UNUSED2, U8 UNUSED3)
{
    BUCKET_NODE* above_bucket_node = &bucket_list_head;
    BUCKET_NODE* bucket_node = bucket_list_head.next;

    // check if there exists a bucket with this ID on this DAM in the bucket list
    while (bucket_node != NULL)
    {
        if ((bucket_node->bucket.dam_id == sending_dam)
            && (bucket_node->bucket.bucket_id == bucket_id))
        {
            // This is the correct bucket. Move on to the next step
            break;
        }

        // this is not the correct bucket. Try the next one
        above_bucket_node = bucket_node;
        bucket_node = bucket_node->next;
    }

    // if the bucket does not exist, make a new one and use it
    if (bucket_node == NULL)
    {
        bucket_node = (BUCKET_NODE*)malloc(sizeof(BUCKET_NODE));

        // test if malloc failed
        if (bucket_node == NULL)
        {
            // resend the command to restart the sequence and note the error
            send_can_command(PRIO_HIGH, sending_dam, SEND_BUCKET_PARAMS, 0, 0, 0, 0);
            set_error_state(DLM_ERR_MALLOC_ERR);
            return;
        }

        // this is needed to make sure the list knows to stop at the end
        bucket_node->next = NULL;
        bucket_node->bucket.param_ids = NULL;

        // set the details of this new bucket
        bucket_node->bucket.dam_id = sending_dam;
        bucket_node->bucket.bucket_id = bucket_id;

        // Disable this bucket by setting the ms_between_requests to 0
        bucket_node->bucket.ms_between_requests = 0;
        bucket_node->bucket.last_request = 0;

        // set the above node to this new node
        above_bucket_node->next = bucket_node;
    }

    // set the number of parameters in this bucket. No parameters have been added yet
    bucket_node->bucket.params_added = 0;
    bucket_node->bucket.num_of_params = num_of_params;

    // free any memory that may have been used for the old parameter array
    free(bucket_node->bucket.param_ids);

    // malloc some memory for the new parameter array
    bucket_node->bucket.param_ids = (BUCKET_PARAM_INFO*)malloc(num_of_params * sizeof(BUCKET_PARAM_INFO));

    // test if the malloc failed
    if (bucket_node->bucket.param_ids == NULL)
	{
		// resend the command to restart the sequence and note the error
		send_can_command(PRIO_HIGH, sending_dam, SEND_BUCKET_PARAMS, 0, 0, 0, 0);
		set_error_state(DLM_ERR_MALLOC_ERR);
		return;
	}
}


// add_param_to_bucket
//  This function is a CAN command, designed to be activated by the DAM. When
//  called, this will add the param inputted to the correct bucket with the assosiated
//  DAM included. Built to handle a general amount of DAMs, params, and buckets
void add_param_to_bucket(U8 sending_dam, void* UNUSED,
	U8 bucket_id, U8 param_id_msb, U8 param_id_lsb, U8 UNUSED3)
{
    BUCKET_NODE* bucket_node = bucket_list_head.next;
    BUCKET_PARAM_INFO* param_array;
    U16 param_id;
    U8 c;

    // create the param_id from the two 8-bit chunks
    param_id = (param_id_msb << BITS_IN_BYTE) | param_id_lsb;

    // check if there exists a bucket with this ID on this DAM in the bucket list
    while (bucket_node != NULL)
    {
        if ((bucket_node->bucket.dam_id == sending_dam)
            && (bucket_node->bucket.bucket_id == bucket_id))
        {
            // This is the correct bucket. Move on to the next step
            break;
        }

        // this is not the correct bucket. Try the next one
        bucket_node = bucket_node->next;
    }

    // if the bucket does not exist, resend SEND_BUCKET_PARAMS to ask the DAM
    // to start the process over again
    if (bucket_node == NULL)
    {
        // there may be some repeats when this is sent, but that is ok
        send_can_command(PRIO_HIGH, sending_dam, SEND_BUCKET_PARAMS, 0, 0, 0, 0);
        return;
    }

    // check if this bucket is already full. If it is, tell the DAM this bucket is ready
    // without adding the sent parameter
    if (bucket_node->bucket.params_added == bucket_node->bucket.num_of_params)
	{
		send_can_command(PRIO_HIGH, sending_dam, BUCKET_OK, bucket_id, 0, 0, 0);
	}

    // get a pointer to the location of the parameter array
    param_array = bucket_node->bucket.param_ids;

    // Check to make sure this parameter is not already in the list. Do not add it
    // if it is
    for (c = 0; c < bucket_node->bucket.params_added; c++)
    {
        if (param_array[c].parameter == param_id)
        {
            // this parameter is a duplicate. Return without adding a new one
            return;
        }
    }

    // set the details of the param_node in the first open spot of the array
    param_array[c].parameter = param_id;
    param_array[c].pending_responce = FALSE;

    // increase the number of parameters added to this bucket
    bucket_node->bucket.params_added++;

    // if the number of params in this bucket is equal to the desired
    // size, send a BUCKET_OK command to the correct DAM
    if (bucket_node->bucket.params_added == bucket_node->bucket.num_of_params)
    {
        send_can_command(PRIO_HIGH, sending_dam, BUCKET_OK, bucket_id, 0, 0, 0);
    }
}


// assign_bucket_to_frq
//  This will take the inputed DAM and bucket ID and set the time to wait between each request
//  in ms. Designed to be called as a CAN command coming from a DAM
void assign_bucket_to_frq(U8 sending_dam, void* UNUSED,
    U8 bucket_id, U8 ms_between_requests_msb, U8 ms_between_requests_lsb, U8 UNUSED3)
{
	// Skip the head node when searching
    BUCKET_NODE* bucket_node = bucket_list_head.next;
    U16 ms_between_requests;

    // create the U16 for ms_between_requests out of the 2 U8s
    ms_between_requests = (ms_between_requests_msb << BITS_IN_BYTE) | ms_between_requests_lsb;

    // check if there exists a bucket with this ID on this DAM in the bucket list
    while (bucket_node != NULL)
    {
        if ((bucket_node->bucket.dam_id == sending_dam)
            && (bucket_node->bucket.bucket_id == bucket_id))
        {
            // This is the correct bucket. Assign the ms_between_request variable in the struct as needed
            bucket_node->bucket.ms_between_requests = ms_between_requests;
            return;
        }

        // this is not the correct bucket. Try the next one
        bucket_node = bucket_node->next;
    }

    // The correct bucket was not found. Return
}


// request_all_buckets
//  Function to run through the list of buckets and checks if they need to be requested. If they do,
//  request it
void request_all_buckets(void)
{
	// Skip the head node
    BUCKET_NODE* bucket_node = bucket_list_head.next;
    BUCKET_PARAM_INFO* param_array;
    U8 c;

    while(bucket_node != NULL)
    {
        // check if it is the correct time to send a new message. 0ms between requests means the
        // bucket is not fully initialized
        if ((bucket_node->bucket.ms_between_requests != 0)
            && (HAL_GetTick() >= bucket_node->bucket.last_request + bucket_node->bucket.ms_between_requests))
        {
            // send the command to request the bucket
            if (send_can_command(PRIO_HIGH, bucket_node->bucket.dam_id,
                REQUEST_BUCKET, bucket_node->bucket.bucket_id, 0, 0, 0) != CAN_SUCCESS)
            {
                // set the last error variable to note the CAN error
                set_error_state(DLM_ERR_CAN_ERR);
            }

            // set the pending response flag for each parameter in this bucket to true
            param_array = bucket_node->bucket.param_ids;
            for (c = 0; c < bucket_node->bucket.params_added; c++)
            {
            	param_array->pending_responce = TRUE;
            	param_array++;
            }

            // update the last request tick
            bucket_node->bucket.last_request = HAL_GetTick();
        }

        // move on to the next bucket
        bucket_node = bucket_node->next;
    }

    // done with requesting the buckets
}


// store_new_data
//  Function to figure out what data stored in the GopherCAN parameters is new
//  based on data in the bucket linked list. If it deturmines the data is new,
//  store that data to the data ring buffer
void store_new_data(PPBuff* sd_buffer, PPBuff* telem_buffer)
{
	// Skip the head node
    BUCKET_NODE* bucket_node = bucket_list_head.next;
    BUCKET_PARAM_INFO* param_array;
    CAN_INFO_STRUCT* param_info;
    U8 c;

    // For each parameter in each bucket, check if the last time it was
    // received is sooner than its bucket was requested and has not been already written
    while (bucket_node != NULL)
    {
        param_array = bucket_node->bucket.param_ids;

        // run through each parameter in the bucket
        for (c = 0; c < bucket_node->bucket.params_added; c++)
        {
            // get the CAN_INFO_STRUCT related to this parameter. Data stores the parameter ID in the node struct
            param_info = (CAN_INFO_STRUCT*)(all_parameter_structs[param_array->parameter]);

            // if the parameter is pending an update and the last RX of the param is after the
            // request was sent, it needs to be added to RAM
            if (param_info->last_rx >= bucket_node->bucket.last_request &&
            	!param_info->pending_response)
            {
                // add the param data to RAM
            	DLM_ERRORS_t error = add_param_to_ram(param_array, bucket_node, sd_buffer, telem_buffer);
                if (error != DLM_ERR_NO_ERR)
                {
                	set_error_state(error);
                	return;
                }

                // successfully added the data point to ram
                clear_error_state(DLM_ERR_RAM_FAIL);

                // the pending_responce flag is being hijacked for saving whether
				// this data point has been logged
				param_info->pending_response = TRUE;
            }

            // move on to the next parameter
            param_array++;
        }
        
        // move on to the next bucket
        bucket_node = bucket_node->next;
    }
}


// add_param_to_ram
//  Function to add the data of a specific parameter to the RAM buffer
DLM_ERRORS_t add_param_to_ram(BUCKET_PARAM_INFO* param_info, BUCKET_NODE* bucket_node,
									 PPBuff* sd_buffer, PPBuff* telem_buffer)
{
	// add the data to the PPBuffs for both the SD write and telem buffers
    CAN_INFO_STRUCT* can_param_struct = (CAN_INFO_STRUCT*)(all_parameter_structs[param_info->parameter]);
    U8 data_size;
    DLM_ERRORS_t error;
    void* data_ptr;
    // get the size of the parameter based on the data type
    switch (parameter_data_types[param_info->parameter])
	{
	case UNSIGNED8:
		data_size = sizeof(U8);
		data_ptr = &((U8_CAN_STRUCT*)can_param_struct)->data;
		break;
	case UNSIGNED16:
			data_size = sizeof(U16);
			data_ptr = &((U16_CAN_STRUCT*)can_param_struct)->data;
			break;
	case UNSIGNED32:
			data_size = sizeof(U32);
			data_ptr = &((U32_CAN_STRUCT*)can_param_struct)->data;
			break;
	case UNSIGNED64:
			data_size = sizeof(U64);
			data_ptr = &((U64_CAN_STRUCT*)can_param_struct)->data;
			break;
	case SIGNED8:
			data_size = sizeof(S8);
			data_ptr = &((S8_CAN_STRUCT*)can_param_struct)->data;
			break;
	case SIGNED16:
			data_size = sizeof(S16);
			data_ptr = &((S16_CAN_STRUCT*)can_param_struct)->data;
			break;
	case SIGNED32:
			data_size = sizeof(S32);
			data_ptr = &((S32_CAN_STRUCT*)can_param_struct)->data;
			break;
	case SIGNED64:
			data_size = sizeof(S64);
			data_ptr = &((S64_CAN_STRUCT*)can_param_struct)->data;
			break;
	case FLOATING:
			data_size = sizeof(float);
			data_ptr = &((FLOAT_CAN_STRUCT*)can_param_struct)->data;
			break;
	default:
		// the datatype is not found for some reason
        return DLM_ERR_DATATYPE;
	}

    // set the time the data was taken as the time is was requested, as there is less
    // TX delay than RX delay
    if (osMutexAcquire(mutex_storage_bufferHandle,
    				   MUTEX_GET_TIMEOUT_ms) != osOK) return DLM_ERR_MUTEX;
    error = append_packet(sd_buffer, STORAGE_BUFFER_SIZE, bucket_node->bucket.last_request,
		      param_info->parameter, data_ptr, data_size);
    if (osMutexRelease(mutex_storage_bufferHandle) != osOK) return DLM_ERR_MUTEX;
    if (error != DLM_ERR_NO_ERR) return error;


	// TODO: only append whitelisted packets to telem buffer
	if (osMutexAcquire(mutex_broadcast_bufferHandle,
					   MUTEX_GET_TIMEOUT_ms) != osOK) return DLM_ERR_MUTEX;
	error = append_packet(telem_buffer, BROADCAST_BUFFER_SIZE, bucket_node->bucket.last_request,
				  param_info->parameter, data_ptr, data_size);
	if (osMutexRelease(mutex_broadcast_bufferHandle) != osOK) return DLM_ERR_MUTEX;
	if (error != DLM_ERR_NO_ERR) return error;

    return DLM_SUCCESS;
}


// End of dlm-manage_data_aquisition.c
