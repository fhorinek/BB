/*
 * dbg_overlay.c
 *
 *  Created on: 23. 11. 2021
 *      Author: horinek
 */

#include "gui.h"
#include "drivers/esp/protocol.h"

static osTimerId_t dbg_overlay_timer = NULL;

#define TIMER_PERIOD    2000
#define ITEM_HEIGHT 15

bool overlay_task_pending = false;

static int compare(const void * a, const void * b)
{
    if (((proto_tasks_item_t*)a)->number < ((proto_tasks_item_t*)b)->number)
        return -1;

    if (((proto_tasks_item_t*)a)->number > ((proto_tasks_item_t*)b)->number)
        return 1;

    return 0;
}

static const lv_color_t bar_colors[] = {LV_COLOR_BLUE, LV_COLOR_ORANGE, LV_COLOR_GREEN};

void dbg_overlay_tasks_update(uint8_t * packet)
{
    gui_lock_acquire();
    if (gui.dbg.tasks != NULL)
    {
        proto_tasks_head_t * head = (proto_tasks_head_t *)packet;
        head->number_of_tasks = min(head->number_of_tasks, 32);

        qsort(packet + sizeof(proto_tasks_head_t), head->number_of_tasks, sizeof(proto_tasks_item_t), compare);

        lv_obj_t * child = lv_obj_get_child_back(gui.dbg.tasks, NULL);
        uint16_t y = 0;
        uint64_t total_runtime = 0;

        for (uint8_t i = 0; i < head->number_of_tasks; i++)
        {
            proto_tasks_item_t * item = (proto_tasks_item_t *)(packet + sizeof(proto_tasks_head_t) + sizeof(proto_tasks_item_t) * i);

            total_runtime += item->run_time;
        }

        for (uint8_t i = 0; i < head->number_of_tasks; i++)
        {
            proto_tasks_item_t * item = (proto_tasks_item_t *)(packet + sizeof(proto_tasks_head_t) + sizeof(proto_tasks_item_t) * i);

            lv_obj_t * l;
            lv_obj_t * s;
            lv_obj_t * t;

            if (child == NULL)
            {
                lv_obj_t * o = lv_obj_create(gui.dbg.tasks, NULL);
                lv_obj_set_size(o, LV_HOR_RES, ITEM_HEIGHT);
                lv_obj_set_pos(o, 0, y);
                lv_obj_set_style_local_bg_opa(o, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
                y += ITEM_HEIGHT;

                t = lv_obj_create(o, NULL);
                lv_obj_align(t, o, LV_ALIGN_IN_TOP_LEFT, 0, 0);
                lv_obj_set_style_local_bg_color(t, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
                lv_obj_set_height(t, ITEM_HEIGHT);

                l = lv_label_create(o, NULL);
                lv_obj_align(l, o, LV_ALIGN_IN_TOP_LEFT, 0, 0);
                lv_obj_set_style_local_text_font(l, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_12);

                s = lv_label_create(o, NULL);
                lv_obj_align(s, o, LV_ALIGN_IN_TOP_RIGHT, 0, 0);
                lv_obj_set_auto_realign(s, true);
                lv_obj_set_style_local_text_font(s, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_12);

            }
            else
            {
                s = lv_obj_get_child(child, NULL);
                l = lv_obj_get_child(child, s);
                t = lv_obj_get_child(child, l);
                y = lv_obj_get_y(child) + ITEM_HEIGHT;
            }

            char c = (item->core == 2) ? ' ' : item->core + '0';

            lv_label_set_text_fmt(l, "%u %s", item->number, item->name);
            lv_label_set_text_fmt(s, "%c\t%u\t%u", c, item->priority, item->watermark);
            uint64_t a = item->run_time;

            lv_obj_set_width(t, (a * LV_HOR_RES) / total_runtime);
            if (bar_colors[item->core].full != lv_obj_get_style_bg_color(t, LV_OBJ_PART_MAIN).full)
                lv_obj_set_style_local_bg_color(t, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, bar_colors[item->core]);

            if (child != NULL)
                child = lv_obj_get_child_back(gui.dbg.tasks, child);
        }

        while (child != NULL)
        {
            lv_obj_del_async(child);
            child = lv_obj_get_child_back(gui.dbg.tasks, child);
        }

        lv_obj_move_foreground(gui.dbg.tasks);
        lv_obj_set_height(gui.dbg.tasks, y);
    }
    gui_lock_release();

    free(packet);

    overlay_task_pending = false;

    RedTaskUnregister();
    vTaskDelete(NULL);
}

static void get_stm_tasks(void * param)
{
    uint8_t cnt = uxTaskGetNumberOfTasks();
    uint32_t total_time;

    TaskStatus_t * task_status = (TaskStatus_t *) ps_malloc(cnt * sizeof(TaskStatus_t));

    cnt = uxTaskGetSystemState(task_status, cnt, &total_time);

    uint16_t buff_size = sizeof(proto_tasks_head_t) + sizeof(proto_tasks_item_t) * cnt;
    uint8_t * proto_buff = malloc(buff_size);

    proto_tasks_head_t * head = (proto_tasks_head_t *)proto_buff;
    head->number_of_tasks = cnt;

    if(total_time > 0UL)
    {
        for(uint8_t i = 0; i < cnt; i++ )
        {
            TaskStatus_t * ts = task_status + i;
            proto_tasks_item_t * item = (proto_tasks_item_t *)(proto_buff + sizeof(proto_tasks_head_t) + sizeof(proto_tasks_item_t) * i);
            item->run_time = ts->ulRunTimeCounter;
            strncpy(item->name, ts->pcTaskName, PROTO_TASK_NAME_LEN);
            item->watermark = ts->usStackHighWaterMark;
            item->number = ts->xTaskNumber;
            item->priority = ts->uxCurrentPriority;
            item->core = 2;
        }
    }

    ps_free(task_status);

    xTaskCreate((TaskFunction_t)dbg_overlay_tasks_update, "dbg_overlay_update", 1024 * 2, (void *)proto_buff, 24, NULL);

    RedTaskUnregister();
    vTaskDelete(NULL);
}

void dbg_overlay_tasks_step()
{
    if (overlay_task_pending)
        return;

    if (config_get_select(&config.debug.tasks) == DBG_TASK_ESP)
    {
        overlay_task_pending = true;
        protocol_send(PROTO_GET_TASKS, NULL, 0);
    }

    if (config_get_select(&config.debug.tasks) == DBG_TASK_STM)
    {
        overlay_task_pending = true;
        xTaskCreate((TaskFunction_t)get_stm_tasks, "get_stm_tasks", 1024 * 2, NULL, 24, NULL);
    }
}

void dbg_overlay_tasks_create()
{
    gui_lock_acquire();
    if (gui.dbg.tasks == NULL)
    {
        gui.dbg.tasks = lv_obj_create(lv_layer_sys(), NULL);
        lv_obj_set_pos(gui.dbg.tasks, 0, 0);
        lv_obj_set_size(gui.dbg.tasks, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_local_bg_opa(gui.dbg.tasks, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_50);

    }
    gui_lock_release();

    overlay_task_pending = false;

    if (dbg_overlay_timer == NULL)
        dbg_overlay_timer = osTimerNew(dbg_overlay_tasks_step, osTimerPeriodic, NULL, NULL);

    if (!osTimerIsRunning(dbg_overlay_timer))
        osTimerStart(dbg_overlay_timer, TIMER_PERIOD);
}

void dbg_overlay_tasks_remove()
{
    gui_lock_acquire();
    if (gui.dbg.tasks != NULL)
    {
        lv_obj_del(gui.dbg.tasks);
        gui.dbg.tasks = NULL;
    }
    gui_lock_release();

    osTimerStop(dbg_overlay_timer);
}

//no gui.lock required, run from GUI thread
void dbg_overlay_step()
{

    if (config_get_bool(&config.debug.lvgl_info))
    {
        if (gui.dbg.lv_info == NULL)
        {
            gui.dbg.lv_info = lv_label_create(lv_layer_sys(), NULL);
            lv_obj_align(gui.dbg.lv_info, NULL, LV_ALIGN_IN_TOP_RIGHT, 0, GUI_STATUSBAR_HEIGHT);
            lv_obj_set_auto_realign(gui.dbg.lv_info, true);
            lv_obj_set_style_local_bg_color(gui.dbg.lv_info, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
            lv_obj_set_style_local_text_color(gui.dbg.lv_info, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
            lv_obj_set_style_local_bg_opa(gui.dbg.lv_info, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
            lv_obj_set_style_local_pad_left(gui.dbg.lv_info, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 2);
        }

        lv_mem_monitor_t mem;
        lv_mem_monitor(&mem);

        lv_label_set_text_fmt(gui.dbg.lv_info, "RT %u/%u\n%u fps %u%%\n%lu free", RedTaskRegistered(), REDCONF_TASK_COUNT,  gui.fps, 100 - mem.used_pct, mem.free_size);
        lv_obj_move_foreground(gui.dbg.lv_info);
    }
    else
    {
        if (gui.dbg.lv_info != NULL)
        {
            lv_obj_del(gui.dbg.lv_info);
            gui.dbg.lv_info = NULL;

        }
    }
}

void dbg_overlay_init()
{
    gui.dbg.tasks = NULL;
    gui.dbg.lv_info = NULL;
    overlay_task_pending = false;
}

