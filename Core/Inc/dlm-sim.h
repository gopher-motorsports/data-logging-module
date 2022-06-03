/*
 * dlm-sim.h
 *
 *  Created on: Apr 3, 2022
 *      Author: jonathan
 */

#ifndef INC_DLM_SIM_H_
#define INC_DLM_SIM_H_

#include "dlm-storage_structs.h"

// #defines for configuring the frequency at which things are written
#define DATA_GEN_FREQ_HZ 100
#define NUM_CHANNELS 1
// TODO: something broke this
//#define FIRST_GCAN_ID (dam_chan_1.param_id)
#define FIRST_GCAN_ID 1
#define DATA_NODE_TYPE FLOAT_DATA_NODE
#define DATA_GEN_MS_BETWEEN (1000/DATA_GEN_FREQ_HZ)

void sim_generate_data(PPBuff* sd_buffer, PPBuff* telem_buffer);
void sim_swap_sd_buffer(PPBuff* sd_buffer);

#endif /* INC_DLM_SIM_H_ */
