/*
 * dlm-util.c
 *
 *  Created on: Apr 5, 2022
 *      Author: jonathan
 */

#include "dlm-util.h"
#include "dlm-storage_structs.h"
#include "GopherCAN.h"

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


// build_data_string
//  Convert the linked list data node in RAM to a string of data. Each
//  data point will be stored as a 16bit parameter id, 32bit time value representing
//  the ms from DLM startup, and 64bit double for the data value. This string is returned
//  in U8* data_str. This must have 14B of memory available, or else bad things will happen.
//  This function will also remove the data node from the LL
void build_data_string(U8 data_str[], DATA_INFO_NODE* data_node)
{
    DPF_CONVERTER data_union;
    U8 c;

    // write the parameter to the first 2 bytes
    for (c = 0; c < STORAGE_PARAM_SIZE; c++)
    {
        data_str[c] = (U8)(data_node->param >> (((STORAGE_PARAM_SIZE - 1) - c) * BITS_IN_BYTE));
    }

    // write the timestamp to the next 4 bytes
    for (c = 0; c < TIMESTAMP_SIZE; c++)
    {
        data_str[c + STORAGE_PARAM_SIZE] = (U8)(data_node->data_time >> (((TIMESTAMP_SIZE - 1) - c) * BITS_IN_BYTE));
    }

    // write the double of the data to the last 8 bytes
    data_union.d = convert_data_to_dpf(data_node);
    for (c = 0; c < DATA_SIZE; c++)
    {
        data_str[c + STORAGE_PARAM_SIZE + TIMESTAMP_SIZE] = (U8)(data_union.u64 >> (((DATA_SIZE - 1) - c) * BITS_IN_BYTE));
    }
}


// convert_data_to_dpf
//  Function to take in a data node, get the data stored in it, and return
//  the double precision float representation of that value to be stored on
//  the external SD card
double convert_data_to_dpf(DATA_INFO_NODE* data_node)
{
    // switch to get the data out of the data_node
    switch (parameter_data_types[data_node->param])
	{
	case UNSIGNED8:
		return (double)(((U8_DATA_NODE*)data_node)->data);

	case UNSIGNED16:
		return (double)(((U16_DATA_NODE*)data_node)->data);

	case UNSIGNED32:
		return (double)(((U32_DATA_NODE*)data_node)->data);

	case UNSIGNED64:
		return (double)(((U64_DATA_NODE*)data_node)->data);

	case SIGNED8:
		return (double)(((S8_DATA_NODE*)data_node)->data);

	case SIGNED16:
		return (double)(((S16_DATA_NODE*)data_node)->data);

	case SIGNED32:
		return (double)(((S32_DATA_NODE*)data_node)->data);

	case SIGNED64:
		return (double)(((S64_DATA_NODE*)data_node)->data);

	case FLOATING:
		return (double)(((FLOAT_DATA_NODE*)data_node)->data);

	default:
        // Something went wrong, just write 0 to data
		return 0;
	}

    // this coude should not be reached, this is to make the compiler happy
    return 0;
}
