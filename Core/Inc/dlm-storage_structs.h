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
	// BETTER_BUCKETS
//	U8 pending_responce;
	U32 last_log;
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

// ping-pong buffer struct
typedef struct PPBuff {
	uint8_t* buffs[2]; // pointers to 2 byte buffers
	uint8_t write; // index of the write buffer (0 or 1)
	uint32_t fill; // fill level of the write buffer (in bytes)
} PPBuff;

#endif


// End of dlm-storage_structs.h
