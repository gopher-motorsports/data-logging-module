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
//  is to help with overall code orginization


// self include
#include "dlm-high_level_functions.h"
#include "base_types.h"
#include "GopherCAN.h"
#include "usb_host.h"
#include "dlm-storage_structs.h"
#include "dlm-manage_data_aquisition.h"
#include "dlm-move_ram_data_to_storage.h"


// Global Variables
DATA_INFO_NODE ram_data = {0, 0, NULL};

// the HAL_CAN structs
CAN_HandleTypeDef* dlm_hcan0;
CAN_HandleTypeDef* dlm_hcan1;

// get the tester variables
extern U8_CAN_STRUCT u8_tester;
extern U16_CAN_STRUCT u16_tester;
extern U32_CAN_STRUCT u32_tester;
extern U64_CAN_STRUCT u64_tester;
extern S8_CAN_STRUCT s8_tester;
extern S16_CAN_STRUCT s16_tester;
extern S32_CAN_STRUCT s32_tester;
extern S64_CAN_STRUCT s64_tester;
extern FLOAT_CAN_STRUCT float_tester;

U16 counter = 0;

const char dlm_file_name[6] = "/TEST.txt";


// dlm_init
//  This function will handle power-on behavior, all completely TBD
//  according to everthing else the module does
void dlm_init(CAN_HandleTypeDef* hcan_ptr0, CAN_HandleTypeDef* hcan_ptr1)
{
    // init GopherCAN
	dlm_hcan0 = hcan_ptr0;
	dlm_hcan1 = hcan_ptr1;

	// initialize CAN
	// NOTE: CAN will also need to be added in CubeMX and code must be generated
	// Check the STM_CAN repo for the file "F0xx CAN Config Settings.pptx" for the correct settings
	if (init_can(dlm_hcan0, DLM_ID)
			|| init_can(dlm_hcan1, DLM_ID))
	{
		// an error has occurred, stay here
		while (1);
	}

	// Declare which bus is which using define_can_bus
	define_can_bus(dlm_hcan1, GCAN0, 0);
	define_can_bus(dlm_hcan0, GCAN1, 1);

	// enable the tester variables
	u8_tester.update_enabled = TRUE;
	u16_tester.update_enabled = TRUE;
	u32_tester.update_enabled = TRUE;
	u64_tester.update_enabled = TRUE;
	s8_tester.update_enabled = TRUE;
	s16_tester.update_enabled = TRUE;
	s32_tester.update_enabled = TRUE;
	s64_tester.update_enabled = TRUE;
	float_tester.update_enabled = TRUE;

    manage_data_aquisition_init(&ram_data);
    move_ram_data_to_storage_init(&ram_data);
}


// manage_data_aquisition
//  This function will control the DAMs by sending a CAN command to send
//  the data of that bucket to the DLM. How these structs are configured
//  is controled by a CAN command sent by the DAM
//
// Call FRQ:
//  This function will need to be called at the maximum parameter
//  request rate the DLM should support.
void manage_data_aquisition()
{
    request_all_buckets();
    store_new_data();
}



// move_ram_data_to_storage
//  This function will move all of the data stored on ram to persistant storage,
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
    // TODO Use some logic to determine when the best time is to write to storage. Right
	// now it just writes every second
	if (counter == 1000)
	{
		write_data_to_storage(dlm_file_name);
		counter = 0;
	}
	else
	{
		counter++;
	}
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
//  This function will be called at the beginning of each logging sesstion,
//  and will create a new file on the persistant storage will the correct name
//  and metadata for that run. What the name is and what metadata needs to be
//  stored is TBD and up to DevOps
//
// Call FRQ:
//  at the beginning of each logging session
void begin_logging_session()
{
    // TODO
}


// end_logging_session
//  This function will get all of the data off the RAM and into persistant
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
//  This function will take data off of the persistant storage and send it
//  to a PC in some way (possibly ethernet). How to begin this process, how
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
//  if it has been recieved
//
// Call FRQ:
//  100us because we can
void can_service_loop()
{
	// This is needed to account for a case where the RX buffer fills up, as the ISR is only
	//  triggered directly on reciving the message
	// TODO enable interrupts when not debugging
	//service_can_rx_hardware(dlm_hcan0, CAN_RX_FIFO0);
	//service_can_rx_hardware(dlm_hcan0, CAN_RX_FIFO1);
	//service_can_rx_hardware(dlm_hcan1, CAN_RX_FIFO0);
	//service_can_rx_hardware(dlm_hcan1, CAN_RX_FIFO1);

	// handle each RX message in the buffer
	if (service_can_rx_buffer())
	{
		// an error has occurred
	}

	service_can_tx_hardware(dlm_hcan0);
	service_can_tx_hardware(dlm_hcan1);
}


// End of dlm-high_level_functions.c
