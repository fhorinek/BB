#include "linked_list.h"

static ll_item_t * first = NULL;
SemaphoreHandle_t ll_semaphore = NULL;

void ll_init()
{
	ll_semaphore = xSemaphoreCreateBinary();
	xSemaphoreGive(ll_semaphore);
}

ll_item_t * ll_add_item(uint8_t msg_type, uint8_t msg_id, uint16_t data_size)
{
	ll_item_t * item = ps_malloc(sizeof(ll_item_t));

	if (data_size > 0)
		item->data_ptr = ps_malloc(data_size);
	else
		item->data_ptr = NULL;

	item->next = NULL;
	item->msg_id = msg_id;
	item->msg_type = msg_type;
	item->write = xSemaphoreCreateBinary();
	item->read = xSemaphoreCreateBinary();

	xSemaphoreGive(item->write);

	//add to list
	if (first == NULL)
	{
		first = item;
	}
	else
	{
		ll_item_t * next = first;
		while (next->next != NULL)
		{
			next = next->next;
		}
		next->next = item;
	}

	return item;
}


void ll_delete_item(ll_item_t * to_delete)
{
	if (to_delete == first)
	{
		first = to_delete->next;
	}
	else
	{
		ll_item_t * item = first;
		while (item != NULL)
		{
			if (item->next == to_delete)
			{
				item->next = to_delete->next;
				break;
			}
		}
	}

	//free memory
	vSemaphoreDelete(to_delete->write);
	vSemaphoreDelete(to_delete->read);

	if (to_delete->data_ptr != NULL)
		free(to_delete->data_ptr);

	free(to_delete);
}

ll_item_t * ll_find_item(uint8_t msg_type, uint8_t msg_id)
{
	ll_item_t * item = first;
	while (item != NULL)
	{
		if (item->msg_type == msg_type && item->msg_id == msg_id)
			break;
		item = item->next;
	}
	return item;
}
