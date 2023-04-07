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
#include "dlm-error_handling.h"

#define MIN_SEND_PERIOD_ms 100

static volatile U8 transmit_done = TRUE;
static U8 last_telem_tx_time[NUM_OF_PARAMETERS] = {0};

void transmit_data_init(void)
{
	// nothing to init
}

void transmit_data(PPBuff* telem_buffer, UART_HandleTypeDef* uart)
{
	// make sure the TX from the last send attempt has completed (blocking)
	while (!transmit_done) osDelay(1);

	if (osMutexWait(mutex_broadcast_bufferHandle, MUTEX_WAIT_TIME_ms) != osOK)
	{
		set_error_state(DLM_ERR_MUTEX);
		return;
	}
	// ping-pong the buffer
	uint32_t transferSize = telem_buffer->fill;
	telem_buffer->fill = 0;
	telem_buffer->write = !telem_buffer->write;
	if (osMutexRelease(mutex_broadcast_bufferHandle) != osOK)
	{
		set_error_state(DLM_ERR_MUTEX);
		return;
	}

	// start transfer
	HAL_UART_Transmit_DMA(uart, telem_buffer->buffs[!telem_buffer->write], transferSize);
	transmit_done = FALSE;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
	// tell the broadcast thread that the transfer is complete
	transmit_done = TRUE;
}

U8 should_add_to_telem_buf(CAN_INFO_STRUCT* param)
{
	if (HAL_GetTick() - last_telem_tx_time[param->ID] >= MIN_SEND_PERIOD_ms)
	{
		last_telem_tx_time[param->ID] = HAL_GetTick();
		return TRUE;
	}
	return FALSE;
}

// End of dlm-transmit_ram_data.c
