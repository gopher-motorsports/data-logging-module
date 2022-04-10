// dlm-high_level_functions.h
//
// Created by Calvin Molitor

#ifndef DLM_HIGH_LEVEL_FUNCTIONS_H
#define DLM_HIGH_LEVEL_FUNCTIONS_H


// Defines
#define TESTER_FILE_NAME "/dlm_data.gdat"

// uncomment both for sim mode
#define SIMULATE_DATA_COLLECTION // this will simulate getting data from a DAM, putting it in the data buffer
//#define AUTO_CLEAR_DATA // this will simulate writing to an SD card, removing data from the data buffer


// Function prototypes
void dlm_init();
void manage_data_aquisition();
void move_ram_data_to_storage();
void transmit_ram_data();
void begin_logging_session();
void end_logging_session();
void eth_udp_cli();
void can_service_loop();


// Enums
typedef enum
{
	NOT_LOGGING = 0,
	LOGGING_ACTIVE = 1
} LOGGING_STATUS;


#endif


// End of dlm-high_level_functions.h
