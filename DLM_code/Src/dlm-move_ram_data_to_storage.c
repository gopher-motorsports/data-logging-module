// dlm-move_ram_data_to_storage.c
//  This file handles moving data stored in the RAM buffer to an off-chip storage medium, in
//  this case an SD card. This file also has code to mount the SD card and make a new file
//  with a unique name for logging


// self include
#include "dlm-move_ram_data_to_storage.h"
#include "GopherCAN.h"
#include "dlm-storage_structs.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_gpio.h"
#include "fatfs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


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
const char* orig_actual_file_name;							// needed for if the file did not open the first time
char actual_file_name[MAX_FILENAME_SIZE + MAX_APPEND_SIZE];	// give extra characters for numbers on the end, just in case
SD_STATUS sd_status = SD_NOT_MOUNTED;
U8 error_counter = 0;

// move_ram_data_to_storage_init
//  This function sets the local pointers to the correct values. Mounting the SD card is handled in the
//  writing data loop
void move_ram_data_to_storage_init(DATA_INFO_NODE* storage_ptr, const char* filename)
{
    ram_data_head_ptr = storage_ptr;
    orig_actual_file_name = filename;

    // toggle the LED once to note the init
    HAL_GPIO_TogglePin(GPIOB, LED1_sd_write_Pin);
}


// write_data_and_handle_errors
//  this function will check if the SD card is mounted, make the file if needed,
//  call write_data_to_storage, and handle file errors by unmounting the SD card
S8 write_data_and_handle_errors()
{
	S8 error_code = 0;

	// check if the SD card is mounted
	if (sd_status != SD_MOUNTED)
	{
		// try again to mount the SD card. If it fails, return
		if (mount_sd_card())
		{
			return SD_MOUNTING_ERROR;
		}

		// if the code reaches this point, the SD should be mounted with a new file made. Create
		// a new file for the log file
		if (create_new_file(orig_actual_file_name))
		{
			// there was an error creating the file. Unmount and return
			f_mount(NULL, SDPath, 1);
			return FILE_ERROR;
		}
	}

	// now try to write to the file
	error_code = write_data_to_storage();

	// if a file operation fails, unmount and try remounting
	if (error_code == FILE_ERROR)
	{
		// note this error and check if too many in a row have occurred
		error_counter++;

		if (error_counter >= MAX_NUM_OF_ERRORS)
		{
			// try to close the file. This probably wont work at this point, but the thought is nice
			f_close(&SDFile);

			// unmount the SD card and try to mount it again next cycle
			// TODO remounting does not work at the moment, so this is basically giving up
			f_mount(NULL, SDPath, 1);
			sd_status = SD_NOT_MOUNTED;
		}

		return FILE_ERROR;
	}

	// if the data buffer is empty, there is prob a CAN error
	if (error_code == EMPTY_DATA_BUFF)
	{
		// TODO handle this in some way, maybe reset the DAM-DLM init process
		return EMPTY_DATA_BUFF;
	}

	// toggle the onboard LED every successful file write and reset the error counter
	HAL_GPIO_TogglePin(GPIOB, LED1_sd_write_Pin);
	error_counter = 0;

	return RAM_SUCCESS;
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

    // check to make sure there is at least one datapoint
    if (data_node == NULL)
    {
    	return EMPTY_DATA_BUFF;
    }

    // The file is already open if the SD is mounted

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

    // sync the file. this replaces opening and closing the file
    if ((fresult = f_sync(&SDFile)) != FR_OK)
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

	// check if the SD card is even inserted using the GPIOpin
	if (HAL_GPIO_ReadPin(SD_Detected_GPIO_Port, SD_Detected_Pin) == GPIO_PIN_RESET)
	{
		// the SD is not inserted. Do not try to mount as it can cause a hardfault
		sd_status = SD_NOT_INSERTED;
		return SD_NOT_INSERTED;

		// This logic replaces the FATfs logic for checking the SD card, that is why there is a warning
		// when auto-generating code for SD
	}

	// attempt to mount the card
	fresult = f_mount(&SDFatFS, SDPath, 1);

	// check if the error is FR_DISK_ERR
	if (fresult == FR_DISK_ERR || fresult == FR_NOT_READY)
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
//  of the file name if needed. This function will modify actual_file_name as needed
S8 create_new_file(const char* filename)
{
	FRESULT fresult;
	U16 append_val = 0;
	char append_str[MAX_APPEND_SIZE];

	// check if the filename is too long
	if (strlen(filename) >= MAX_FILENAME_SIZE)
	{
		return FILENAME_TOO_LONG;
	}

	// copy the original filename to actual_file_name so it can be modified
	strcpy(actual_file_name, filename);

	// try to open the file
	fresult = f_open(&SDFile, actual_file_name, FA_CREATE_NEW|FA_WRITE);

	// if this filename is already taken, append a number on it and try again until it works
	while (fresult == FR_EXIST)
	{
		// TODO this adds a number after the file type, not after the name
		append_val++;

		// revert actual_file_name to the target filename
		strcpy(actual_file_name, filename);

		// build the string we want to append to the actual_file_name
		sprintf(append_str, "%i", append_val);

		// put a number on the actual_file_name, then try again
		strcat(actual_file_name, append_str);
		fresult = f_open(&SDFile, actual_file_name, FA_CREATE_NEW|FA_WRITE);

		// if append_val gets unreasonably large, just delete the file with that large of
		// a number at the end
		if (append_val >= MAX_APPEND_NUM)
		{
			fresult = f_open(&SDFile, actual_file_name, FA_CREATE_ALWAYS|FA_WRITE);
		}
	}

	// check to make sure there wasn't an error
	if (fresult != FR_OK)
	{
		file_error_code = fresult;
		return FILE_ERROR;
	}

	// add the metadata to the first line. Right now it is just the filename
	f_printf(&SDFile, "%s:\n", actual_file_name);

	// everything worked. Return
	return RAM_SUCCESS;
}


// End of dlm-move_ram_data_to_storage.c
