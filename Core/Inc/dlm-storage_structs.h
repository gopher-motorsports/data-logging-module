// dlm-storage_structs.h
// structs for storing CAN data types


#ifndef DLM_STORAGE_STRUCTS_H
#define DLM_STORAGE_STRUCTS_H

#include "base_types.h"


// Union for converting a double to a U64
typedef union
{
	double d;
	U64 u64;
} DPF_CONVERTER;


// parameter detail array for buckets
typedef struct
{
	U16 parameter;
	U8 pending_responce;
} BUCKET_PARAM_INFO;


// bucket struct
typedef struct
{
    U8 dam_id;                  	// this does not necessarily need to be a DAM, any gopherCAN module would work
    U8 bucket_id;               	// the ID of this bucket. This is used with dam_id to identify the bucket
    U16 ms_between_requests;    	// time between the requests in ms
    U32 last_request;           	// last time, in ms, the bucket was requested
    U8 num_of_params;           	// the number of parameters that will be stored in this bucket
    U8 params_added;				// the number of parameters that are currently in this bucket
    BUCKET_PARAM_INFO* param_ids;   // Pointer to the first node of an array of structs that represent each parameter in the bucket
} DAM_BUCKET;


// bucket linked list node struct
typedef struct BUCKET_NODE BUCKET_NODE;
struct BUCKET_NODE
{
    DAM_BUCKET bucket;
    BUCKET_NODE* next;
};


// Node of all of the data storage info, but not the data
typedef struct DATA_INFO_NODE DATA_INFO_NODE;
struct DATA_INFO_NODE
{
    U32 data_time;
    U16 param;
    DATA_INFO_NODE* next;
};

typedef struct
{
    U32 data_time;
    U16 param;
    DATA_INFO_NODE* next;
    U8 data;
} U8_DATA_NODE;

typedef struct
{
    U32 data_time;
    U16 param;
    DATA_INFO_NODE* next;
    U16 data;
} U16_DATA_NODE;

typedef struct
{
    U32 data_time;
    U16 param;
    DATA_INFO_NODE* next;
    U32 data;
} U32_DATA_NODE;

typedef struct
{
    U32 data_time;
    U16 param;
    DATA_INFO_NODE* next;
    U64 data;
} U64_DATA_NODE;

typedef struct
{
    U32 data_time;
    U16 param;
    DATA_INFO_NODE* next;
    S8 data;
} S8_DATA_NODE;

typedef struct
{
    U32 data_time;
    U16 param;
    DATA_INFO_NODE* next;
    S16 data;
} S16_DATA_NODE;

typedef struct
{
    U32 data_time;
    U16 param;
    DATA_INFO_NODE* next;
    S32 data;
} S32_DATA_NODE;

typedef struct
{
    U32 data_time;
    U16 param;
    DATA_INFO_NODE* next;
    S64 data;
} S64_DATA_NODE;

typedef struct
{
    U32 data_time;
    U16 param;
    DATA_INFO_NODE* next;
    float data;
} FLOAT_DATA_NODE;


#endif


// End of dlm-storage_structs.h
