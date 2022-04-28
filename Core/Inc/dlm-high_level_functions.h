// dlm-high_level_functions.h
//
// Created by Calvin Molitor

#ifndef DLM_HIGH_LEVEL_FUNCTIONS_H
#define DLM_HIGH_LEVEL_FUNCTIONS_H

#include "base_types.h"
#include "stm32f7xx_hal.h"

// Defines
#define TESTER_FILE_NAME "/dlm_data.gdat"

// uncomment for sim mode
//#define SIMULATE_DATA_COLLECTION
//#define AUTO_CLEAR_DATA


// Function prototypes
void dlm_init(CAN_HandleTypeDef* hcan_ptr1, CAN_HandleTypeDef* hcan_ptr2,
		CAN_HandleTypeDef* hcan_ptr3, GPIO_TypeDef* error_port, U16 error_pin,
		GPIO_TypeDef* sd_write_pin, U16 sd_write_port);
void manage_data_aquisition();
void move_ram_data_to_storage();
void handle_error_led(void);
void transmit_ram_data();
void begin_logging_session();
void end_logging_session();
void offload_data();
void control_vehicle_systems();
void can_service_loop();


// Enums
typedef enum
{
	NOT_LOGGING = 0,
	LOGGING_ACTIVE = 1
} LOGGING_STATUS;


#endif


// End of dlm-high_level_functions.h
