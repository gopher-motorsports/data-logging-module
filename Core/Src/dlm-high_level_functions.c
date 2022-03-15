// dlm-high_level_functions.c
// 
// Created by Calvin Molitor
//
// This file contains the main high-level organization for the DLM.
//  These functions are designed to be called by the RTOS in main.c, and
//  correlate with the requirements for this module ('DLM Requirements' in
//  google drive)
//
// Note current function docs will likely change significantly when the
//  functions are complete
//
// Each function will call functions will only call functions in files
//  with the name 'dlm-FUNCTION_NAME.c'. Ex: the function 'manage_data_aquisition'
//  will call functions from the file 'dlm-manage_data_aquisition.c'. This
//  is to help with overall code organization


// self include
#include "dlm-high_level_functions.h"
#include "base_types.h"
#include "GopherCAN.h"
#include "dlm-storage_structs.h"
#include "dlm-manage_data_aquisition.h"
#include "dlm-move_ram_data_to_storage.h"
#include "dlm-manage_logging_session.h"


// Global Variables
// this is the head node for the RAM data linked list
DATA_INFO_NODE ram_data =
{
		.data_time = 0,
		.param = 0,
		.next = NULL
};

// the HAL_CAN structs
CAN_HandleTypeDef* dlm_hcan1;
CAN_HandleTypeDef* dlm_hcan2;
CAN_HandleTypeDef* dlm_hcan3;

char dlm_file_name[MAX_FILENAME_SIZE];

// variable to store the logging status
LOGGING_STATUS logging_status = NOT_LOGGING;

// DEBUG
#include "GopherCAN.h"
#include "main.h"
static void change_led_state(U8 sender, void* parameter, U8 remote_param, U8 UNUSED1, U8 UNUSED2, U8 UNUSED3);

// dlm_init
//  This function will handle power-on behavior, all completely TBD
//  according to everything else the module does
void dlm_init(CAN_HandleTypeDef* hcan_ptr1, CAN_HandleTypeDef* hcan_ptr2,
		CAN_HandleTypeDef* hcan_ptr3)
{
    // init GopherCAN
	dlm_hcan1 = hcan_ptr1;
	dlm_hcan2 = hcan_ptr2;
	dlm_hcan3 = hcan_ptr3;

	// initialize CAN
	// NOTE: CAN will also need to be added in CubeMX and code must be generated
	// Check the STM_CAN repo for the file "Fxxx CAN Config Settings.pptx" for the correct settings
	if (init_can(dlm_hcan1, DLM_ID, BXTYPE_MASTER)
			|| init_can(dlm_hcan2, DLM_ID, BXTYPE_SLAVE)
			|| init_can(dlm_hcan3, DLM_ID, BXTYPE_MASTER))
	{
		// an error has occurred, stay here
		while (1);
	}

	// Declare which bus is which using define_can_bus
	define_can_bus(dlm_hcan1, GCAN0, 0);
	define_can_bus(dlm_hcan2, GCAN1, 1);
	define_can_bus(dlm_hcan3, GCAN2, 2);

	// enable the tester variables
	set_all_params_state(TRUE);

	// use the RTC to generate the filename
	generate_filename(dlm_file_name);

	// init the main tasks of the DLM
	manage_logging_session_init(dlm_file_name);
    manage_data_aquisition_init(&ram_data);
    move_ram_data_to_storage_init(&ram_data, dlm_file_name);

    // in REV1 we will start the logging session right away
    begin_logging_session();

    // DEBUG testing all CAN buses
    add_custom_can_func(SET_LED_STATE, &change_led_state, TRUE, NULL);
}


static void change_led_state(U8 sender, void* parameter, U8 remote_param, U8 UNUSED1, U8 UNUSED2, U8 UNUSED3)
{
	HAL_GPIO_WritePin(GPIOB, LED1_sd_write_Pin, !!remote_param);
	return;
}


// manage_data_aquisition
//  This function will control the DAMs by sending a CAN command to send
//  the data of that bucket to the DLM. How these structs are configured
//  is controlled by a CAN command sent by the DAM
//
// Call FRQ:
//  This function will need to be called at the maximum parameter
//  request rate the DLM should support.
void manage_data_aquisition()
{
	if (logging_status != LOGGING_ACTIVE)
	{
		return;
	}

    request_all_buckets();
    store_new_data();
}



// move_ram_data_to_storage
//  This function will move all of the data stored on ram to persistent storage,
//  then wipe the RAM. The way this will be stored is still TBD, but will depend
//  heavily on what DevOps wants
//
// Call FRQ:
//  Pretty slow, prob anywhere from 1sec to 10sec. Choosing this value will
//  involve many factors including:
//   - the max amount of lost data that we are willing to take
//   - the size of the RAM buffer and how long that will take to fill up
//   - how many write cycles to the persistent storage we are ok giving up
void move_ram_data_to_storage()
{
	if (logging_status != LOGGING_ACTIVE)
	{
		return;
	}

    // TODO Use some logic to determine when the best time is to write to storage. Right
	// now it just writes every 2 seconds
	write_data_and_handle_errors();
}


// interface_with_vtm
//  This function will handle sending the appropriate data to the VTM 
//  over some interface. How much data is sent and how the data is sent
//  is still TBD depending on the on-car telemety hardware
//
// Call FRQ:
//  Completely TBD based on what the VTM is
void interface_with_vtm()
{
    // TODO
}


// begin_logging_session
//  This function will be called at the beginning of each logging session,
//  and will create a new file on the persistent storage with the correct name
//  for that run. Name will store the date and time of the beginning of the
//  logging session
//
// Call FRQ:
//  at the beginning of each logging session
void begin_logging_session()
{
	logging_status = LOGGING_ACTIVE;
}


// end_logging_session
//  This function will get all of the data off the RAM and into persistent
//  storage (call move_ram_data_to_storage()) one more time, then complete
//  and close the data storage file, and finally get the module ready to
//  begin a new logging session
//
// Call FRQ:
//  at the end of each logging session
void end_logging_session()
{
    // TODO
}


// offload_data
//  This function will take data off of the persistent storage and send it
//  to a PC in some way (possibly Ethernet). How to begin this process, how
//  to choose a file to offload, and how to actually accomplish the offloading
//  is still very TBD
//
// Call FRQ:
//  When activated, how that will happen is TBD
void offload_data()
{
    // TODO
}


// control_vehicle_systems
//  This function will do things (TBD) based on the fault states of other modules
//  as handled by the GopherCAN fault parameters for each module. Also will look at the
//  last time the Module returned a CAN request (look at last_rx of the fault parameter)
//  and possibly send a CAN command to the PDM to tell it to restart that module
//
// Call FRQ:
//  Prob 10ms-50ms, depending on how often we want fault parameters
void control_vehicle_systems()
{
    // TODO
}


// can_service_loop
//  This will perform all of the CAN servicing actions required for GopherCAN,
//  including servicing TX and RX hardware/buffers. Should be called more frequently
//  then store_data_to_ram() in order to ensure there is new data to be stored
//  if it has been received
//
// Call FRQ:
//  100us because we can
void can_service_loop()
{
	// This is needed to account for a case where the RX buffer fills up, as the ISR is only
	//  triggered directly on receiving the message
	// if debugging and want to disable interrupts, uncomment these lines
	//service_can_rx_hardware(dlm_hcan1, CAN_RX_FIFO0);
	//service_can_rx_hardware(dlm_hcan1, CAN_RX_FIFO1);
	//service_can_rx_hardware(dlm_hcan2, CAN_RX_FIFO0);
	//service_can_rx_hardware(dlm_hcan2, CAN_RX_FIFO1);
	//service_can_rx_hardware(dlm_hcan3, CAN_RX_FIFO0);
	//service_can_rx_hardware(dlm_hcan3, CAN_RX_FIFO1);

	// handle each RX message in the buffer
	if (service_can_rx_buffer())
	{
		// an error has occurred
	}

	service_can_tx_hardware(dlm_hcan1);
	service_can_tx_hardware(dlm_hcan2);
	service_can_tx_hardware(dlm_hcan3);
}


// End of dlm-high_level_functions.c
