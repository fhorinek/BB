/*
 * widgets.cc
 *
 *  Created on: 10. 8. 2020
 *      Author: horinek
 */

#include "widgets.h"

bool widgets_save_to_file(page_layout_t * page, char * layout_name)
{
    char path[PATH_LEN];
    uint8_t ret;

    snprintf(path, sizeof(path), "%s/%s/%s.pag", PATH_PAGES_DIR, config_get_text(&config.flight_profile), layout_name);
    char key[16];

    db_insert_int(path, "widgets", page->number_of_widgets);

    for (uint8_t i = 0; i < page->number_of_widgets; i++)
    {
        widget_slot_t * ws = &page->widget_slots[i];

        snprintf(key, sizeof(key), "w%u_type");
        db_insert(path, key, ws->widget->short_name);

        //store widget parameters
        snprintf(key, sizeof(key), "w%u_x", i);
        db_insert_int(path, key, ws->x);
        snprintf(key, sizeof(key), "w%u_y", i);
        db_insert_int(path, key, ws->y);
        snprintf(key, sizeof(key), "w%u_w", i);
        db_insert_int(path, key, ws->w);
        snprintf(key, sizeof(key), "w%u_h", i);
        db_insert_int(path, key, ws->h);
        snprintf(key, sizeof(key), "w%u_f", i);
        db_insert(path, key, ws->flags);
    }

    return true;
}

void widget_dimension_check(widget_slot_t * ws)
{
    ws->w = max(ws->w, ws->widget->w_min);
    ws->h = max(ws->h, ws->widget->h_min);
    ws->x = min(ws->x, LV_HOR_RES - ws->w);
    ws->y = min(ws->y, LV_VER_RES - GUI_STATUSBAR_HEIGHT - ws->h);
}

bool widgets_load_from_file_abs(page_layout_t * page, char * path)
{
    if (!file_exists(path))
        return;

    db_query_int_def(path, "widgets", &page->number_of_widgets, 0);
    page->base = NULL;

	if (page->number_of_widgets > 0)
	{
	    //allocate memory for widget array
	    page->widget_slots = (widget_slot_t *) malloc(sizeof(widget_slot_t) * page->number_of_widgets);
		ASSERT(page->widget_slots != NULL);

		for (uint8_t i = 0; i < page->number_of_widgets; i++)
		{
			widget_slot_t * ws = &page->widget_slots[i];
			ws->widget = NULL;

			char key[16];

			//Get widget type
			snprintf(key, sizeof(key), "w%u_type", i);
			char type[16];

			if (db_query(path, key, type, sizeof(type)))
			    ws->widget = widget_find_by_name(type);

			if (ws->widget == NULL)
			{
				ws->widget = widgets[0];
				WARN("widget[%u] type '%s' unknown", i, type);
			}

			//get widget parameters
			snprintf(key, sizeof(key), "w%u_x", i);
			db_query_int_def(path, key, &ws->x, 0);

			snprintf(key, sizeof(key), "w%u_y", i);
            db_query_int_def(path, key, &ws->y, 0);

			snprintf(key, sizeof(key), "w%u_w", i);
            db_query_int_def(path, key, &ws->w, 0);

			snprintf(key, sizeof(key), "w%u_h", i);
            db_query_int_def(path, key, &ws->h, 0);

			snprintf(key, sizeof(key), "w%u_f", i);
			char flags[WIDGET_FLAGS_LEN];
			if (db_query_def(path, key, flags, sizeof(flags), ""))
				strncpy(ws->flags, flags, WIDGET_FLAGS_LEN - 1);
			else
				ws->flags[0] = 0;

			widget_dimension_check(ws);

            ws->vars = NULL;
            ws->obj = NULL;
		}
	}

	return true;
}

bool widgets_load_from_file(page_layout_t * page, char * layout_name)
{
	char path[PATH_LEN];

	snprintf(path, sizeof(path), "%s/%s/%s.pag", PATH_PAGES_DIR, config_get_text(&config.flight_profile), layout_name);
	return widgets_load_from_file_abs(page, path);
}

void widgets_sort_page(page_layout_t * page)
{
	if (page->number_of_widgets <= 1)
		return;

	bool swaped = false;
    do
    {
    	swaped = false;
    	for (uint8_t i = 0; i < page->number_of_widgets - 1; i++)
    	{
    		widget_slot_t * left = &page->widget_slots[i];
    		widget_slot_t * right = &page->widget_slots[i + 1];

    		uint32_t left_val = left->x + left->y * LV_HOR_RES;
    		uint32_t right_val = right->x + right->y * LV_HOR_RES;

    		if (left_val > right_val)
    		{
    			swaped = true;
    			__align widget_slot_t tmp;
    			safe_memcpy(&tmp, left, sizeof(widget_slot_t));
    			safe_memcpy(left, right, sizeof(widget_slot_t));
    			safe_memcpy(right, &tmp, sizeof(widget_slot_t));
    		}
    	}
    } while (swaped);
}


widget_t * widget_find_by_name(char * widget_short_name)
{
    for (uint8_t i = 0; i < number_of_widgets(); i++)
    {
        if (strcmp(widget_short_name, widgets[i]->short_name) == 0)
        {
            return widgets[i];
        }
    }

    return NULL;
}

void widget_init(widget_slot_t * ws, lv_obj_t * par)
{
    ws->title = NULL;
    widget_dimension_check(ws);

    //allocate memory for widget
    if (ws->widget->vars_size > 0)
    {
        ws->vars = malloc(ws->widget->vars_size);
        ASSERT(ws->vars != NULL);
    }
    else
    {
        ws->vars = NULL;
    }

    //run the initialisation
    ws->widget->init(par, ws);
    widget_update(ws);
}

void widget_update(widget_slot_t * ws)
{
    if (ws->widget->update != NULL)
        ws->widget->update(ws);
}

lv_obj_t * widgets_add_page_empty_label(page_layout_t * page)
{
    //page not found
    lv_obj_t * label = lv_label_create(page->base, NULL);
    lv_label_set_text(label, "Page is empty.");
    lv_obj_align_origo(label, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_move_background(label);

    return label;
}

void widgets_create_base(page_layout_t * page, lv_obj_t * par)
{
    //create base
    page->base = lv_obj_create(par, NULL);
    lv_obj_set_style_local_bg_color(page->base, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_size(page->base, LV_HOR_RES, LV_VER_RES - GUI_STATUSBAR_HEIGHT);
    lv_obj_set_pos(page->base, 0, 0);
    lv_obj_move_background(page->base);
}

void widgets_init_page(page_layout_t * page, lv_obj_t * par)
{
    widgets_create_base(page, par);

    //init loaded widgets
    for (uint8_t i = 0; i < page->number_of_widgets; i++)
    {
        widget_init(&page->widget_slots[i], page->base);
    }

    if (page->number_of_widgets == 0)
    {
        widgets_add_page_empty_label(page);
    }
}


void widget_deinit(widget_slot_t * ws)
{
    //widget specific stop
    if (ws->widget->stop != NULL)
        ws->widget->stop(ws);

    //free widget extra memory
    if (ws->vars != NULL)
        free(ws->vars);
}

void widgets_deinit_page(page_layout_t * page)
{
	for (uint8_t i = 0; i < page->number_of_widgets; i++)
	{
	    widget_deinit(&page->widget_slots[i]);
	}

	widgets_free(page);
}

void widgets_free(page_layout_t * page)
{
    //free widget slot pointer memory
    if (page->number_of_widgets > 0)
        free(page->widget_slots);
}

bool widgets_editable(page_layout_t * page)
{
	for (uint8_t i = 0; i < page->number_of_widgets; i++)
	{
		if (page->widget_slots[i].widget->edit != NULL)
			return true;
	}

	return false;
}

void widgets_edit(widget_slot_t * ws, uint8_t action)
{
	ASSERT(ws != NULL);
    ws->widget->edit(ws, action);
}

widget_slot_t * widgets_editable_select_next(page_layout_t * page, widget_slot_t * last)
{
    if (!widgets_editable(page))
        return NULL;

    for (uint8_t i = 0; i < page->number_of_widgets; i++)
    {
        if (page->widget_slots[i].widget->edit == NULL)
            continue;

        if (last == NULL)
            return &page->widget_slots[i];

        if (last == &page->widget_slots[i])
            last = NULL;
    }

    return NULL;
}

void widgets_update(page_layout_t * page)
{
    for (uint8_t i = 0; i < page->number_of_widgets; i++)
    {
        widget_update(&page->widget_slots[i]);
    }
}

void widgets_add(page_layout_t * page, widget_t * w)
{
    widget_slot_t * new_widget_slots = (widget_slot_t *) malloc(sizeof(widget_slot_t) * (page->number_of_widgets + 1));
    if (page->number_of_widgets > 0)
    {
        memcpy(new_widget_slots, page->widget_slots, sizeof(widget_slot_t) * page->number_of_widgets);
        free(page->widget_slots);
    }
    page->widget_slots = new_widget_slots;

    page->widget_slots[page->number_of_widgets].widget = w;
    page->widget_slots[page->number_of_widgets].x = 0;
    page->widget_slots[page->number_of_widgets].y = 0;
    page->widget_slots[page->number_of_widgets].w = WIDGET_DEFAULT_W;
    page->widget_slots[page->number_of_widgets].h = WIDGET_DEFAULT_H;
    page->widget_slots[page->number_of_widgets].flags[0] = 0;

    widget_init(&page->widget_slots[page->number_of_widgets], page->base);

    page->number_of_widgets++;
}

void widgets_change(page_layout_t * page, uint8_t index, widget_t * w)
{
    widget_deinit(&page->widget_slots[index]);

    page->widget_slots[index].widget = w;

    widget_init(&page->widget_slots[index], page->base);
}

void widgets_remove(page_layout_t * page, uint8_t index)
{
    if (index < page->number_of_widgets)
    {
        widget_deinit(&page->widget_slots[index]);

        for (uint8_t i = index; i < page->number_of_widgets - 1; i++)
        {
            safe_memcpy(&page->widget_slots[i], &page->widget_slots[i + 1], sizeof(widget_slot_t));
        }

        page->number_of_widgets--;

        widget_slot_t * new_widget_slots = NULL;
        if (page->number_of_widgets > 0)
        {
            new_widget_slots = (widget_slot_t *) malloc(sizeof(widget_slot_t) * page->number_of_widgets);
            safe_memcpy(new_widget_slots, page->widget_slots, sizeof(widget_slot_t) * page->number_of_widgets);
        }

        free(page->widget_slots);
        page->widget_slots = new_widget_slots;
    }


}

bool widget_flag_is_set(widget_slot_t * slot, widget_flags_id_t flag_id)
{
	char flag = widgets_flags[flag_id].flag;
	char * pos = strchr(slot->flags, flag);
	return pos != 0;
}

void widget_flag_set(widget_slot_t * slot, widget_flags_id_t flag_id)
{
	if (!widget_flag_is_set(slot, flag_id))
	{
	    char flag = widgets_flags[flag_id].flag;
		uint8_t len = strlen(slot->flags);
		if (len < WIDGET_FLAGS_LEN - 1)
		{
			slot->flags[len] = flag;
			slot->flags[len + 1] = 0;
		}
	}
}

void widget_flag_unset(widget_slot_t * slot, widget_flags_id_t flag_id)
{
	if (widget_flag_is_set(slot, flag_id))
	{
	    char flag = widgets_flags[flag_id].flag;
		for (uint8_t i = 0; i < WIDGET_FLAGS_LEN - 1; i++)
		{
			if (slot->flags[i] == flag)
			{
				for (uint8_t j = i; j < WIDGET_FLAGS_LEN - 1; j++)
					slot->flags[j] = slot->flags[j + 1];
				slot->flags[WIDGET_FLAGS_LEN - 1] = 0;
			}
		}
	}
}

