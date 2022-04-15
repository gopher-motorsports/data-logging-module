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
#include "dlm-mutex.h"
#include "GopherCAN.h"

DATA_INFO_NODE* ram_data_head;

void sim_init(DATA_INFO_NODE* ram_ptr) {
	ram_data_head = ram_ptr;
}


// #defines for configuring the frequency at which things are written
#define DATA_GEN_FREQ_HZ 100
#define NUM_CHANNELS 24
#define FIRST_GCAN_ID (dam_chan_1.param_id)
#define DATA_NODE_TYPE FLOAT_DATA_NODE

#define DATA_GEN_MS_BETWEEN (1000/DATA_GEN_FREQ_HZ)


// this function is called every 1ms, so keep that in mind when doing timings
void sim_generate_data()
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
			// create a data node.
			DATA_NODE_TYPE* data_node = malloc(sizeof(DATA_NODE_TYPE));
			if (data_node == NULL)
			{
				// ran out of memory
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
				return;
			}
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
			data_node->data = nodeCount; // GCC should handle the implicit cast to whatever data type is required

			DATA_INFO_NODE* node = (DATA_INFO_NODE*)data_node;
			node->data_time = time;
			node->param = pid;

			// add the new node to the front of the list, after the head node
			while (!get_mutex_lock(&ram_data_mutex)) osDelay(1);
			taskENTER_CRITICAL();

			node->next = ram_data_head->next;
			ram_data_head->next = node;

			taskEXIT_CRITICAL();
			release_mutex(&ram_data_mutex);
		}

		nodeCount++;
    }
}

void sim_clear_ram() {
	DATA_INFO_NODE* node = ram_data_head->next;

	// clear all nodes from the buffer
	while (node != NULL) {
		// get the mutex. It may not be needed for any nodes but the first data node after the head
		while (!get_mutex_lock(&ram_data_mutex)) osDelay(1);
		taskENTER_CRITICAL();

		// remove the pointer from the LL
		ram_data_head->next = node->next;

		// free the memory for this node
		free(node);

		// move on to the next data node
		node = ram_data_head->next;

		taskEXIT_CRITICAL();
		release_mutex(&ram_data_mutex);
	}
}
