/*
 * linked_list.h
 *
 *  Created on: 24. 11. 2020
 *      Author: horinek
 */

#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include "common.h"

typedef struct _ll_item_t
{
	uint32_t feature_addr;
	struct _ll_item_t * next;
	struct _ll_item_t * prev;
} ll_item_t;

void list_add_sorted_unique(uint32_t addr, ll_item_t ** list_start, ll_item_t ** list_end);
void list_free(ll_item_t * list_start, ll_item_t * list_end);
void list_dbg(ll_item_t * list_start);

#endif /* LINKED_LIST_H_ */
