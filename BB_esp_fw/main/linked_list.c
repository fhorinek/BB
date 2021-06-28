#include "linked_list.h"

ll_item_t * first = NULL;

ll_item_t * ll_get_last()
{
	ll_item_t * item = first;
	if (item == NULL)
		return NULL;

	while (item->next != NULL)
	{
		item = item->next;
	}

	return item;
}

void ll_add_item(uint8_t msg_type, uint8_t msg_id, uint16_t data_size)
{
	ll_item_t * item = ps_malloc(sizeof(ll_item_t));

	if (data_size > 0)
		item->data_ptr = ps_malloc(data_size);
	else
		item->data_ptr = NULL;

	item->msg_id = msg_id;
	item->msg_type = msg_type;
}


void ll_delete_item(ll_item_t * item);
ll_item_t * ll_find_item(uint8_t msg_type, uint8_t msg_id);
