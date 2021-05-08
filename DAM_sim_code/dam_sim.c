// dam_sim.c
//  This is a .c file with functions to be called by the RTOS and CAN commands. It is designed
//  to act like a DAM and interface with the DLM for testing.

#include "dam_sim.h"
#include "GopherCAN.h"


U8 last_button_state;
U16 led_to_change;

// HAL CAN struct
CAN_HandleTypeDef* example_hcan;

U8 this_module = DAM_ID;

void change_led_state(U8 sender, void* parameter, U8 remote_param, U8 UNUSED1, U8 UNUSED2, U8 UNUSED3);

// dam_sim_init
//  init GopherCAN and the tester variables
void dam_sim_init(CAN_HandleTypeDef* hcan_ptr)
{
    example_hcan = hcan_ptr;

    // init gopherCAN stuff
    if (init_can(example_hcan, this_module, MASTER))
	{
		// an error has occurred, stay here
		while (1);
	}

    // enable the tester variables
	set_all_params_state(TRUE);

    // add all of the CAN commands needed
    add_custom_can_func(SEND_BUCKET_PARAMS, &send_bucket_params, TRUE, NULL);
    add_custom_can_func(BUCKET_OK, &bucket_ok, TRUE, NULL);
    add_custom_can_func(REQUEST_BUCKET, &bucket_requested, TRUE, NULL);

    led_to_change = GPIO_PIN_5; // this is LD1 on the dev board

    if (add_custom_can_func(SET_LED_STATE, &change_led_state,
        TRUE, (void*)&led_to_change))
    {
        // an error has occurred
    }
}


// main_loop
//  Loop to simulate getting new data by incrementing the different tester variables
void main_loop()
{
	U8 button_state;

    // Modify each of the tester variables
    u8_tester.data += 1;
    u16_tester.data += 2;
	u32_tester.data += 4;
    u64_tester.data += 8;
	s8_tester.data -= 1;
    s16_tester.data -= 2;
	s32_tester.data -= 4;
    s64_tester.data -= 8;
	float_tester.data += 0.1;

	// If the button is pressed send a can command to another to change the LED state
	// To on or off depending on the button. Make sure to disable any heartbeat before
	// trying this, they may conflict
	button_state = !HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

	// this is to prevent spamming CAN messages
	if (button_state != last_button_state)
	{
		last_button_state = button_state;

		if (send_can_command(PRIO_HIGH, ALL_MODULES_ID, SET_LED_STATE,
				button_state, button_state, button_state, button_state))
		{
			// error sending command
		}
	}
}


// service_can_hardware
//  Function to service the CAN hardware and the RX handling
void service_can_hardware()
{
    service_can_rx_buffer();
	service_can_tx_hardware(example_hcan);
}


// send_bucket_params
//  CAN command to send commands to the DLM to add parameters to buckets, and then assign
//  frequencies to that bucket
void send_bucket_params(MODULE_ID sender, void* parameter,
    U8 UNUSED0, U8 UNUSED1, U8 UNUSED2, U8 UNUSED3)
{
	static U8 counter = 0;

    // set the number of parameters in each bucket
    send_can_command(PRIO_HIGH, DLM_ID, SET_BUCKET_SIZE, BUCKET_0, BUCKET_0_SIZE, 0, 0);
    send_can_command(PRIO_HIGH, DLM_ID, SET_BUCKET_SIZE, BUCKET_1, BUCKET_1_SIZE, 0, 0);

    // test not sending the size first to the DLM
    if (counter)
    {
    	send_can_command(PRIO_HIGH, DLM_ID, SET_BUCKET_SIZE, BUCKET_2, BUCKET_2_SIZE, 0, 0);
    }
    counter++;

    // Send a command for each parameter to tell what bucket it is in
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, U8_TESTER_ID, BUCKET_0, 0);

    // test sending the same parameter multiple times
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, U16_TESTER_ID, BUCKET_0, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, U16_TESTER_ID, BUCKET_0, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, U16_TESTER_ID, BUCKET_0, 0);

    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, U32_TESTER_ID, BUCKET_0, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, U64_TESTER_ID, BUCKET_1, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, S8_TESTER_ID, BUCKET_1, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, S16_TESTER_ID, BUCKET_1, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, S32_TESTER_ID, BUCKET_2, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, S64_TESTER_ID, BUCKET_2, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, FLOAT_TESTER_ID, BUCKET_2, 0);
}


// bucket_ok
//  this CAN command is received when a bucket is completely full. At this point
//  the data can be safely requested
void bucket_ok(MODULE_ID sender, void* parameter,
    U8 bucket_id, U8 UNUSED1, U8 UNUSED2, U8 UNUSED3)
{
    // Assign a frequency to the bucket that was givin the ok
    switch (bucket_id)
    {
    case BUCKET_0:
        send_can_command(PRIO_HIGH, DLM_ID, ASSIGN_BUCKET_TO_FRQ,
                BUCKET_0, 0, BUCKET_0_RQ_FRQ, 0);
        break;

    case BUCKET_1:
        send_can_command(PRIO_HIGH, DLM_ID, ASSIGN_BUCKET_TO_FRQ,
                BUCKET_1, 0, BUCKET_1_RQ_FRQ, 0);
        break;

    case BUCKET_2:
        send_can_command(PRIO_HIGH, DLM_ID, ASSIGN_BUCKET_TO_FRQ,
                BUCKET_2, 0, BUCKET_2_RQ_FRQ, 0);
        break;
    }
}


// bucket_requested
//  CAN command to send each parameter in the bucket specified by the remote parameter
void bucket_requested(MODULE_ID sender, void* parameter,
    U8 bucket_id, U8 UNUSED1, U8 UNUSED2, U8 UNUSED3)
{
    // Send each parameter in the bucket
    switch (bucket_id)
    {
    case BUCKET_0:
        send_parameter(PRIO_HIGH, DLM_ID, U8_TESTER_ID);
        send_parameter(PRIO_HIGH, DLM_ID, U16_TESTER_ID);
        send_parameter(PRIO_HIGH, DLM_ID, U32_TESTER_ID);
        break;

    case BUCKET_1:
        send_parameter(PRIO_HIGH, DLM_ID, U64_TESTER_ID);
        send_parameter(PRIO_HIGH, DLM_ID, S8_TESTER_ID);
        send_parameter(PRIO_HIGH, DLM_ID, S16_TESTER_ID);
        break;

    case BUCKET_2:
        send_parameter(PRIO_HIGH, DLM_ID, S32_TESTER_ID);
        send_parameter(PRIO_HIGH, DLM_ID, S64_TESTER_ID);
        send_parameter(PRIO_HIGH, DLM_ID, FLOAT_TESTER_ID);
        break;
    
    default:
        // something went wrong
        break;
    }
}


// change_led_state
//  a custom function that will change the state of the LED specified
//  by parameter to remote_param. In this case parameter is a U16*, but
//  any data type can be pointed to, as long as it is configured and casted
//  correctly
void change_led_state(U8 sender, void* parameter, U8 remote_param, U8 UNUSED1, U8 UNUSED2, U8 UNUSED3)
{
	// this function will set the LED to high or low, depending on remote_param
	// the LED to change is dependent on the parameter stored on this module (*((U16*)parameter))
	U16 gpio_pin = *((U16*)parameter);

	HAL_GPIO_WritePin(GPIOA, gpio_pin, !!remote_param);

	return;
}


// End of dam_sim.c
