/*
 * dlm-transmit_ram_data.c
 *
 *  Created on: Mar 20, 2022
 *      Author: jonathan
 */

#include "cmsis_os.h"

#include "dlm-transmit_ram_data.h"
#include "dlm-move_ram_data_to_storage.h"
#include "dlm-storage_structs.h"
#include "dlm-mutex.h"
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
//		U8 xbFull = HAL_GPIO_ReadPin(NCTS_GPIO_Port, NCTS_Pin);
		U8 xbFull = 0;
		if (!xbFull) {
			HAL_UART_Transmit(uart, packet, packetLength, HAL_MAX_DELAY);
		}
	}

	// remember the first node in this transmission
	prevTransmission = transmissionHead;
}

// takes a data node, breaks it into bytes, and writes it to a packet (byte array)
// returns the length of the packet
U8 packetize_node(DATA_INFO_NODE* node, U8 packet[]) {
    U8 i;
    U8* bytes;
    U8 packetLength = 0;

    packet[packetLength] = (U8) 0x7e;
    packetLength++;

    for (i = sizeof(node->data_time); i > 0; i--) {
        bytes = (U8*) &(node->data_time);
        packetLength = append_byte(packet, packetLength, bytes[i - 1]);
    }

    for (i = sizeof(node->param); i > 0; i--) {
        bytes = (U8*) &(node->param);
        packetLength = append_byte(packet, packetLength, bytes[i - 1]);
    }

    // write the double of the data to the last 8 bytes
    DPF_CONVERTER data_union;
	data_union.d = convert_data_to_dpf(node);
    for (i = sizeof(data_union.u64); i > 0; i--) {
        bytes = (U8*) &(data_union.u64);
        packetLength = append_byte(packet, packetLength, bytes[i - 1]);
    }

    return packetLength;
}

// escapes a byte if necessary, then appends it to the packet
// takes the current length of the packet and the byte to append
// returns the new packet length
U8 append_byte(U8 packet[], U8 packetLength, U8 byte) {
    U8 bytesFilled = 0;

    // check for a control byte
    if (byte == 0x7e || byte == 0x7d) {
        // append an escape byte
        packet[packetLength + bytesFilled] = 0x7d;
        bytesFilled++;
        // append escaped byte
        packet[packetLength + bytesFilled] = byte ^ 0x20;
        bytesFilled++;
    } else {
        packet[packetLength + bytesFilled] = byte;
        bytesFilled++;
    }

    return packetLength + bytesFilled;
}
