// dlm-move_ram_data_to_storage.c
//  This file handles moving data stored in the RAM buffer to an off-chip storage medium, in
//  this case an SD card. This file also has code to mount the SD card and make a new file
//  with a unique name for logging


// self include
#include "dlm-move_ram_data_to_storage.h"
#include "GopherCAN.h"
#include "dlm-storage_structs.h"
#include "fatfs.h"
#include "cmsis_os.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dlm-error_handling.h"
#include "main.h"

static void run_sd_led(void);
static SD_WRITE_ERR_t write_data_to_storage(PPBuff* sd_buffer);
static SD_WRITE_ERR_t mount_sd_card(void);
static SD_WRITE_ERR_t create_new_file(const char* filename);

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

// Status and error handling things
SD_STATUS sd_status = SD_NOT_MOUNTED;
U8 error_counter = 0;
static GPIO_TypeDef* sd_led_port = NULL;
static U16 sd_led_pin = 0;


// move_ram_data_to_storage_init
//  This function sets the local pointers to the correct values. Mounting the SD card is handled in the
//  writing data loop
void move_ram_data_to_storage_init(const char* filename, GPIO_TypeDef* led_port, U16 led_pin)
{
    orig_actual_file_name = filename;

    sd_led_port = led_port;
    sd_led_pin = led_pin;

    // toggle the LED once to note the init
    HAL_GPIO_TogglePin(sd_led_port, sd_led_pin);
}


// write_data_and_handle_errors
//  this function will check if the SD card is mounted, make the file if needed,
//  call write_data_to_storage, and handle file errors by unmounting the SD card.
//  designed to be called with high frequency, but will not write to the SD every
//  time the function is called
void write_data_and_handle_errors(PPBuff* sd_buffer)
{
	SD_WRITE_ERR_t error_code = 0;
	static U32 last_write;

	run_sd_led();

	// check if it is time to write to the SD, right now just do every
	// 500ms, but maybe we could have smarter logic later
	if (HAL_GetTick() - last_write < SD_WRITE_DELAY)
	{
		return;
	}
	last_write = HAL_GetTick();

	// check if the SD card is mounted
	if (sd_status != SD_MOUNTED)
	{
		// try again to mount the SD card. If it fails, set the global error
		// state and return
		error_code = mount_sd_card();

		switch (error_code)
		{
		case SD_SUCCESS:
			// clear error states and move on
			clear_error_state(DLM_ERR_SD_MOUNT);
			break;

		case NO_SD_CARD:
			// it is fine we could not mount, return without clearing any previous
			// errors
			return;

		default:
			// The SD card failed to mount even though it was detected. Set an error
			// state and return
			set_error_state(DLM_ERR_SD_MOUNT);
			return;
		}

		// if the code reaches this point, the SD should be mounted with a new file made. Create
		// a new file for the log file
		if (create_new_file(orig_actual_file_name) != SD_SUCCESS)
		{
			// there was an error creating the file. Unmount and return
			f_mount(NULL, SDPath, 1);
			set_error_state(DLM_ERR_FILE_CREATE);
			return;
		}

		// we were able to create a new file, clear the FILE CREATE error
		clear_error_state(DLM_ERR_FILE_CREATE);
	}

	// now try to write to the file
	error_code = write_data_to_storage(sd_buffer);

	switch (error_code)
	{
	case SD_SUCCESS:
		// clear all SD writing errors, we are all good now
		clear_error_state(DLM_ERR_NO_DATA);
		clear_error_state(DLM_ERR_FILE_WRITE);
		clear_error_state(DLM_ERR_MUTEX);

		// toggle the SD LED to signal a successful write
		if (sd_led_port) HAL_GPIO_TogglePin(sd_led_port, sd_led_pin);
		// message to indicate the DLM is working
		send_can_command(PRIO_HIGH, ALL_MODULES_ID, LOG_COMPLETE, 0, 0, 0, 0);
		error_counter = 0;
		return;

	case EMPTY_DATA_BUF:
		// no data in the buffer. There may be a CAN error
		set_error_state(DLM_ERR_NO_DATA);
		return;

	case MUTEX_ERROR:
		// os error
		set_error_state(DLM_ERR_MUTEX);
		return;

	case FILE_ERROR:
	default:
		// note this error and check if too many in a row have occurred
		error_counter++;

		if (error_counter >= MAX_NUM_OF_ERRORS)
		{
			// we have failed to write many times in a row after a mounting. Something is very wrong,
			// so reset the system and hope it works better next time
			NVIC_SystemReset();
		}

		set_error_state(DLM_ERR_FILE_WRITE);
		return;
	}
}


// write_data_to_storage
//  Function to run through each data node in the ram_data linked list while adding the data
//  to the SD card and deleting the node from the list. This function does not need to be thread
//  safe as the STM32 is single threaded (except for RX and TX interrupts, which only affect
//  the CAN buffers)
static SD_WRITE_ERR_t write_data_to_storage(PPBuff* sd_buffer)
{
	FRESULT fresult;
	UINT bytes_written; // UINT for the compiler to be happy with fatfs

	// we only need to get the mutex when we ping pong the buffer
	if (osMutexAcquire(mutex_storage_bufferHandle,
			SD_MUTEX_GET_TIMEOUT_ms) != osOK) return MUTEX_ERROR;
	// ping-pong the storage buffer
	U32 transferSize = sd_buffer->fill;
	sd_buffer->fill = 0;
	sd_buffer->write = !sd_buffer->write;
	if (osMutexRelease(mutex_storage_bufferHandle) != osOK) return MUTEX_ERROR;

	// check to make sure the buffer is not empty
	if (transferSize == 0) return EMPTY_DATA_BUF;

	// transfer the read half of the ping pong buffer to SD card
	fresult = f_write(&SDFile, sd_buffer->buffs[!sd_buffer->write], transferSize, &bytes_written);

	if (transferSize != bytes_written)
	{
		// TODO better error handling, possibly keep attempting to
		// write the rest
		return FILE_ERROR;
	}

	if (fresult != FR_OK)
	{
		file_error_code = fresult;
		return FILE_ERROR;
	}

    // sync the file. this replaces opening and closing the file
    fresult = f_sync(&SDFile);

    if (fresult != FR_OK)
	{
		file_error_code = fresult;
		return FILE_ERROR;
	}

    // everything worked. Return
    return SD_SUCCESS;
}


// mount_sd_card
//  this function will try to mount the SD card
static SD_WRITE_ERR_t mount_sd_card(void)
{
	FRESULT fresult;

	// check if the SD card is even inserted using the GPIOpin
	//if (HAL_GPIO_ReadPin(SD_Detected_GPIO_Port, SD_Detected_Pin) == GPIO_PIN_RESET)
	if (HAL_GPIO_ReadPin(SD_Detected_GPIO_Port, SD_Detected_Pin) == GPIO_PIN_SET)
	{
		// the SD is not inserted. Do not try to mount as it can cause a hardfault
		sd_status = SD_NOT_INSERTED;
		return NO_SD_CARD;

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
		return MOUNT_FAILURE;
	}

	// check if another error has occurred
	else if (fresult != FR_OK)
	{
		file_error_code = fresult;
		sd_status = SD_MOUNTING_ERROR;
		return MOUNT_FAILURE;
	}

	// the mounting worked
	sd_status = SD_MOUNTED;
	return SD_SUCCESS;
}


// create_new_file
//  this function will create a new file for the DLM to write to, appending numbers to the end
//  of the file name if needed. This function will modify actual_file_name as needed
static SD_WRITE_ERR_t create_new_file(const char* filename)
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
		// NOTE: this adds a number after the file type, not after the name
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
	return SD_SUCCESS;
}


// run_sd_led
//  This function is non-blocking and will blink the led according to the current
//  SD status. If there is no error, the light will be toggled on each successful
//  write
static void run_sd_led(void)
{
	static U32 last_blink_time;
	static U8 num_led_blinks;

	// dont hardfault
	if (!sd_led_port) return;

	// if the SD is ok, let the write task handle blinking on each successful write
	if (sd_status == SD_MOUNTED) return;

	// there is an error active
	if (!num_led_blinks)
	{
		// long delay and reset
		if (HAL_GetTick() - last_blink_time >= ERR_WAIT_TIME)
		{
			HAL_GPIO_WritePin(sd_led_port, sd_led_pin, RESET);
			last_blink_time = HAL_GetTick();
			num_led_blinks = (U8)sd_status << 1; // double so there is an on and off for each blink number
		}
	}
	else
	{
		if (HAL_GetTick() - last_blink_time >= ERR_BLINK_TIME)
		{
			HAL_GPIO_TogglePin(sd_led_port, sd_led_pin);
			last_blink_time = HAL_GetTick();
			num_led_blinks--;
		}
	}

	return;
}


// End of dlm-move_ram_data_to_storage.c
