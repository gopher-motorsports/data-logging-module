/*
 * dlm-transmit_ram_data.h
 *
 *  Created on: Mar 20, 2022
 *      Author: jonathan
 */

#ifndef INC_DLM_TRANSMIT_RAM_DATA_H_
#define INC_DLM_TRANSMIT_RAM_DATA_H_

#include "base_types.h"
#include "dlm-storage_structs.h"
#include "main.h"

void transmit_ram_data_init(DATA_INFO_NODE* ram_ptr);
void transmit_data(UART_HandleTypeDef* uart);
U8 packetize_node(DATA_INFO_NODE* node, U8 packet[]);
U8 append_byte(U8 packet[], U8 packetLength, U8 byte);

#endif /* INC_DLM_TRANSMIT_RAM_DATA_H_ */
