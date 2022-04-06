/*
 * dlm-util.h
 *
 *  Created on: Apr 5, 2022
 *      Author: jonathan
 */

#ifndef INC_DLM_UTIL_H_
#define INC_DLM_UTIL_H_

#include "base_types.h"
#include "dlm-storage_structs.h"

U8 packetize_node(DATA_INFO_NODE* node, U8 packet[]);
U8 append_byte(U8 packet[], U8 packetLength, U8 byte);
double convert_data_to_dpf(DATA_INFO_NODE* data_node);

#endif /* INC_DLM_UTIL_H_ */
