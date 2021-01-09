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


// Global Variables
BUCKET_NODE* bucket_list;


// init
//  This function will handle power-on behavior, all completely TBD
//  according to everthing else the module does
void init()
{
    // TODO

    manage_data_aquisition_init();
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
//   - how many write cycles to the persistant storage we are ok giving up
void move_ram_data_to_storage()
{
    // TODO check heap usage, when it reaches a certain level, start flushing the RAM
    //  buffer to persistant storage
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
    // TODO
}


// End of dlm-high_level_functions.c