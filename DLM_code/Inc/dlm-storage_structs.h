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


// Node of all of the data storage info, but not the data
typedef struct
{
    U32 data_time;
    U16 param;
    DATA_INFO_NODE* next;
} DATA_INFO_NODE;

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