// dlm-move_ram_data_to_storage.h
//  header file for dlm-move_ram_data_to_storage.c


#ifndef DLM_MOVE_RAM_DATA_TO_STORAGE_H
#define DLM_MOVE_RAM_DATA_TO_STORAGE_H


// includes
#include "../../../STM32_CAN/GopherCAN.h"
//#include "GopherCAN.h"
#include "dlm-storage_structs.h"


// function prototypes
// TODO


// defines
// sizes of the different parts of the persistant storage data points. Sizes in Bytes
#define STORAGE_PARAM_SIZE 2
#define TIMESTAMP_SIZE 4
#define DATA_SIZE 8
#define DATA_POINT_STORAGE_SIZE STORAGE_PARAM_SIZE+TIMESTAMP_SIZE+DATA_SIZE


#endif


// End of dlm-move_ram_data_to_storage.h