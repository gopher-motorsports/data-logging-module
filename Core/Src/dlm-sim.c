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

DATA_INFO_NODE* ram_data_head;

void sim_init(DATA_INFO_NODE* ram_ptr) {
	ram_data_head = ram_ptr;
}

void sim_generate_data() {
    static U32 nodeCount = 0;

    // 1/100 chance of generating a node (not actually random bc no seed)
    if(rand() % 100 != 0) return;

    // create a data node
    U8_DATA_NODE* u8_data_node = malloc(sizeof(U8_DATA_NODE));
    if (u8_data_node == NULL) return;
    u8_data_node->data = 0x09;

    DATA_INFO_NODE* node = (DATA_INFO_NODE*)u8_data_node;
    node->data_time = nodeCount;
	node->param = 0x0001;

	// add the new node to the front of the list, after the head node
	while (!get_mutex_lock(&ram_data_mutex)) osDelay(1);
	taskENTER_CRITICAL();

	node->next = ram_data_head->next;
	ram_data_head->next = node;

	taskEXIT_CRITICAL();
	release_mutex(&ram_data_mutex);

	nodeCount++;
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
