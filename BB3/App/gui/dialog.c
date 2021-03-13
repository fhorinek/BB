/*
 * dialog.c
 *
 *  Created on: Jan 13, 2021
 *      Author: horinek
 */

#include "dialog.h"

#include "gui.h"

static void dialog_opa_anim(lv_obj_t * obj, lv_anim_value_t v)
{
    lv_obj_set_style_local_opa_scale(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, v);
}

void dialog_stop(dialog_result_t res)
{
    //restore groups
    lv_indev_reset(gui.input.indev, NULL);
    lv_indev_wait_release(gui.input.indev);
    lv_indev_set_group(gui.input.indev, gui.input.group);

    lv_group_remove_all_objs(gui.dialog.group);
    lv_obj_del_async(gui.dialog.window);

    if (gui.dialog.cb != NULL)
        gui.dialog.cb(res);
}

static void dialog_event_cb(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_KEY || event == LV_EVENT_CLICKED)
    {
        uint32_t key;

        if (event == LV_EVENT_CLICKED)
            key = LV_KEY_ENTER;
        else
            key = *((uint32_t*) lv_event_get_data());

        switch (gui.dialog.type)
        {
            case (dialog_yes_no):
                if (key == LV_KEY_ESC)
                    dialog_stop(dialog_res_no);
                if (key == LV_KEY_ENTER)
                    dialog_stop(dialog_res_yes);
            break;
        }
    }

}
void dialog_progress_set_progress(uint8_t progress)
{
    lv_obj_t * cont = lv_obj_get_child_back(gui.dialog.window, NULL);
    lv_obj_t * p = lv_obj_get_child(cont, NULL);
    lv_obj_t * spinner = lv_obj_get_child(cont, p);

    lv_arc_set_angles(spinner, 270, 270 + 360 * (progress / 100.0));
    lv_obj_invalidate(spinner);

    lv_obj_t * label = lv_obj_get_child(gui.dialog.window, NULL);
    lv_label_set_text_fmt(label, "%u%%", progress);
}

void dialog_progress_set_subtitle(char * text)
{
    lv_obj_t * cont = lv_obj_get_child_back(gui.dialog.window, NULL);
    lv_obj_t * label = lv_obj_get_child(cont, NULL);

    lv_label_set_text(label, text);
}

void dialog_close()
{
    dialog_stop(dialog_res_none);
}

void dialog_show(char * title, char * message, dialog_type_t type, gui_dialog_cb_t cb)
{
    //switch group
    lv_indev_set_group(gui.input.indev, gui.dialog.group);

    gui.dialog.cb = cb;
    gui.dialog.type = type;
    gui.dialog.window = lv_obj_create(lv_layer_sys(), NULL);
    lv_obj_set_pos(gui.dialog.window, 0, 0);
    lv_obj_set_size(gui.dialog.window, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_local_bg_color(gui.dialog.window, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_COLOR_BLACK);

    lv_group_add_obj(gui.dialog.group, gui.dialog.window);
    lv_obj_set_event_cb(gui.dialog.window, dialog_event_cb);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, gui.dialog.window);
    lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_90);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)dialog_opa_anim);
    lv_anim_start(&a);

    lv_obj_t * cont = lv_cont_create(gui.dialog.window, NULL);
    lv_cont_set_layout(cont, LV_LAYOUT_COLUMN_MID);
    lv_obj_set_auto_realign(cont, true);
    lv_obj_align_origo(cont, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_cont_set_fit(cont, LV_FIT_TIGHT);
    lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    lv_obj_t * title_label = lv_label_create(cont, NULL);
    lv_label_set_align(title_label, LV_LABEL_ALIGN_CENTER);
    lv_obj_set_style_local_text_font(title_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_22);
    lv_label_set_text(title_label, title);

    lv_obj_t * text = lv_label_create(cont, NULL);
    lv_label_set_align(text, LV_LABEL_ALIGN_CENTER);
    lv_label_set_long_mode(text, LV_LABEL_LONG_BREAK);
    lv_obj_set_width(text, (LV_HOR_RES * 3) / 4);
    lv_obj_set_style_local_pad_top(text, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_label_set_text(text, message);

    switch (type)
    {
        case (dialog_yes_no):
        {
            lv_obj_t * yes = lv_label_create(gui.dialog.window, NULL);
            lv_obj_set_style_local_text_font(yes, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_22);
            lv_obj_set_style_local_text_color(yes, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
            lv_obj_set_style_local_pad_all(yes, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 5);
            lv_label_set_text(yes, LV_SYMBOL_OK);
            lv_obj_align(yes, gui.dialog.window, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

            lv_obj_t * no = lv_label_create(gui.dialog.window, NULL);
            lv_obj_set_style_local_text_font(no, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_22);
            lv_obj_set_style_local_text_color(no, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
            lv_obj_set_style_local_pad_all(no, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 5);
            lv_label_set_text(no, LV_SYMBOL_CLOSE);
            lv_obj_align(no, gui.dialog.window, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
        }
        break;

        case (dialog_progress):
        {
            lv_obj_t * spinner = lv_arc_create(cont, NULL);
            lv_obj_set_size(spinner, 150, 150);
            lv_obj_align(spinner, cont, LV_ALIGN_CENTER, 0, 0);
            lv_arc_set_start_angle(spinner, 270);

            lv_obj_t * progress = lv_label_create(gui.dialog.window, NULL);
            lv_label_set_align(progress, LV_LABEL_ALIGN_CENTER);
            lv_obj_align(progress, spinner, LV_ALIGN_CENTER, 0, 0);

            lv_obj_t * subtitle = lv_label_create(cont, NULL);
            lv_label_set_align(subtitle, LV_LABEL_ALIGN_CENTER);
        }
        break;
    }
}

