// dlm-high_level_functions.h
//
// Created by Calvin Molitor

#ifndef DLM_HIGH_LEVEL_FUNCTIONS_h
#define DLM_HIGH_LEVEL_FUNCTIONS_h

// Defines


// Function prototypes
void init();
void manage_data_aquisition();
void store_data_to_ram();
void move_ram_data_to_storage();
void interface_with_vtm();
void begin_logging_session();
void end_logging_session();
void offload_data();
void control_vehicle_systems();
void can_service_loop();

#endif


// End of dlm-high_level_functions.h