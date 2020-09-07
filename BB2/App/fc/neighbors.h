/*
 * neighbors.h
 *
 *  Created on: May 28, 2020
 *      Author: horinek
 */

#ifndef FC_NEIGHBORS_H_
#define FC_NEIGHBORS_H_

#include "../common.h"
#include "fc.h"

void neighbors_init();
void neighbors_update(neighbor_t new_data);
void neighbors_update_name(fanet_addr_t addr, char * name);
void neighbors_step();

#endif /* FC_NEIGHBORS_H_ */
