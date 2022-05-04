/*
 * dlm-sim.h
 *
 *  Created on: Apr 3, 2022
 *      Author: jonathan
 */

#ifndef INC_DLM_SIM_H_
#define INC_DLM_SIM_H_

#include "dlm-storage_structs.h"

void sim_generate_data(PPBuff* sd_buffer, PPBuff* telem_buffer);
void sim_swap_sd_buffer(PPBuff* sd_buffer);

#endif /* INC_DLM_SIM_H_ */
