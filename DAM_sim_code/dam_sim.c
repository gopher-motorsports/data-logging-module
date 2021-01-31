// dam_sim.c
//  This is a .c file with functions to be called by the RTOS and CAN commands. It is designed
//  to act like a DAM and interface with the DLM for testing.

#include "dam_sim.h"
#include "GopherCAN.h"


// tester variables
extern U8_CAN_STRUCT u8_tester;
extern U16_CAN_STRUCT u16_tester;
extern U32_CAN_STRUCT u32_tester;
extern U64_CAN_STRUCT u64_tester;
extern S8_CAN_STRUCT s8_tester;
extern S16_CAN_STRUCT s16_tester;
extern S32_CAN_STRUCT s32_tester;
extern S64_CAN_STRUCT s64_tester;
extern FLOAT_CAN_STRUCT float_tester;

// HAL CAN struct
CAN_HandleTypeDef* example_hcan;

U8 this_module = DAM_ID;

// dam_sim_init
//  init GopherCAN and the tester variables
void dam_sim_init(CAN_HandleTypeDef* hcan_ptr)
{
    example_hcan = hcan_ptr;

    // init gopherCAN stuff
    if (init_can(example_hcan, this_module))
	{
		// an error has occurred, stay here
		while (1);
	}

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

    // add all of the CAN commands needed
    add_custom_can_func(SEND_BUCKET_PARAMS, &send_bucket_params, TRUE, NULL);
    add_custom_can_func(REQUEST_BUCKET, &bucket_requested, TRUE, NULL);
}


// main_loop
//  Loop to simulate getting new data by incrementing the different tester variables
void main_loop()
{
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
    // Send a command for each parameter to tell what bucket it is in
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, U8_TESTER_ID, BUCKET_0, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, U16_TESTER_ID, BUCKET_0, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, U32_TESTER_ID, BUCKET_0, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, U64_TESTER_ID, BUCKET_1, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, S8_TESTER_ID, BUCKET_1, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, S16_TESTER_ID, BUCKET_1, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, S32_TESTER_ID, BUCKET_2, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, S64_TESTER_ID, BUCKET_2, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ADD_PARAM_TO_BUCKET, 0, FLOAT_TESTER_ID, BUCKET_2, 0);

    // Assign a frequency to each bucket
    send_can_command(PRIO_HIGH, DLM_ID, ASSIGN_BUCKET_TO_FRQ, BUCKET_0, 0, BUCKET_0_RQ_FRQ, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ASSIGN_BUCKET_TO_FRQ, BUCKET_1, 0, BUCKET_1_RQ_FRQ, 0);
    send_can_command(PRIO_HIGH, DLM_ID, ASSIGN_BUCKET_TO_FRQ, BUCKET_2, 0, BUCKET_2_RQ_FRQ, 0);
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


// End of dam_sim.c
