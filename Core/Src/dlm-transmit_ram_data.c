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
#include "GopherCAN.h"

void transmit_data_init(void)
{
	// clear transfer flag initially
	osThreadFlagsSet(transmit_ram_Handle, FLAG_TRANSFER_DONE);
}

void transmit_data(PPBuff* telem_buffer, UART_HandleTypeDef* uart)
{
	if (osMutexAcquire(mutex_broadcast_bufferHandle, MUTEX_WAIT_TIME_ms) != osOK)
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
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
	// tell the broadcast thread that the transfer is complete
	osThreadFlagsSet(transmit_ram_Handle, FLAG_TRANSFER_DONE);
}


// End of dlm-transmit_ram_data.c
