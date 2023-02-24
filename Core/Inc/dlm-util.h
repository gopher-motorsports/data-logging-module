/*
 * dlm-util.h
 *
 *  Created on: May 2, 2022
 *      Author: jonathan
 */

#ifndef INC_DLM_UTIL_H_
#define INC_DLM_UTIL_H_

#include "base_types.h"
#include "dlm-error_handling.h"
#include "dlm-storage_structs.h"

// byte to indicate the start of a packet
#define START_BYTE 0x7e
// byte to indicate that the next byte has been escaped
#define ESCAPE_BYTE 0x7d
#define ESCAPE_XOR 0x20

#define MUTEX_GET_TIMEOUT_ms 500
#define DLM_SUCCESS 0

DLM_ERRORS_t append_packet(PPBuff* buffer, U32 bufferSize, U32 timestamp, U16 id, void* data, U8 dataSize);

#endif /* INC_DLM_UTIL_H_ */
