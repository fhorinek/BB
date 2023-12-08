/*
 * help.c
 *
 *  Created on: 18. 10. 2022
 *      Author: horinek
 */

#include "help.h"
#include "gui/gui.h"
#include "gui/tasks/menu/flightbook/flightbook_statistics.h"

static char help_path[256] = {0};
static char help_path_default[256] = {0};

void help_init_gui()
{
    gui.help.group = lv_group_create();

    gui.help.icon = lv_led_create(lv_layer_sys(), NULL);
    lv_obj_set_size(gui.help.icon, 22, 22);

    lv_obj_t * label = lv_label_create(gui.help.icon, NULL);
    lv_label_set_text(label, MD_HELP_CIRCLE);
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 1, 0);
    lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

    lv_obj_set_hidden(gui.help.icon, true);
}

void help_unset()
{
    help_path[0] = '\0';
    help_path_default[0] = '\0';
    help_icon_hide();
}

void help_set_base(char * base_id)
{
    help_path[0] = '\0';
    strcpy(help_path, PATH_HELP_DIR);
    strcat(help_path, "/");

    strcat(help_path, languages_id[config_get_select_index(&config.display.language)]);

    strcat(help_path, "/");
    strcat(help_path, base_id);

    INFO("help base_id: %s", help_path);
    red_mkdirs(help_path);

    help_path_default[0] = '\0';
    strcpy(help_path_default, PATH_HELP_DIR);
    strcat(help_path_default, "/");

    strcat(help_path_default, languages_id[LANG_EN]);

    strcat(help_path_default, "/");
    strcat(help_path_default, base_id);

    INFO("help default base_id: %s", help_path_default);
    red_mkdirs(help_path_default);

}

void help_title_to_id(char * id, char * title)
{
    id[0] = '\0';
    uint8_t i = 0;
    uint8_t r = 0;
    while (i < HELP_ID_LEN - 1)
    {
        char c = title[r];

        if (c == '\0')
            break;

        //digit or alpha is ok
        if (IS_DIGIT(c) || IS_ALPHA(c))
        {
            id[i] = c;
            i++;
        }

        //space is ok
        if (c == ' ')
        {
            if (i != 0) //but not at the beginning
            {
                if (id[i - 1] != ' ') //or more spaces together
                {
                    id[i] = c;
                    i++;
                }
            }
        }

        r++;
    }
    id[i] = '\0';

    //remove trailing space
    if (id[i - 1] == ' ')
        id[i - 1] = '\0';
}



bool help_avalible(char * id)
{
    if (help_path[0] == 0)
        return false;

    char file_path[PATH_LEN];
    snprintf(file_path, sizeof(file_path), "%s/%s.txt", help_path, id);

    bool ret = file_exists(file_path);

    if (!ret)
    {
        snprintf(file_path, sizeof(file_path), "%s/%s.txt", help_path_default, id);
        ret = file_exists(file_path);

        if (config_get_bool(&config.debug.help_show_id))
        {
            if (ret)
            {
                lv_obj_set_style_local_bg_color(gui.help.icon, LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
                lv_obj_set_style_local_shadow_color(gui.help.icon, LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
            }
            else
            {
                lv_obj_set_style_local_bg_color(gui.help.icon, LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
                lv_obj_set_style_local_shadow_color(gui.help.icon, LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
            }
            return true;
        }
    }

    if (ret)
    {
        lv_obj_set_style_local_bg_color(gui.help.icon, LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
        lv_obj_set_style_local_shadow_color(gui.help.icon, LV_LED_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

        return true;
    }

    return false;
}

bool help_show_icon_if_avalible_from_title(char * title)
{
    if (help_path[0] == 0)
        return false;

    char id[HELP_ID_LEN];
    help_title_to_id(id, title);

    //INFO("help_show_icon_if_avalible_from_title %s -> %s", title, id);

    if (strlen(id) > 0 && help_avalible(id))
    {
        help_icon_show();
        return true;
    }
    else
    {
        help_icon_hide();
        return false;
    }
}

bool help_show_icon_if_avalible(char * id)
{
    if (help_path[0] == 0)
        return false;

    if (strlen(id) > 0 && help_avalible(id))
    {
        help_icon_show();
        return true;
    }
    else
    {
        help_icon_hide();
        return false;
    }
}

void help_icon_show()
{
    lv_obj_set_hidden(gui.help.icon, false);
    lv_obj_move_foreground(gui.help.icon);

    if (gui.ctx.mode == ctx_active || gui.task.actual == &gui_flightbook_statistics)
        lv_obj_align(gui.help.icon, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -2, -30);
    else
        lv_obj_align(gui.help.icon, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -2, -5);
}

void help_icon_hide()
{
    lv_obj_set_hidden(gui.help.icon, true);
}


static void help_done_cb(lv_anim_t * a)
{
    lv_obj_del_async(gui.help.window);
}

static void help_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_CANCEL || event == LV_EVENT_CLICKED)
    {
        //restore groups
        lv_group_set_editing(gui.help.group, false);
        lv_indev_reset(gui.input.indev, NULL);
        lv_indev_wait_release(gui.input.indev);
        lv_indev_set_group(gui.input.indev, gui.input.group);

        lv_group_remove_all_objs(gui.help.group);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, gui.help.window);
        lv_anim_set_values(&a, 0, LV_VER_RES);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_ready_cb(&a, help_done_cb);
        lv_anim_start(&a);
    }

    else if(event == LV_EVENT_KEY)
    {
        int32_t key = *((uint32_t*) lv_event_get_data());

        int16_t scroll = 0;
        if (key == LV_KEY_LEFT)
            scroll = +1;
        if (key == LV_KEY_RIGHT)
            scroll = -1;

        if (scroll != 0)
        {
            lv_obj_t * s = lv_obj_get_child(gui.help.window, NULL);
            lv_page_scroll_ver(s, scroll * 16);
        }
    }

}


lv_obj_t * help_create_gui()
{
    //switch indev
    lv_indev_set_group(gui.input.indev, gui.help.group);

    gui.help.window = lv_obj_create(lv_layer_sys(), NULL);
    lv_obj_set_pos(gui.help.window, 0, 0);
    lv_obj_set_size(gui.help.window, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_local_bg_color(gui.help.window, LV_OBJ_PART_MAIN, LV_STATE_EDITED, LV_COLOR_BLACK);
    lv_obj_set_style_local_bg_opa(gui.help.window, LV_OBJ_PART_MAIN, LV_STATE_EDITED, LV_OPA_90);

    lv_group_add_obj(gui.help.group, gui.help.window);
    lv_obj_set_event_cb(gui.help.window, help_event_cb);
    lv_group_set_editing(gui.help.group, true);

    lv_obj_t * win = lv_page_create(gui.help.window, NULL);
    lv_obj_set_style_local_bg_opa(win, LV_PAGE_PART_BG, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(win, LV_PAGE_PART_SCROLLABLE, LV_STATE_DEFAULT, LV_OPA_TRANSP);
    lv_obj_set_size(win, LV_HOR_RES, LV_VER_RES);
    lv_page_set_scrl_layout(win, LV_LAYOUT_COLUMN_LEFT);
    //lv_page_set_scrollable_fit2(win, LV_FIT_PARENT, LV_FIT_PARENT);
    lv_page_set_anim_time(win, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, gui.help.window);
    lv_anim_set_values(&a, LV_VER_RES, 0);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_start(&a);

    return win;
}


void help_show(char * id)
{
    if (help_path[0] == 0)
        return;

    char file_path[PATH_LEN];
    snprintf(file_path, sizeof(file_path), "%s/%s.txt", help_path, id);

    if (!file_exists(file_path) && !config_get_bool(&config.debug.help_show_id))
    {
        snprintf(file_path, sizeof(file_path), "%s/%s.txt", help_path_default, id);
    }

    int32_t f = red_open(file_path, RED_O_RDONLY);
    if (f > 0)
    {
        lv_obj_t * content = help_create_gui();

        while (true)
        {
            char line[1024];
            char * pos = red_gets(line, sizeof(line), f);
            if (pos == NULL || pos == GETS_CORRUPTED)
                break;

            lv_obj_t * label = lv_label_create(content, NULL);
            lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);
            lv_label_set_recolor(label, true);

            //Bigger font when starts with *
            if (*pos == '*')
            {
                lv_obj_set_style_local_text_font(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_22);
                pos++;
            }

            //remove trailing \n
            if (pos[strlen(pos) - 1] == '\n')
                pos[strlen(pos) - 1] = '\0';

            //decode icons
            for (uint16_t i = 0; i < strlen(pos); i++)
            {
                if (pos[i] == '[' && pos[i + 9] == ']')
                {
                    byte4 tmp;
                    tmp.uint32 = atoi_hex32(&pos[i + 1]);
                    for (uint8_t j = 0; j < 4; j++)
                    {
                        pos[i + j] = tmp.uint8[3 - j];
                    }
                    //replace rest of the notation with \1, lvgl will not complain and will not draw them
                    memset(&pos[i + 4], 1, 6);
                    i += 8;
                }
            }

            lv_obj_set_width(label, lv_page_get_width_fit(content));
            lv_label_set_text(label, pos);
        }

        red_close(f);
    }
    else
    {
        if (config_get_bool(&config.debug.help_show_id))
        {
            f = red_open(file_path, RED_O_WRONLY | RED_O_CREAT);
            char buff[64];

            sprintf(buff, "*%s\n\nItem description\n\n", id);
            red_write(f, buff, strlen(buff));

            strcpy(buff, "File path:\n");
            red_write(f, buff, strlen(buff));
            red_write(f, file_path, strlen(file_path));

            strcpy(buff, "\n");
            red_write(f, buff, strlen(buff));

            red_close(f);

            help_show(id);
        }
    }
}

void help_show_from_title(char * title)
{
    if (help_path[0] == 0)
        return;

    char id[HELP_ID_LEN];
    help_title_to_id(id, title);
    help_show(id);
}


