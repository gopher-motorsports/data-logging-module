// dlm-manage_data_aquisition.c
//  Code for managing data requests and storage from the DAMs


// includes
#include "dlm-manage_data_aquisition.h"
#include <stdlib.h>
#include "dlm-storage_structs.h"
#include "base_types.h"
#include "GopherCAN.h"
#include "stm32f7xx_hal_gpio.h"


// The head node for the linked list of all of the buckets.
//  A linked list is a good canidate for storing all of the buckets because they will need
//  to be squentially run through in order to send the correct request, and the DLM
//  must be able to handle a general amount of them.
BUCKET_NODE bucket_list_head = {{0, 0, 0, 0, NULL}, NULL};

// Head node pointer and first pointer to the linked list for all of the data points
//  in the RAM data buffer. A linked list is a good candidate for many of the
//  same reasons as the bucket LL, but a head node is required as deletion will
//  be common
DATA_INFO_NODE* ram_data_head;

// variable to store the last error
MDA_ERROR last_mda_error = NO_MDA_ERROR;

// from GopherCAN.c
extern void* all_parameter_structs[NUM_OF_PARAMETERS];
extern U8 parameter_data_types[NUM_OF_PARAMETERS];


// manage_data_aquisition_init
//  Assign the pointer to the head node, set up the CAN commands, and tell the DAMs to start
//  defining their buckets
void manage_data_aquisition_init(DATA_INFO_NODE* ram_data)
{
    ram_data_head = ram_data;

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
            last_mda_error = MDA_MALLOC_ERROR;
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

    // set the number of parameters in this bucket
    bucket_node->bucket.num_of_params = num_of_params;
}


// add_param_to_bucket
//  This function is a CAN command, designed to be activated by the DAM. When
//  called, this will add the param inputted to the correct bucket with the assosiated
//  DAM included. Built to handle a general amount of DAMs, params, and buckets
void add_param_to_bucket(U8 sending_dam, void* UNUSED,
    U8 param_id_msb, U8 param_id_lsb, U8 bucket_id, U8 UNUSED3)
{
	BUCKET_NODE* above_bucket_node = &bucket_list_head;
    BUCKET_NODE* bucket_node = bucket_list_head.next;
    U16_LIST_NODE* param_node;
    U16 param_id;
    U8 params_in_this_bucket = 0;

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
        above_bucket_node = bucket_node;
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

    // Check to make sure this parameter is not already in the list. Do not add it
    // if it is
    // TODO

    // malloc some new memory for the U16 node to store the parameter
    param_node = (U16_LIST_NODE*)malloc(sizeof(U16_LIST_NODE));

    // test if malloc failed
    if (param_node == NULL)
    {
        // a failed malloc will result in the parameter not being added,
        // and in that case the bucket will not be completely filled, leading
        // to the DLM asking the DAM to fill the bucket again. However these
        // mallocs are unlikely
        last_mda_error = MDA_MALLOC_ERROR;
        return;
    }

    // add this param to the front of the param linked list. It can be added to
    // the front because order does not matter in this list
    param_node->next = bucket_node->bucket.param_ids;
    bucket_node->bucket.param_ids = param_node;

    // set the details of the param_node
    param_node->data = param_id;
    param_node->pending_responce = FALSE;

    // TODO if the number of params in this bucket is equal to the desired
    // size, send a BUCKET_OK command to the correct DAM
}


// assign_bucket_to_frq
//  This will take the inputted DAM and bucket ID and set the time to wait between each request
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
void request_all_buckets()
{
	// Skip the head node
    BUCKET_NODE* bucket_node = bucket_list_head.next;
    U16_LIST_NODE* param_node;

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
                last_mda_error = MDA_CAN_ERROR;
            }

            // set the pending responce flag for each parameter in this bucket to true
            param_node = bucket_node->bucket.param_ids;
            while (param_node != NULL)
            {
                param_node->pending_responce = TRUE;
                param_node = param_node->next;
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
void store_new_data()
{
	// Skip the head node
    BUCKET_NODE* bucket_node = bucket_list_head.next;
    U16_LIST_NODE* param_node;
    CAN_INFO_STRUCT* param_info;

    // For each parameter in each bucket, check if the last time it was
    // recieved is sooner than its bucket was requested and has not been already written
    while (bucket_node != NULL)
    {
        param_node = bucket_node->bucket.param_ids;

        // run through each parameter in the bucket
        while (param_node != NULL)
        {
            // get the CAN_INFO_STRUCT related to this parameter. Data stores the parameter ID in the node struct
            param_info = (CAN_INFO_STRUCT*)(all_parameter_structs[param_node->data]);

            // if the parameter is pending an update and the last RX of the param is after the
            // request was sent, it needs to be added to RAM
            if (param_node->pending_responce == TRUE
                && param_info->last_rx >= bucket_node->bucket.last_request)
            {
                // add the param data to RAM
                if (add_param_to_ram(param_node, bucket_node))
                {
                	// TODO malloc error handling

                	// for now, turn on the onboard LED (ld2, blue)
                	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
                	return;
                }

                // adding the parameter was successful. Turn off the malloc failure LED
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);

                // disable the pending responce flag
                param_node->pending_responce = FALSE;
            }

            // move on to the next parameter
            param_node = param_node->next;
        }
        
        // move on to the next bucket
        bucket_node = bucket_node->next;
    }
}


// add_param_to_ram
//  Function to add the data of a specific parameter to the RAM buffer
S8 add_param_to_ram(U16_LIST_NODE* param_node, BUCKET_NODE* bucket_node)
{
    // Data will be stored in a linked list of nodes that include what parameter
    //  (param_id), the ms since startup that the datapoint was requested, and the param data.
    //  The size of the data can be obtained using the lookup table in GopherCAN

    DATA_INFO_NODE* data_node;
    CAN_INFO_STRUCT* can_param_struct;

    can_param_struct = (CAN_INFO_STRUCT*)(all_parameter_structs[param_node->data]);

    // Choose the correct type of data node based on the parameter data type, then malloc the memory needed
    switch (parameter_data_types[param_node->data])
	{
	case UNSIGNED8: ;
        U8_DATA_NODE* u8_data_node = (U8_DATA_NODE*)malloc(sizeof(U8_DATA_NODE));

        // check for malloc failure
        if (u8_data_node == NULL)
        {
            return DLM_MALLOC_ERROR;
        }

		u8_data_node->data = ((U8_CAN_STRUCT*)(can_param_struct))->data;
        data_node = (DATA_INFO_NODE*)u8_data_node;

        break;

	case UNSIGNED16: ;
		U16_DATA_NODE* u16_data_node = (U16_DATA_NODE*)malloc(sizeof(U16_DATA_NODE));

        // check for malloc failure
        if (u16_data_node == NULL)
        {
            return DLM_MALLOC_ERROR;
        }

		u16_data_node->data = ((U16_CAN_STRUCT*)(can_param_struct))->data;
        data_node = (DATA_INFO_NODE*)u16_data_node;

        break;

	case UNSIGNED32: ;
		U32_DATA_NODE* u32_data_node = (U32_DATA_NODE*)malloc(sizeof(U32_DATA_NODE));

        // check for malloc failure
        if (u32_data_node == NULL)
        {
            return DLM_MALLOC_ERROR;
        }

		u32_data_node->data = ((U32_CAN_STRUCT*)(can_param_struct))->data;
        data_node = (DATA_INFO_NODE*)u32_data_node;

        break;

	case UNSIGNED64: ;
		U64_DATA_NODE* u64_data_node = (U64_DATA_NODE*)malloc(sizeof(U64_DATA_NODE));

        // check for malloc failure
        if (u64_data_node == NULL)
        {
            return DLM_MALLOC_ERROR;
        }

		u64_data_node->data = ((U64_CAN_STRUCT*)(can_param_struct))->data;
        data_node = (DATA_INFO_NODE*)u64_data_node;

        break;

	case SIGNED8: ;
		S8_DATA_NODE* s8_data_node = (S8_DATA_NODE*)malloc(sizeof(S8_DATA_NODE));

        // check for malloc failure
        if (s8_data_node == NULL)
        {
            return DLM_MALLOC_ERROR;
        }

		s8_data_node->data = ((S8_CAN_STRUCT*)(can_param_struct))->data;
        data_node = (DATA_INFO_NODE*)s8_data_node;

        break;

	case SIGNED16: ;
		S16_DATA_NODE* s16_data_node = (S16_DATA_NODE*)malloc(sizeof(S16_DATA_NODE));

        // check for malloc failure
        if (s16_data_node == NULL)
        {
            return DLM_MALLOC_ERROR;
        }

		s16_data_node->data = ((S16_CAN_STRUCT*)(can_param_struct))->data;
        data_node = (DATA_INFO_NODE*)s16_data_node;

        break;

	case SIGNED32: ;
		S32_DATA_NODE* s32_data_node = (S32_DATA_NODE*)malloc(sizeof(S32_DATA_NODE));

        // check for malloc failure
        if (s32_data_node == NULL)
        {
            return DLM_MALLOC_ERROR;
        }

		s32_data_node->data = ((S32_CAN_STRUCT*)(can_param_struct))->data;
        data_node = (DATA_INFO_NODE*)s32_data_node;

        break;

	case SIGNED64: ;
		S64_DATA_NODE* s64_data_node = (S64_DATA_NODE*)malloc(sizeof(S64_DATA_NODE));

        // check for malloc failure
        if (s64_data_node == NULL)
        {
            return DLM_MALLOC_ERROR;
        }

		s64_data_node->data = ((S64_CAN_STRUCT*)(can_param_struct))->data;
        data_node = (DATA_INFO_NODE*)s64_data_node;

        break;

	case FLOATING: ;
		FLOAT_DATA_NODE* float_data_node = (FLOAT_DATA_NODE*)malloc(sizeof(FLOAT_DATA_NODE));

        // check for malloc failure
        if (float_data_node == NULL)
        {
            return DLM_MALLOC_ERROR;
        }

		float_data_node->data = ((FLOAT_CAN_STRUCT*)(can_param_struct))->data;
        data_node = (DATA_INFO_NODE*)float_data_node;

        break;

	default:
		// the datatype is not found for some reason
        return DLM_DATATYPE_NOT_FOUND;
	}

    // set the time the data was taken as the time is was requested, as there is less
    // TX delay than RX delay
    data_node->data_time = bucket_node->bucket.last_request;

    // the parameter id is stored in the data of the parameter node
    data_node->param = param_node->data;

    // add the new node to the front of the list, after the head node
    data_node->next = ram_data_head->next;
    ram_data_head->next = data_node;

    return DLM_SUCCESS;
}


// End of dlm-manage_data_aquisition.c
