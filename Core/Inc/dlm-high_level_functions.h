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

// size in bytes for half of the ping-pong buffer
#define STORAGE_BUFFER_SIZE 128000
#define BROADCAST_BUFFER_SIZE 64000

// Function prototypes
void dlm_init(CAN_HandleTypeDef* hcan_ptr1, CAN_HandleTypeDef* hcan_ptr2,
		CAN_HandleTypeDef* hcan_ptr3, UART_HandleTypeDef* huart, GPIO_TypeDef* error_port,
		U16 error_pin, GPIO_TypeDef* sd_write_port, U16 sd_write_pin);
void manage_data_aquisition(void);
void move_ram_data_to_storage(void);
void handle_error_led(void);
void transmit_ram_data(void);
void begin_logging_session(void);
void end_logging_session(void);
void offload_data(void);
void control_vehicle_systems(void);
void can_service_loop(void);


// Enums
typedef enum
{
	NOT_LOGGING = 0,
	LOGGING_ACTIVE = 1
} LOGGING_STATUS;


#endif


// End of dlm-high_level_functions.h
