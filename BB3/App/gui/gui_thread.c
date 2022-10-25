/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "gui_thread.h"
#include "gui.h"

#include "map/map_thread.h"
#include "drivers/sensors/mems_thread.h"
#include "drivers/esp/esp.h"
#include "drivers/gnss/gnss_thread.h"

#include "lib/lvgl/lvgl.h"
#include "lib/lvgl/src/lv_gpu/lv_gpu_stm32_dma2d.h"

#include "drivers/tft/tft.h"
#include "drivers/power/led.h"

void gui_save_screen()
{
    static uint16_t index = 0;
    char path[32];

    red_mkdir(PATH_SCREENSHOT);

    int32_t f;
    do
    {
        sprintf(path, "%s/%08u.scr", PATH_SCREENSHOT, index);
        index++;
        f = red_open(path, RED_O_WRONLY);
    } while (f < 0);

    INFO("Saving screenshot to %s", path);

    red_write(f, tft_buffer, TFT_BUFFER_SIZE * sizeof(uint16_t));
    red_close(f);

    gui.take_screenshot = 0;
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_flush_ready()' has to be called when finished
 * This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
static lv_disp_drv_t * last_dsp_drv = NULL;
void gui_disp_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t * color_p)
{
    last_dsp_drv = drv;

    if (gui.take_screenshot == 2)
    {
        //broken with double buffering method
        gui_save_screen();
    }
    else
    {
        tft_refresh_buffer(area->x1, area->y1, area->x2, area->y2, (uint16_t *)color_p);
    }

	//lv_disp_flush_ready(drv);
}

void gui_disp_ready()
{
    if (last_dsp_drv != NULL)
    {
        lv_disp_flush_ready(last_dsp_drv);
        last_dsp_drv = NULL;
    }
}

static bool gui_pin_set[5] = {0};


void gui_set_pin(uint8_t i)
{
    gui_pin_set[i] = 1;
}

bool gui_get_button_state(uint8_t index)
{
	GPIO_TypeDef* button_port[] = {BT1_GPIO_Port, BT2_GPIO_Port, BT3_GPIO_Port, BT4_GPIO_Port, BT5_GPIO_Port};
	uint16_t button_pin[] = {BT1_Pin, BT2_Pin, BT3_Pin, BT4_Pin, BT5_Pin};

	bool ret = (HAL_GPIO_ReadPin(button_port[index], button_pin[index]) == LOW) || gui_pin_set[index];
	gui_pin_set[index] = 0;

	return ret;
}

bool gui_input_cb(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
	static uint8_t last_key_index = 0;
	static uint8_t last_key_state = LV_INDEV_STATE_REL;

	uint8_t key_map[] = {LV_KEY_LEFT, LV_KEY_ESC, LV_KEY_ENTER, LV_KEY_HOME, LV_KEY_RIGHT};

	if (last_key_state == LV_INDEV_STATE_REL)
	{
		for (uint8_t i = 0; i < sizeof(key_map); i++)
		{
			if (gui_get_button_state(i))
			{
				last_key_index = i;
				last_key_state = LV_INDEV_STATE_PR;
				break;
			}
		}
	}
	else
	{
		if (gui_get_button_state(last_key_index) == false)
			last_key_state = LV_INDEV_STATE_REL;
	}

	data->state = last_key_state;
	data->key = key_map[last_key_index];

	return false;
}

/**
 * Log print function. Receives "Log Level", "File path", "Line number", "Function name" and "Description".
 */
void lv_debug_cb(lv_log_level_t level, const char * path, uint32_t line, const char * function, const char * desc)
{
    //we have same level enumeration

    if (level == LV_LOG_LEVEL_ERROR)
        bsod_msg("LVGL error\n %s:%lu (%s) %s", path, line, function, desc);
    else
        debug_send(level, "[LVGL] %s:%lu (%s) %s", path, line, function, desc);
}

void gui_clean_dcache(struct _disp_drv_t * disp_drv)
{

}

void gui_take_screenshot()
{
    gui.take_screenshot = 1;
}

void gui_print_memory()
{
    lv_mem_monitor_t mem;
    lv_mem_monitor(&mem);

    INFO("=== LVGL memory ===");
    INFO("%u %% used  %lu/%lu (free/total)", mem.used_pct, mem.free_size, mem.total_size);
    INFO("slots %lu/%lu", mem.used_cnt, mem.free_cnt);
    INFO("watermark %lu", mem.max_used);
    INFO("biggest free %lu", mem.free_biggest_size);
    INFO("fragmentation %u%%", mem.frag_pct);
    INFO("mem test %u", lv_mem_test());
    INFO("");
}

static TaskHandle_t gui_lock_owner = NULL;

void gui_lock_acquire()
{
    while(gui.lock == NULL)
    {
        osDelay(10);
    }

	if (xTaskGetCurrentTaskHandle() != thread_gui)
	{
	    uint32_t start = HAL_GetTick();


	    uint32_t wait = (gui_lock_owner == NULL) ? WAIT_INF : 3000;

        TaskHandle_t prev_lock_owner = gui_lock_owner;

//        osStatus_t stat = osSemaphoreAcquire(gui.lock, wait);
        osStatus_t stat = osMutexAcquire(gui.lock, wait);
	    if (stat == osErrorTimeout)
	    {
	        bsod_msg("Not able to acquire gui.lock in time from task '%s' blocked by task '%s'!",
	                pcTaskGetName(xTaskGetCurrentTaskHandle()), pcTaskGetName(gui_lock_owner));
	    }
	    uint32_t delta = HAL_GetTick() - start;
	    if (delta > 100 && prev_lock_owner != NULL)
	    {
	        WARN("'%s' was unable to acquire gui lock from '%s' for %u ms!",
                    pcTaskGetName(xTaskGetCurrentTaskHandle()), pcTaskGetName(prev_lock_owner), delta);
	    }
        gui_lock_owner = xTaskGetCurrentTaskHandle();
	}
}

void gui_lock_release()
{
	if (xTaskGetCurrentTaskHandle() != thread_gui)
	{

        gui_lock_owner = NULL;
        osMutexRelease(gui.lock);
	}
}

void gui_create_lock()
{
    //Create lock for lvgl
//    gui.lock = osSemaphoreNew(1, 0, NULL);
    gui.lock = NULL;
    //lock is active on create
}

void gui_unlock_cb()
{
    osMutexRelease(gui.lock);
    //Force unlock GUI thread
    osMutexAcquire(gui.lock, WAIT_INF);
}

void gui_disp_wait_cb(struct _disp_drv_t * disp_drv)
{
    osMutexRelease(gui.lock);
    tft_wait_to_finish_dma();
    osMutexAcquire(gui.lock, WAIT_INF);
}

//restricts gui drawing performance
void gui_low_priority(bool val)
{
    gui.restrict_gui_speed = val;
}

void thread_gui_start(void *argument)
{
    UNUSED(argument);

    system_wait_for_handle(&thread_gui);

    gui.lock = osMutexNew(NULL);
    osMutexAcquire(gui.lock, WAIT_INF);
    vQueueAddToRegistry(gui.lock, "gui.lock");

	INFO("Started");

    //Littlvgl init
	lv_log_register_print_cb(lv_debug_cb);
	lv_init();

	//bck light
	led_init();
	led_set_backlight(config_get_int(&config.display.backlight));

	//display init
	tft_init();

	//Display driver glue
	static lv_disp_buf_t disp_buf;
	lv_disp_drv_t disp_drv;

	lv_disp_buf_init(&disp_buf, tft_buffer_1, tft_buffer_2, TFT_BUFFER_SIZE);
	lv_disp_drv_init(&disp_drv);

	disp_drv.hor_res = TFT_WIDTH;
	disp_drv.ver_res = TFT_HEIGHT;
	disp_drv.flush_cb = gui_disp_flush_cb;
	disp_drv.wait_cb = gui_disp_wait_cb;
	disp_drv.buffer = &disp_buf;

	disp_drv.clean_dcache_cb = gui_clean_dcache;

	lv_disp_drv_register(&disp_drv);


	//Input group for navigation
	gui.input.group = lv_group_create();

	lv_indev_drv_t indev_drv_enc;
	lv_indev_drv_init(&indev_drv_enc);
	indev_drv_enc.type = LV_INDEV_TYPE_ENCODER;
	indev_drv_enc.read_cb = gui_input_cb;

	gui.input.indev = lv_indev_drv_register(&indev_drv_enc);
    lv_indev_set_group(gui.input.indev, gui.input.group);

    //Input group for modal dialog
    gui.dialog.group = lv_group_create();

    gui_init();

    INFO("GUI ready");

    osMutexRelease(gui.lock);

    uint32_t delay;
    uint32_t last_tick = 0;
    uint32_t frame_duration = 0;
    uint8_t frame_counter = 0;

    lv_task_create(gui_unlock_cb, 200, LV_TASK_PRIO_HIGHEST, NULL);

    while (!system_power_off)
	{
		gui_loop();

		if (gui.take_screenshot == 1)
		{
		    gui.take_screenshot = 2;
		    lv_obj_invalidate(lv_scr_act());
		    lv_refr_now(NULL);
		    gui.take_screenshot = 0;
		}

		osMutexAcquire(gui.lock, WAIT_INF);
		gui_lock_owner = xTaskGetCurrentTaskHandle();

		if (gui.injected_function != NULL)
		{
		    gui.injected_function();
		    gui.injected_function = NULL;
		}

		delay = lv_task_handler();

		gui_lock_owner = NULL;
		osMutexRelease(gui.lock);

		//do not starve idle, allow more time for processing
		uint16_t stay_idle_time = gui.restrict_gui_speed ? 100 : 1;

		osDelay(max(stay_idle_time, delay));

		if (config_get_bool(&config.debug.lvgl_info))
		{
            uint32_t now = HAL_GetTick();
            frame_duration += now - last_tick;
            last_tick = now;
            frame_counter++;

            if (frame_counter == FPS_SAMPLES)
            {
                gui.fps = 1000 / max(1, (frame_duration / FPS_SAMPLES));
                frame_counter = 0;
                frame_duration = 0;
            }
		}
	}

    gui_stop();

    INFO("Done");
    osThreadSuspend(thread_gui);
}
