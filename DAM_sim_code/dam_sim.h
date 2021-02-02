// dam_sim.h
//  Header file for dam_sim.c

#ifndef DAM_SIM_H
#define DAM_SIM_H

// includes
#include "GopherCAN.h"
#include "base_types.h"


// Function prototypes
void dam_sim_init(CAN_HandleTypeDef* hcan_ptr);
void main_loop();
void service_can_hardware();
void send_bucket_params(MODULE_ID sender, void* parameter,
    U8 UNUSED0, U8 UNUSED1, U8 UNUSED2, U8 UNUSED3);
void bucket_requested(MODULE_ID sender, void* parameter,
    U8 bucket_id, U8 UNUSED1, U8 UNUSED2, U8 UNUSED3);


// defines
#define BUCKET_0 0
#define BUCKET_1 1
#define BUCKET_2 2

#define BUCKET_0_RQ_FRQ 10
#define BUCKET_1_RQ_FRQ 50
#define BUCKET_2_RQ_FRQ 100

#endif


// End of dam_sim.h
