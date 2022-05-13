/*
 * dlm-util.c
 *
 *  Created on: May 2, 2022
 *      Author: jonathan
 */

#include "dlm-util.h"
#include <string.h>

DLM_ERRORS_t append_packet(PPBuff* buffer, U32 buffer_size, U32 timestamp, U16 id, void* data, U8 data_size) {
	U32 free_space = buffer_size - buffer->fill;
	if (MAX_PACKET_SIZE > free_space) {
		// this packet might not fit
		return DLM_ERR_RAM_FAIL;
	}

	U8 packet[MAX_PACKET_SIZE] = {0};
	U8 packet_fill = 0;

	packet[packet_fill++] = START_BYTE;
	packet_fill = append_value(packet, packet_fill, &timestamp, sizeof(timestamp));
	packet_fill = append_value(packet, packet_fill, &id, sizeof(id));
	packet_fill = append_value(packet, packet_fill, data, data_size);

	// math trick to get the minimum packet size that's a multiple of 8
	U8 packet_size = ((packet_fill - 1) | 7) + 1;

	// copy this packet to the write buffer
	memcpy(&buffer->buffs[buffer->write][buffer->fill], packet, packet_size);
	buffer->fill += packet_size;

	// success
	return DLM_ERR_NO_ERR;
}

U8 append_value(U8 packet[], U8 fill, void* value, U8 size) {
	// append the value byte-by-byte (big endian)
	U8 i;
	U8* bytes = (U8*) value;
	for (i = size; i > 0; i--) {
		U8 byte = bytes[i - 1];
		// check for a control byte
		if (byte == START_BYTE || byte == ESCAPE_BYTE) {
			// append an escape byte
			packet[fill++] = ESCAPE_BYTE;
			// append the desired byte, escaped
			packet[fill++] = byte ^ 0x20;
		} else {
			// append the raw byte
			packet[fill++] = byte;
		}
	}

	return fill;
}
