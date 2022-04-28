// dlm-move_ram_data_to_storage.h
//  header file for dlm-move_ram_data_to_storage.c


#ifndef DLM_MOVE_RAM_DATA_TO_STORAGE_H
#define DLM_MOVE_RAM_DATA_TO_STORAGE_H


// includes
#include "base_types.h"
#include "GopherCAN.h"
#include "dlm-storage_structs.h"
#include "stm32f7xx_hal.h"


// function prototypes
void move_ram_data_to_storage_init(DATA_INFO_NODE* storage_ptr, const char* filename,
								   GPIO_TypeDef* led_port, U16 led_pin);
void write_data_and_handle_errors();
void build_data_string(U8* data_str, DATA_INFO_NODE* data_node);

// defines
// sizes of the different parts of the persistent storage data points. Sizes in Bytes
#define MAX_FILENAME_SIZE 100
#define MAX_APPEND_SIZE 6
#define MAX_APPEND_NUM 0xFFFF
#define MAX_NUM_OF_ERRORS 10

// how often to write to the SD card
#define SD_WRITE_DELAY 500

// sd status defines
typedef enum
{
	SD_MOUNTED = 0,
	SD_NOT_MOUNTED = 1,
	SD_MOUNTING_ERROR = 2,
	SD_NOT_INSERTED = 3,
	SD_UN_MOUNTED = 4
} SD_STATUS;

// error defines
typedef enum
{
	SD_SUCCESS = 0,
	FILE_ERROR = 1,
	NO_SD_CARD = 2,
	SD_ERROR = 3,
	FILENAME_TOO_LONG = 4,
	EMPTY_DATA_BUF = 5,
	MOUNT_FAILURE = 6
} SD_WRITE_ERR_t;


#endif


// End of dlm-move_ram_data_to_storage.h
