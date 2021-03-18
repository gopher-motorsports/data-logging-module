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
extern char USBHPath[4];   // USBH logical drive path
extern FATFS USBHFatFS;    // File system object for USBH logical drive
extern FIL USBHFile;       // File object for USBH

// a specific file's details
FILINFO USBHfno;

// capacity related
FATFS *pUSBHFatFS;
U32 fre_clust;
U32 total_space;
U32 free_space;
U16 bytes_written = 0;

// Error codes for the USB host
FRESULT file_error_code = FR_OK;

// variable for if the USB is actually mounted
U8 usb_mounted = USB_NOT_MOUNTED;

// move_ram_data_to_storage_init
//  TODO DOCS
void move_ram_data_to_storage_init(DATA_INFO_NODE* storage_ptr)
{
    ram_data_head_ptr = storage_ptr;

    // TODO file name, metadata, lots of other things im sure
}


// write_data_to_storage
//  Function to run through each data node in the ram_data linked list while adding the data
//  to the USB and deleting the node from the list. This function does not need to be thread
//  safe as the STM32 is single threaded (except for RX and TX interrupts, which only affect
//  the CAN buffers)
void write_data_to_storage(const char* file_name)
{
	FRESULT fresult;
    DATA_INFO_NODE* data_node_above = ram_data_head_ptr;
    DATA_INFO_NODE* data_node = ram_data_head_ptr->next;
    U8 data_point_str[DATA_POINT_STORAGE_SIZE];

    // make sure the USB is mounted
    if (usb_mounted != USB_MOUNTED)
    {
    	return;
    }

    // check if the file exists
    fresult = f_stat(file_name, &USBHfno);
	if (fresult != FR_OK)
	{
		// the file does not exist. Create it then close it
		fresult = f_open(&USBHFile, file_name, FA_CREATE_NEW|FA_READ|FA_WRITE);
		if (fresult != FR_OK)
		{
			// failed to create file
			file_error_code = fresult;
			return;
		}

		// close it so it can later be opened in append mode
		fresult = f_close(&USBHFile);
		if (fresult != FR_OK)
		{
			// failed to close the file
			file_error_code = fresult;
		    return;
		}
	}

    // open the file
    fresult = f_open(&USBHFile, file_name, FA_OPEN_APPEND|FA_WRITE);

    // check to make sure the file actually opened
    if (fresult != FR_OK)
    {
    	file_error_code = fresult;
    	return;
    }

    // run through each data node in the RAM LL
    while (data_node != NULL)
    {
        // build the data string for this node
        build_data_string(data_point_str, data_node);

        // append the file with this new string
        fresult = f_write(&USBHFile, data_point_str, DATA_POINT_STORAGE_SIZE, (UINT*)(&bytes_written));

        // check if the writing was successful
        if (fresult != FR_OK)
        {
        	file_error_code = fresult;
        	return;
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
    fresult = f_close(&USBHFile);

    // make sure the file was actually closed
    if (fresult != FR_OK)
    {
    	file_error_code = fresult;
    	return;
    }
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


// mount_usb
//  TODO DOCS
void mount_usb(void)
{
	// attempt to mount the USB
	FRESULT fresult = f_mount(&USBHFatFS, USBHPath, 1);

	// if there is an error, set the error code to the global variable
	if (fresult != FR_OK)
	{
		file_error_code = fresult;
	}

	// note the USB has been mounted
	usb_mounted = USB_MOUNTED;
}


// unmount_usb
//  TODO DOCS
void unmount_usb(void)
{
	// attempt to unmount
	FRESULT fresult = f_mount(NULL, USBHPath, 1);

	// if there is an error, set the error code to the global variable
	if (fresult != FR_OK)
	{
		file_error_code = fresult;
	}

	// note the USB has been removed
	usb_mounted = USB_NOT_MOUNTED;
}


// check_usb_details
//  TODO DOCS
void check_usb_details(void)
{
    /* Check free space */
    f_getfree("", &fre_clust, &pUSBHFatFS);

    total_space = (U32)((pUSBHFatFS->n_fatent - 2) * pUSBHFatFS->csize * 0.5);
    free_space = (U32)(fre_clust * pUSBHFatFS->csize * 0.5);
}


// End of dlm-move_ram_data_to_storage.c
