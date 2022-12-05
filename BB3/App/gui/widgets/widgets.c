/*
 * widgets.cc
 *
 *  Created on: 10. 8. 2020
 *      Author: horinek
 */

#include "widgets.h"
#include "gui/statusbar.h"

bool widgets_save_to_file(page_layout_t * page, char * layout_name)
{
    char path[PATH_LEN];

    snprintf(path, sizeof(path), "%s/%s/%s.pag", PATH_PAGES_DIR, config_get_text(&config.flight_profile), layout_name);
    char buff[32];

    int32_t f = red_open(path, RED_O_WRONLY | RED_O_TRUNC | RED_O_CREAT);
    if (f > 0)
    {
        int8_t len = snprintf(buff, sizeof(buff), "widgets=%u\n", page->number_of_widgets);
        red_write(f, buff, len);

        for (uint8_t i = 0; i < page->number_of_widgets; i++)
        {
            widget_slot_t * ws = &page->widget_slots[i];

            len = snprintf(buff, sizeof(buff), "w%u_type=%s\n", i, ws->widget->short_name);
            red_write(f, buff, len);

            //store widget parameters
            len = snprintf(buff, sizeof(buff), "w%u_x=%d\n", i, ws->x);
            red_write(f, buff, len);
            len = snprintf(buff, sizeof(buff), "w%u_y=%d\n", i, ws->y);
            red_write(f, buff, len);
            len = snprintf(buff, sizeof(buff), "w%u_w=%d\n", i, ws->w);
            red_write(f, buff, len);
            len = snprintf(buff, sizeof(buff), "w%u_h=%d\n", i, ws->h);
            red_write(f, buff, len);
            len = snprintf(buff, sizeof(buff), "w%u_f=%s\n", i, ws->flags);
            red_write(f, buff, len);
        }
        red_close(f);

        return true;
    }
    return false;
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
    page->number_of_widgets = 0;
    page->base = NULL;
    page->widget_slots = NULL;

    if (!file_exists(path))
        return false;

    int32_t f = red_open(path, RED_O_RDONLY);
    char buff[32];

    for (;;)
    {
        char * line = red_gets(buff, sizeof(buff), f);
        if (line == NULL)
            break;

        if (line == NULL)
        {
            statusbar_msg_add(STATUSBAR_MSG_ERROR, "Page file corrupted");
            break;
        }

        line[strlen(line) - 1] = 0;

        char key[16];
        char value[16];
        char * sep = strchr(line, '=');
        if (sep == NULL)
        {
            WARN("Wrong line formating '%s'", line);
            continue;
        }
        strncpy(key, line, sep - line);
        key[sep - line] = 0;
        strcpy(value, sep + 1);

        if (strcmp(key, "widgets") == 0)
        {
            if (page->widget_slots == NULL)
            {
                page->number_of_widgets = atoi(value);

                if (page->number_of_widgets > 0)
                    page->widget_slots = (widget_slot_t *) tmalloc(sizeof(widget_slot_t) * page->number_of_widgets);
                else
                    page->widget_slots = NULL;

                for (uint8_t i = 0; i < page->number_of_widgets; i++)
                {
                    //set default for widgets
                    widget_slot_t * ws = &page->widget_slots[i];
                    ws->widget = NULL;
                    ws->vars = NULL;
                    ws->obj = NULL;
                    ws->x = 0;
                    ws->y = 0;
                    ws->w = 0;
                    ws->h = 0;
                    ws->flags[0] = 0;
                }
            }
            else
            {
                ERR("Number of widgets already set");
            }
            continue;
        }

        unsigned int id;
        char ss[8];
        sscanf(key, "w%u_%s", &id, ss);
        if (id < page->number_of_widgets)
        {
            widget_slot_t * ws = &page->widget_slots[id];
            if (ss[0] == 'x')
                ws->x = atoi(value);
            else if (ss[0] == 'y')
                ws->y = atoi(value);
            else if (ss[0] == 'w')
                ws->w = atoi(value);
            else if (ss[0] == 'h')
                ws->h = atoi(value);
            else if (ss[0] == 'f')
                strcpy(ws->flags, value);
            else if (strcmp(ss, "type") == 0)
                ws->widget = widget_find_by_name(value);

            continue;
        }
    }

    for (uint8_t i = 0; i < page->number_of_widgets; i++)
    {
        //if widget is unknown, set default
        widget_slot_t * ws = &page->widget_slots[i];

        if (ws->widget == NULL)
        {
            ws->widget = widgets[0];
        }

        widget_dimension_check(ws);
    }

    red_close(f);

	return true;
}

bool widgets_load_from_file(page_layout_t * page, char * layout_name)
{
	char path[PATH_LEN];

	snprintf(path, sizeof(path), PATH_PAGES_DIR "/%s/%s.pag", config_get_text(&config.flight_profile), layout_name);

	if (!file_exists(path))
        snprintf(path, sizeof(path), PATH_ASSET_DIR "/defaults/pages/%s.pag", layout_name);

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
        ws->vars = tmalloc(ws->widget->vars_size);
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
        tfree(ws->vars);
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
        tfree(page->widget_slots);
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
    widget_slot_t * new_widget_slots = (widget_slot_t *) tmalloc(sizeof(widget_slot_t) * (page->number_of_widgets + 1));
    if (page->number_of_widgets > 0)
    {
        memcpy(new_widget_slots, page->widget_slots, sizeof(widget_slot_t) * page->number_of_widgets);
        tfree(page->widget_slots);
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
            new_widget_slots = (widget_slot_t *) tmalloc(sizeof(widget_slot_t) * page->number_of_widgets);
            safe_memcpy(new_widget_slots, page->widget_slots, sizeof(widget_slot_t) * page->number_of_widgets);
        }

        tfree(page->widget_slots);
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

