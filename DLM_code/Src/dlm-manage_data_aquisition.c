// dlm-manage_data_aquisition.c
//  TODO DOCS

#include "dlm-manage_data_aquisition.h"


// Pointer to the first node for the linked list of all of the buckets.
//  A linked list is a good canidate for storing all of the buckets because they will need
//  to be squentially run through in order to send the correct request, and the DLM
//  must be able to handle a general amount of them. A head node is not needed as deletion
//  is not a needed feature except in the case of deleting everything
BUCKET_NODE* first_bucket = NULL;


// manage_data_aquisition_init
//  TODO DOCS
void manage_data_aquisition_init()
{
    // Add the correct CAN command functions
    // TODO

    // Send CAN commands to all modules (specifically to the DAMs) that
    // this module is ready to be interacted with to add buckets
    // TODO
}


// manage_data_aquisition_deinit
//  Called if the DLM needs to delete all of the buckets. Will free all associated memory
void manage_data_aquisition_deinit()
{
    // TODO
}


// add_param_to_bucket
//  This function is a CAN command, designed to be activated by the DAM. When
//  called, this will add the param inputted to the correct bucket with the assosiated
//  DAM included. Built to handle a general amount of DAMs, params, and buckets
void add_param_to_bucket(U8 sending_dam, U16 param_id, U8 bucket_id)
{
    BUCKET_NODE* bucket_node = first_bucket;
    U16_LIST_NODE* param_node;

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

    // if the bucket does not exist, make a new one and use it
    if (bucket_node->next == NULL)
    {
        bucket_node->next = (BUCKET_NODE*)malloc(sizeof(BUCKET_NODE));
        bucket_node = bucket_node->next;

        // test if malloc failed
        if (bucket_node == NULL)
        {
            // TODO handle a malloc error
            return;
        }

        // this is needed to make sure the list knows to stop at the end
        bucket_node->next = NULL;
        bucket_node->bucket.param_ids = NULL;
    }

    // malloc some new memory for the U16 node to store the parameter
    param_node = (U16_LIST_NODE*)malloc(sizeof(U16_LIST_NODE));

    // test if malloc failed
    if (param_node == NULL)
    {
        // TODO handle a malloc error
        return;
    }

    // add this param to the front of the param linked list. It can be added to
    // the front because order does not matter in this list
    param_node->next = bucket_node->bucket.param_ids->next;
    bucket_node->bucket.param_ids->next = param_node;
}


// assign_bucket_to_frq
//  This will take the inputted DAM and bucket ID and set the time to wait between each request
//  in ms. Designed to be called as a CAN command coming from a DAM
void assign_bucket_to_frq(U8 sending_dam, U8 bucket_id, U16 ms_between_requests)
{
    BUCKET_NODE* bucket_node = first_bucket;

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
    BUCKET_NODE* bucket_node = first_bucket;

    while(bucket_node != NULL)
    {
        // check if it is the correct time to send a new message
        if (HAL_GetTick() >= bucket_node->bucket.last_request + bucket_node->bucket.ms_between_requests)
        {
            // send the command to request the bucket
            if (send_can_command(PRIO_HIGH, bucket_node->bucket.dam_id,
                BUCKET_REQUEST_COMMAND, bucket_node->bucket.bucket_id) != CAN_SUCCESS)
            {
                // TODO error handling
            }

            bucket_node->bucket.last_request = HAL_GetTick();
        }

        bucket_node = bucket_node->next;
    }
}