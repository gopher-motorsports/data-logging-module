// dlm-manage_data_aquisition.h
//  header file for manage_data_aquisition.c

#ifndef DLM_MANAGE_DATA_AQUISITION_H
#define DLM_MANAGE_DATA_AQUISITION_H

// includes
#include "base_types.h"
#include "GopherCAN.h"
#include "dlm-storage_structs.h"

// function prototypes
void manage_data_aquisition_init();
void manage_data_aquisition_deinit();
void set_bucket_size(U8 sending_dam, void* UNUSED,
    U8 bucket_id, U8 num_of_params, U8 UNUSED2, U8 UNUSED3);
void add_param_to_bucket(U8 sending_dam, void* UNUSED,
	U8 bucket_id, U8 param_id_msb, U8 param_id_lsb, U8 UNUSED3);
void assign_bucket_to_frq(U8 sending_dam, void* UNUSED,
    U8 bucket_id, U8 ms_between_requests_msb, U8 ms_between_requests_lsb, U8 UNUSED3);
void request_all_buckets();
void store_new_data();
S8 add_param_to_ram(BUCKET_PARAM_INFO* param_node, BUCKET_NODE* bucket_node);


// defines
#define DLM_SUCCESS 0
#define DLM_MALLOC_ERROR -1
#define DLM_DATATYPE_NOT_FOUND -2

// error enum (MDA = manage data aquisition)
typedef enum
{
    NO_MDA_ERROR = 0,
    MDA_MALLOC_ERROR = -1,
    MDA_CAN_ERROR = -2
} MDA_ERROR;

#endif


// End of dlm-manage_data_aquisition.h
