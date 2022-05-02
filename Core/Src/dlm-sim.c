/*
 * dlm-sim.c
 *
 *  Created on: Apr 3, 2022
 *      Author: jonathan
 */

// functions for running the DLM in simulation mode

#include <stdlib.h>
#include "cmsis_os.h"
#include "base_types.h"
#include "dlm-storage_structs.h"
#include "GopherCAN.h"

// #defines for configuring the frequency at which things are written
#define DATA_GEN_FREQ_HZ 100
#define NUM_CHANNELS 24
#define FIRST_GCAN_ID (dam_chan_1.param_id)
#define DATA_NODE_TYPE FLOAT_DATA_NODE

#define DATA_GEN_MS_BETWEEN (1000/DATA_GEN_FREQ_HZ)


// this function is called every 1ms, so keep that in mind when doing timings
void sim_generate_data(void)
{
    static U32 nodeCount = 0;
    static U32 time_counter = 0;
    U32 time = HAL_GetTick();

    // check if it the right time to add data to the buffer
    if (++time_counter >= DATA_GEN_MS_BETWEEN)
    {
    	time_counter = 0;

		// create data for each of the parameters required
		for (U16 pid = FIRST_GCAN_ID; pid < FIRST_GCAN_ID + NUM_CHANNELS; pid++)
		{
			time++; // removes a compiler warning
			// TODO fix for new storage standard
		}

		nodeCount++;
    }
}

void sim_clear_ram(void)
{
	// TODO fix for new storage standard
}
