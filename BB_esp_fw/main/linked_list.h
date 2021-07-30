#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

#include "common.h"

typedef struct _ll_item_t
{
	struct _ll_item_t * next;

	SemaphoreHandle_t write;
	SemaphoreHandle_t read;
	void * data_ptr;

	uint8_t msg_type;
	uint8_t msg_id;

} ll_item_t;

void ll_init();
ll_item_t * ll_add_item(uint8_t msg_type, uint8_t msg_id, uint16_t data_size);
void ll_delete_item(ll_item_t * item);
ll_item_t * ll_find_item(uint8_t msg_type, uint8_t msg_id);

#endif /* LINKED_LIST_H_ */
