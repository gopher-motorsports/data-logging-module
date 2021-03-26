// dlm-move_ram_data_to_storage.c
//  This file handles moving data stored in the RAM buffer to an off-chip storage medium, in
//  this case an SD card. This file also has code to mount the SD card and make a new file
//  with a unique name for logging


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
const char* orig_file_name;			// needed for if the file did not open the first time
char file_name[MAX_FILENAME_SIZE];
S8 sd_status = SD_NOT_MOUNTED;

// move_ram_data_to_storage_init
//  This function sets the local pointers to the correct values, then attempts to mount the
//  SD card and create the storage file
void move_ram_data_to_storage_init(DATA_INFO_NODE* storage_ptr, const char* filename)
{
    ram_data_head_ptr = storage_ptr;
    orig_file_name = filename;

    // try to mount the SD card
    if (mount_sd_card())
    {
    	return;
    }

    // open the file, creating a new name if it already exists
    if (create_new_file(filename))
    {
    	// failed to create a new file. Unmount the SD card
    	f_mount(NULL, SDPath, 1);
    }
}


// write_data_and_handle_errors
//  this function will call write_data_to_storage, and handle file errors by unmounting the
//  SD card
void write_data_and_handle_errors()
{
	if (write_data_to_storage() == FILE_ERROR)
	{
		// unmount the SD card and try to mount it again next cycle
		f_mount(NULL, SDPath, 1);
	}

	// TODO toggle the onboard LED every successful file write
}


// write_data_to_storage
//  Function to run through each data node in the ram_data linked list while adding the data
//  to the SD card and deleting the node from the list. This function does not need to be thread
//  safe as the STM32 is single threaded (except for RX and TX interrupts, which only affect
//  the CAN buffers)
S8 write_data_to_storage()
{
	FRESULT fresult;
    DATA_INFO_NODE* data_node_above = ram_data_head_ptr;
    DATA_INFO_NODE* data_node = ram_data_head_ptr->next;
    U8 data_point_str[DATA_POINT_STORAGE_SIZE];

    // check if the SD card is mounted
    if (sd_status != SD_MOUNTED)
    {
    	// check if there was an SD error
    	if (sd_status != SD_NOT_MOUNTED)
    	{
    		// if there is an SD error, return without trying to mount again
    		return SD_ERROR;
    	}

    	// try again to mount the SD card. If it fails, return
		if (mount_sd_card())
		{
			return SD_MOUNTING_ERROR;
		}

		// if the code reaches this point, the SD should be mounted with a new file made. Create
		// a new file for the log file
		if (create_new_file(orig_file_name))
		{
			// there was an error creating the file. Return
			return FILE_ERROR;
		}
    }

    // open the file. Mounting the sd card also will create the file
    if ((fresult = f_open(&SDFile, file_name, FA_OPEN_APPEND|FA_WRITE)) != FR_OK)
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
        if ((fresult = f_write(&SDFile, data_point_str, DATA_POINT_STORAGE_SIZE, (UINT*)(&bytes_written))) != FR_OK)
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
//  the external SD card
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


// mount_sd_card
//  this function will try to mount the SD card
S8 mount_sd_card()
{
	FRESULT fresult;

	// attempt to mount the card
	fresult = f_mount(&SDFatFS, SDPath, 1);

	// check if the error is FR_DISK_ERR (means the SD card is not inserted)
	if (fresult == FR_DISK_ERR)
	{
		file_error_code = fresult;
		sd_status = SD_NOT_MOUNTED;
		return sd_status;
	}

	// check if another error has occurred
	else if (fresult != FR_OK)
	{
		file_error_code = fresult;
		sd_status = SD_MOUNTING_ERROR;
		return sd_status;
	}

	// the mounting worked
	sd_status = SD_MOUNTED;
	return sd_status;
}


// create_new_file
//  this function will create a new file for the DLM to write to, appending numbers to the end
//  of the file name if needed. This function will modify file_name as needed
S8 create_new_file(const char* filename)
{
	FRESULT fresult;
	U8 c = 0;
	U16 append_val = 0;

	// save the original file name. This is basically strcpy but with the DLM max filename size
	do {
		// check if the filename is too long
		if (c >= MAX_FILENAME_SIZE)
		{
			// terminate the filename at the max length
			file_name[MAX_FILENAME_SIZE - 1] = '\0';
		}

		// copy this character
		file_name[c] = filename[c];
	} while (filename[c] != '\0');

	// open the file
	fresult = f_open(&SDFile, file_name, FA_CREATE_NEW|FA_WRITE);

	// if this filename is already taken, append a number on it and try again until it works
	while (fresult == FR_EXIST)
	{
		// put a number on it, then try again
		// TODO
	}

	// check to make sure there wasn't an error
	if (fresult != FR_OK)
	{
		file_error_code = fresult;
		return FILE_ERROR;
	}

	// everything worked. Return
	return RAM_SUCCESS;
}


// End of dlm-move_ram_data_to_storage.c
