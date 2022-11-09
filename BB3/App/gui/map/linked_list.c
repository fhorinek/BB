/*
 * linked_list.c
 *
 *  Created on: 24. 11. 2020
 *      Author: horinek
 */


#include "linked_list.h"

static char malloc_tag[] = "linked_list";

void list_add_sorted_unique(uint32_t addr, ll_item_t ** list_start, ll_item_t ** list_end)
{
    ll_item_t * add_item(ll_item_t * actual, ll_item_t * last, ll_item_t ** list_end)
    {
        ll_item_t * new = (ll_item_t *) tmalloc_2(sizeof(ll_item_t), malloc_tag);

        new->feature_addr = addr;
        new->prev = actual;
        new->next = last;

        if (last == NULL)
            *list_end = new;
        else
            last->prev = new;

        return new;
    }

    ll_item_t * actual = *list_end;
    ll_item_t * last = NULL;

    while (true)
    {
        if (actual == NULL)
        {
            *list_start = add_item(actual, last, list_end);;
            return;
        }

        if(actual->feature_addr < addr)
        {
            actual->next = add_item(actual, last, list_end);
            return;
        }

        //the addr is already in the list, return
        if (actual->feature_addr == addr)
        {
            return;
        }

        if (actual->feature_addr > addr)
        {
            last = actual;
            actual = actual->prev;
            continue;
        }
    }
}

void list_dbg(ll_item_t * list_start)
{
	ll_item_t * actual = list_start;
	uint32_t cnt = 0;

	printf("---------------------\n");

	uint32_t last = 0;

	while(actual != NULL)
	{
//		printf(":%8lX\n", actual);
		printf("addr: %lu\n", actual->feature_addr);
		if (last > actual->feature_addr)
		{
			printf("Error not rising!");
			return;
		}
		last = actual->feature_addr;
//		printf("prev: %8lX\n", actual->prev);
//		printf("next: %8lX\n", actual->next);
//		printf("\n");

		actual = actual->next;
		cnt++;
	}

	printf("Total %lu\n", cnt);
}


void list_free(ll_item_t * list_start, ll_item_t * list_end)
{
	ll_item_t * actual = list_start;
	ll_item_t * tmp;

	while(actual != NULL)
	{
		tmp = actual;
		actual = actual->next;
		tfree_2(tmp, malloc_tag, sizeof(ll_item_t));
	}
}
