// dlm-manage_data_aquisition.h
//  header file for manage_data_aquisition.c

#ifndef DLM_MANAGE_DATA_AQUISITION_H
#define DLM_MANAGE_DATA_AQUISITION_H

// includes
#include <stdlib.h>
#include "../../../C-Utils/base_types.h"
//#include "base_types.h"
#include "../../../STM32_CAN/GopherCAN.h"
//#include "GopherCAN.h"
#include "dlm-storage_structs.h"

// function prototypes
void manage_data_aquisition_init();
void manage_data_aquisition_deinit();
void add_param_to_bucket(U8 sending_dam, U16 param_id, U8 bucket_id);
void assign_bucket_to_frq(U8 sending_dam, U8 bucket_id, U16 ms_between_requests);
void request_all_buckets();
void store_new_data();
static void add_param_to_ram(U16_LIST_NODE* param_node);



// defines
#define BUCKET_REQUEST_COMMAND // TODO add this to the master spreadsheet

// structs
// bucket struct
typedef struct
{
    U8 dam_id;
    U8 bucket_id;
    U16 ms_between_requests; // time between the requests in ms
    U32 last_request; // last time, in ms, the bucket was requested
    U16_LIST_NODE* param_ids; // Pointer to the first node of a linked list of all of the parameters in this bucket
} DAM_BUCKET;


// bucket linked list node struct
typedef struct
{
    DAM_BUCKET bucket;
    BUCKET_NODE* next;
} BUCKET_NODE;


// u16 linked list node for parameters struct
typedef struct
{
    U16 data;
    U16_LIST_NODE* next;
    U8 pending_responce;
} U16_LIST_NODE;


#endif


// End of dlm-manage_data_aquisition.h