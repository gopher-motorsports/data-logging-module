// dlm-move_ram_data_to_storage.h
//  header file for dlm-move_ram_data_to_storage.c


#ifndef DLM_MOVE_RAM_DATA_TO_STORAGE_H
#define DLM_MOVE_RAM_DATA_TO_STORAGE_H


// includes
#include "base_types.h"
#include "GopherCAN.h"
#include "dlm-storage_structs.h"


// function prototypes
void move_ram_data_to_storage_init(DATA_INFO_NODE* storage_ptr);
S8 write_data_to_storage(const char* file_name);
void build_data_string(U8* data_str, DATA_INFO_NODE* data_node);
double convert_data_to_dpf(DATA_INFO_NODE* data_node);
void mount_usb(void);
void unmount_usb(void);
void check_usb_details(void);


// defines
// sizes of the different parts of the persistant storage data points. Sizes in Bytes
#define STORAGE_PARAM_SIZE 2
#define TIMESTAMP_SIZE 4
#define DATA_SIZE 8
#define DATA_POINT_STORAGE_SIZE STORAGE_PARAM_SIZE+TIMESTAMP_SIZE+DATA_SIZE

#define USB_MOUNTED 1
#define USB_NOT_MOUNTED 0

// error defines
#define RAM_SUCCESS 0
#define FILE_ERROR -1
#define USB_NOT_MOUNTED_YET -2


#endif


// End of dlm-move_ram_data_to_storage.h
