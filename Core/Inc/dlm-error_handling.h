// dlm-error_handling.h
//  Header file for dlm-error_handling.c

#ifndef DLM_ERROR_HANDLING_H
#define DLM_ERROR_HANDLING_H

#include "base_types.h"
#include "stm32f7xx_hal.h"

#define NO_ERR_BLINK_TIME 500
#define ERR_BLINK_TIME 200
#define ERR_WAIT_TIME 800
#define LED_DELAY_TIME 100

// errors enum
typedef enum
{
	DLM_ERR_NO_ERR = 0,
	DLM_ERR_RAM_FAIL = 1,
	DLM_ERR_NO_DATA = 2,
	DLM_ERR_CAN_ERR = 3,
	DLM_ERR_SD_MOUNT = 4,
	DLM_ERR_FILE_WRITE = 5,
	DLM_ERR_INIT_FAIL = 6,
	DLM_ERR_DATATYPE = 7,
	DLM_ERR_FILE_CREATE = 8,
	DLM_ERR_MALLOC_ERR = 9,
	DLM_ERR_MUTEX = 10
} DLM_ERRORS_t;


void error_init(GPIO_TypeDef* error_port, U16 error_pin);
void run_led_task(void);
void set_error_state(DLM_ERRORS_t new_error);
void clear_error_state(DLM_ERRORS_t error);


#endif // DLM_ERROR_HANDLING_H


// End of dlm-error_handling.h
