/*
 * dlm-sim.h
 *
 *  Created on: Apr 3, 2022
 *      Author: jonathan
 */

#ifndef INC_DLM_SIM_H_
#define INC_DLM_SIM_H_

#include "dlm-storage_structs.h"

void sim_init(DATA_INFO_NODE* ram_ptr);
void sim_generate_data();
void sim_clear_ram();

#endif /* INC_DLM_SIM_H_ */
