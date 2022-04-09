/*
 * dlm-transmit_ram_data.c
 *
 *  Created on: Mar 20, 2022
 *      Author: jonathan
 */

#include "cmsis_os.h"
#include "main.h"
#include "dlm-transmit_ram_data.h"
#include "dlm-move_ram_data_to_storage.h"
#include "dlm-storage_structs.h"
#include "dlm-mutex.h"
#include "dlm-util.h"
#include "GopherCAN.h"

DATA_INFO_NODE* ram_data_head;

void transmit_ram_data_init(DATA_INFO_NODE* ram_ptr) {
	ram_data_head = ram_ptr;
}

void transmit_data(UART_HandleTypeDef* uart) {
	static DATA_INFO_NODE* prevTransmission = NULL;
	U8 packet[32];
	DATA_INFO_NODE* node = ram_data_head->next;
	DATA_INFO_NODE* transmissionHead = node;

	// step through nodes from the buffer head to the start of the last transmission
	while (node != NULL && node != prevTransmission) {
		// wait for access to the data buffer
		while (!get_mutex_lock(&ram_data_mutex)) {
			osDelay(1);
		}
		taskENTER_CRITICAL();

		// build a packet from the data node
		U8 packetLength = packetize_node(node, packet);
		node = node->next;

		taskEXIT_CRITICAL();
		release_mutex(&ram_data_mutex);

		// if the xbee buffer has space, send packet
		U8 xbFull = HAL_GPIO_ReadPin(XB_NCTS_GPIO_Port, XB_NCTS_Pin);
		if (!xbFull) {
			HAL_UART_Transmit(uart, packet, packetLength, HAL_MAX_DELAY);
		}
	}

	// remember the first node in this transmission
	prevTransmission = transmissionHead;
}
