// dlm-move_ram_data_to_storage.c
//  TODO DOCS


// self include
#include "dlm-move_ram_data_to_storage.h"
#include "GopherCAN.h"
#include "dlm-storage_structs.h"
#include "stm32f7xx_hal.h"
#include "fatfs.h"
#include <stdlib.h>


// This is the same head for the RAM storage linked list in manage_data_aquisition
DATA_INFO_NODE* ram_data_head_ptr;
extern U8 parameter_data_types[NUM_OF_PARAMETERS];

// File handling externs
extern char SDPath[4];				// path to the SD card
extern FATFS SDFatFS;				// file system struct
extern FIL SDFile;					// singular file struct

// SD related variables
FRESULT file_error_code = FR_OK;
U32 bytes_written;

// move_ram_data_to_storage_init
//  TODO DOCS
void move_ram_data_to_storage_init(DATA_INFO_NODE* storage_ptr)
{
    ram_data_head_ptr = storage_ptr;

    // TODO try to mount the SD or make notes about its status

    // TODO Init the file? possibly do it when beginning the logging session instead
}


// write_data_to_storage
//  Function to run through each data node in the ram_data linked list while adding the data
//  to the USB and deleting the node from the list. This function does not need to be thread
//  safe as the STM32 is single threaded (except for RX and TX interrupts, which only affect
//  the CAN buffers)
S8 write_data_to_storage(const char* file_name)
{
	FRESULT fresult;
    DATA_INFO_NODE* data_node_above = ram_data_head_ptr;
    DATA_INFO_NODE* data_node = ram_data_head_ptr->next;
    U8 data_point_str[DATA_POINT_STORAGE_SIZE];

    // make sure the SD is mounted
    // TODO replace this with something better, some logic to check if the SD is already mounted
    // Third arg as 1 means mount immediately, 0 means delayed mount
    fresult = f_mount(&SDFatFS, SDPath, 1);

    // TODO check if the error is FR_DISK_ERR (means the sd card is not inserted)

    // open the file. This will create a new file if it does not already exist, but it should as the file
    // metadata should already be there
    fresult = f_open(&SDFile, file_name, FA_CREATE_NEW|FA_OPEN_APPEND|FA_WRITE);

    if (fresult == FR_EXIST)
    {
    	fresult = f_open(&SDFile, file_name, FA_OPEN_APPEND|FA_WRITE);
    }

    // check to make sure the file actually opened
    if (fresult != FR_OK)
    {
    	file_error_code = fresult;
    	return FILE_ERROR;
    }

    // run through each data node in the RAM LL
    while (data_node != NULL)
    {
        // build the data string for this node
        build_data_string(data_point_str, data_node);

        // append the file with this new string
        fresult = f_write(&SDFile, data_point_str, DATA_POINT_STORAGE_SIZE, (UINT*)(&bytes_written));

        // check if the writing was successful
        if (fresult != FR_OK)
        {
        	file_error_code = fresult;
        	return FILE_ERROR;
        }

        // remove the pointer from the LL
        data_node_above->next = data_node->next;

        // free the memory for this node, stdlib is smart enough to know how much memory was
        // originally malloced
        free(data_node);

        // move on to the next data node
        data_node = data_node_above->next;
    }

    // close the file
    fresult = f_close(&SDFile);

    // make sure the file was actually closed
    if (fresult != FR_OK)
    {
    	file_error_code = fresult;
    	return FILE_ERROR;
    }

    // unmount SD. TODO this will not be needed when there is better logic for the SD card
    fresult = f_mount(&SDFatFS, NULL, 1);

    // everything worked. Return
    return RAM_SUCCESS;
}


// build_data_string
//  Convert the linked list data node in RAM to a string of data. Each
//  data point will be stored as a 16bit parameter id, 32bit time value representing
//  the ms from DLM startup, and 64bit double for the data value. This string is returned
//  in U8* data_str. This must have 14B of memory available, or else bad things will happen.
//  This function will also remove the data node from the LL
void build_data_string(U8 data_str[], DATA_INFO_NODE* data_node)
{
    DPF_CONVERTER data_union;
    U8 c;

    // write the parameter to the first 2 bytes
    for (c = 0; c < STORAGE_PARAM_SIZE; c++)
    {
        data_str[c] = (U8)(data_node->param >> (((STORAGE_PARAM_SIZE - 1) - c) * BITS_IN_BYTE));
    }

    // write the timestamp to the next 4 bytes
    for (c = 0; c < TIMESTAMP_SIZE; c++)
    {
        data_str[c + STORAGE_PARAM_SIZE] = (U8)(data_node->data_time >> (((TIMESTAMP_SIZE - 1) - c) * BITS_IN_BYTE));
    }

    // write the double of the data to the last 8 bytes
    data_union.d = convert_data_to_dpf(data_node);
    for (c = 0; c < DATA_SIZE; c++)
    {
        data_str[c + STORAGE_PARAM_SIZE + TIMESTAMP_SIZE] = (U8)(data_union.u64 >> (((DATA_SIZE - 1) - c) * BITS_IN_BYTE));
    }
}


// convert_data_to_dpf
//  Function to take in a data node, get the data stored in it, and return
//  the double precision float representation of that value to be stored on
//  the external USB
double convert_data_to_dpf(DATA_INFO_NODE* data_node)
{
    // switch to get the data out of the data_node
    switch (parameter_data_types[data_node->param])
	{
	case UNSIGNED8:
		return (double)(((U8_DATA_NODE*)data_node)->data);

	case UNSIGNED16:
		return (double)(((U16_DATA_NODE*)data_node)->data);

	case UNSIGNED32:
		return (double)(((U32_DATA_NODE*)data_node)->data);

	case UNSIGNED64:
		return (double)(((U64_DATA_NODE*)data_node)->data);

	case SIGNED8:
		return (double)(((S8_DATA_NODE*)data_node)->data);

	case SIGNED16:
		return (double)(((S16_DATA_NODE*)data_node)->data);

	case SIGNED32:
		return (double)(((S32_DATA_NODE*)data_node)->data);

	case SIGNED64:
		return (double)(((S64_DATA_NODE*)data_node)->data);

	case FLOATING:
		return (double)(((FLOAT_DATA_NODE*)data_node)->data);

	default:
        // Something went wrong, just write 0 to data
		return 0;
	}

    // this coude should not be reached, this is to make the compiler happy
    return 0;
}


// End of dlm-move_ram_data_to_storage.c
