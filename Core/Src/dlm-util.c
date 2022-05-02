/*
 * dlm-util.c
 *
 *  Created on: May 2, 2022
 *      Author: jonathan
 */

#include "dlm-util.h"

DLM_ERRORS_t append_packet(PPBuff* buffer, U32 bufferSize, U32 timestamp,
								  U16 id, void* data, U8 dataSize)
{
	// calculate the packet size and available buffer space. We will assume each each
	// character is an escape byte to make sure we never write to far
	U8 packetSize = (1 + sizeof(timestamp) + sizeof(id) + dataSize) * 2;
	U32 freeSpace = bufferSize - buffer->fill;
	if (packetSize > freeSpace)
	{
		// this packet won't fit
		return DLM_ERR_RAM_FAIL;
	}

	// find the write buffer based on the buffer not being read from
	U8* buff = buffer->buffs[buffer->write];
    U8 i;
    U8* bytes;

    // insert start byte
    buff[buffer->fill++] = START_BYTE;

    // append components with MSB first
    bytes = (U8*) &(timestamp);
    for (i = sizeof(timestamp); i > 0; i--)
    {
        append_byte(buffer, bytes[i - 1]);
    }

    bytes = (U8*) &(id);
    for (i = sizeof(id); i > 0; i--)
    {
		append_byte(buffer, bytes[i - 1]);
	}

    bytes = (U8*) data;
    for (i = dataSize; i > 0; i--)
    {
		append_byte(buffer, bytes[i - 1]);
	}

    // success
    return DLM_ERR_NO_ERR;
}

void append_byte(PPBuff* buffer, U8 byte)
{
	// find the write buffer
	U8* buff = buffer->buffs[buffer->write];

    // check for a control byte
    if (byte == START_BYTE || byte == ESCAPE_BYTE)
    {
        // append an escape byte
    	buff[buffer->fill++] = ESCAPE_BYTE;
        // append the desired byte, escaped
    	buff[buffer->fill++] = byte ^ ESCAPE_XOR;
    }
    else
    {
    	// append the raw byte
    	buff[buffer->fill++] = byte;
    }
}
