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
#include "GopherCAN.h"

#define FLAG_TRANSFER_DONE 0x00000001U
#define MUTEX_WAIT_TIME_ms 500

void transmit_data_init(void);
void transmit_data(PPBuff* telem_buffer, UART_HandleTypeDef* uart);
U8 should_add_to_telem_buf(CAN_INFO_STRUCT* param);

#endif /* INC_DLM_TRANSMIT_RAM_DATA_H_ */
