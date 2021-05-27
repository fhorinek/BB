/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "gui_thread.h"
#include "gui.h"

#include "map/map_thread.h"

#include "lib/lvgl/lvgl.h"
#include "lib/lvgl/src/lv_gpu/lv_gpu_stm32_dma2d.h"

#include "drivers/tft/tft.h"
#include "drivers/power/led.h"

void gui_save_screen()
{
    static uint16_t index = 0;
    FIL f;
    char path[32];
    FRESULT res;

    f_mkdir(PATH_SCREENSHOT);

    do
    {
        sprintf(path, "%s/%08u.scr", PATH_SCREENSHOT, index);
        index++;
        res = f_open(&f, path, FA_WRITE | FA_CREATE_NEW);
    } while (res != FR_OK);

    INFO("Saving screenshot to %s", path);

    UINT bw;
    f_write(&f, tft_buffer, TFT_BUFFER_SIZE * sizeof(uint16_t), &bw);
    f_close(&f);

    gui.take_screenshot = 0;
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_flush_ready()' has to be called when finished
 * This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
void gui_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t * color_p)
{
    if (gui.take_screenshot == 2)
    {
        gui_save_screen();
    }
    else
    {
        tft_refresh_buffer(area->x1, area->y1, area->x2, area->y2);
    }

	lv_disp_flush_ready(drv);
}

bool gui_get_button_state(uint8_t index)
{
	GPIO_TypeDef* button_port[] = {BT1_GPIO_Port, BT2_GPIO_Port, BT3_GPIO_Port, BT4_GPIO_Port, BT5_GPIO_Port};
	uint16_t button_pin[] = {BT1_Pin, BT2_Pin, BT3_Pin, BT4_Pin, BT5_Pin};

	return (HAL_GPIO_ReadPin(button_port[index], button_pin[index]) == LOW);
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
	debug_send(level, "[LVGL] %s:%lu (%s) %s", path, line, function, desc);
}

void gui_clean_dcache(struct _disp_drv_t * disp_drv)
{

}

void gui_take_screenshot()
{
    gui.take_screenshot = 1;
}

void gui_lock_acquire()
{
    osSemaphoreAcquire(gui.lock, WAIT_INF);
}

void gui_lock_release()
{
    osSemaphoreRelease(gui.lock);
}

void thread_gui_start(void *argument)
{
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

	lv_disp_buf_init(&disp_buf, tft_buffer, NULL, TFT_BUFFER_SIZE);
	lv_disp_drv_init(&disp_drv);

	disp_drv.hor_res = TFT_WIDTH;
	disp_drv.ver_res = TFT_HEIGHT;
	disp_drv.flush_cb = gui_disp_flush;
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

    //Create lock for lvgl
    gui.lock = osSemaphoreNew(1, 0, NULL);
    vQueueAddToRegistry(gui.lock, "gui.lock");
    gui_lock_release();

    start_thread(thread_map);

    gui_init();

    uint32_t delay;

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

		gui_lock_acquire();
		delay = lv_task_handler();
		gui_lock_release();
		osDelay(delay);
	}

    INFO("Done");
    osThreadSuspend(thread_gui);
}
