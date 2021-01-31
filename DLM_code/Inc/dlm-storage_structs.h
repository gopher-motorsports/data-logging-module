// dlm-storage_structs.h
// structs for storing CAN data types


#ifndef DLM_STORAGE_STRUCTS_H
#define DLM_STORAGE_STRUCTS_H

#include "..\\C-Utils\\base_types.h"


// Union for converting a double to a U64
typedef union
{
	double d;
	U64 u64;
} DPF_CONVERTER;


// u16 linked list node for parameters struct
typedef struct U16_LIST_NODE U16_LIST_NODE;
struct U16_LIST_NODE
{
    U16 data;
    U16_LIST_NODE* next;
    U8 pending_responce;
};


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
