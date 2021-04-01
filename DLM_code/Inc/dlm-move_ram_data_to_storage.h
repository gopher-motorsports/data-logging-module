// dlm-move_ram_data_to_storage.h
//  header file for dlm-move_ram_data_to_storage.c


#ifndef DLM_MOVE_RAM_DATA_TO_STORAGE_H
#define DLM_MOVE_RAM_DATA_TO_STORAGE_H


// includes
#include "base_types.h"
#include "GopherCAN.h"
#include "dlm-storage_structs.h"


// function prototypes
void move_ram_data_to_storage_init(DATA_INFO_NODE* storage_ptr, const char* filename);
S8 write_data_and_handle_errors();
S8 write_data_to_storage();
void build_data_string(U8* data_str, DATA_INFO_NODE* data_node);
double convert_data_to_dpf(DATA_INFO_NODE* data_node);
S8 mount_sd_card();
S8 create_new_file(const char* filename);


// defines
// sizes of the different parts of the persistant storage data points. Sizes in Bytes
#define STORAGE_PARAM_SIZE 2
#define TIMESTAMP_SIZE 4
#define DATA_SIZE 8
#define DATA_POINT_STORAGE_SIZE STORAGE_PARAM_SIZE+TIMESTAMP_SIZE+DATA_SIZE
#define MAX_FILENAME_SIZE 100
#define MAX_APPEND_SIZE 6
#define MAX_APPEND_NUM 0xFFFF

// sd status defines
typedef enum
{
	SD_MOUNTED = 0,
	SD_NOT_MOUNTED = -1,
	SD_MOUNTING_ERROR = -2
} SD_STATUS;

// error defines
#define RAM_SUCCESS 0
#define FILE_ERROR -1
#define NO_SD_CARD -2
#define SD_ERROR -3
#define FILENAME_TOO_LONG -4
#define EMPTY_DATA_BUFF -5


#endif


// End of dlm-move_ram_data_to_storage.h
