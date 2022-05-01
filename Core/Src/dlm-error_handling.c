// dlm-error_handling.c
//  This file houses controlling the error LED, which will show malloc errors and
//  other errors. The LED control is in a low-priority task, and errors can be set
//  by any task by calling a function in this file. Errors are reset on restart

#include "dlm-error_handling.h"


static GPIO_TypeDef* err_port = NULL;
static U16 err_pin = 0;
static DLM_ERRORS_t curr_err = DLM_ERR_NO_ERR;


// error_init
//  pass in the error LED to start blinking in case of an error
void error_init(GPIO_TypeDef* error_port, U16 error_pin)
{
	err_port = error_port;
	err_pin = error_pin;
	curr_err = DLM_ERR_NO_ERR;

	// start the LED high so we know if the RTOS starts
	HAL_GPIO_WritePin(err_port, err_pin, SET);
}


// run_led_task
//  Blink the LED in an error code if there is an issue. If there is
//  no error, the LED will be off. If an error is present, it is guaranteed
//  to flash once even if it is overwritten or cleared
void run_led_task(void)
{
	static U8 num_led_blinks = 0;
	static U32 last_blink_time = 0;

	// make sure the led port is not NULL
	if (!err_port)
	{
		return;
	}

	// this will wait until the led is done blinking an error until turning off
	if (curr_err != DLM_ERR_NO_ERR || num_led_blinks > 0)
	{
		// there is an error active
		if (!num_led_blinks)
		{
			// long delay and reset
			if (HAL_GetTick() - last_blink_time >= ERR_WAIT_TIME)
			{
				HAL_GPIO_WritePin(err_port, err_pin, RESET);
				last_blink_time = HAL_GetTick();
				num_led_blinks = (U8)curr_err << 1; // double so there is an on and off for each blink number
			}
		}
		else
		{
			if (HAL_GetTick() - last_blink_time >= ERR_BLINK_TIME)
			{
				HAL_GPIO_TogglePin(err_port, err_pin);
				last_blink_time = HAL_GetTick();
				num_led_blinks--;
			}
		}
	}
	else
	{
		// turn off the LED when the error is finished being displayed
		HAL_GPIO_WritePin(err_port, err_pin, RESET);
	}
}


// set_error_state
//  Set the current error of the task. Next blink cycle, the LED will
//  follow the code that corresponds to the current error
void set_error_state(DLM_ERRORS_t new_error)
{
	curr_err = new_error;
}


// clear_error_state
//  If the inputted error state is active, return the error to no error
void clear_error_state(DLM_ERRORS_t error)
{
	if (curr_err == error) curr_err = DLM_ERR_NO_ERR;
}


// End of dlm-error_handling.c
