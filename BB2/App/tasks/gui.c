/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "../common.h"
#include "../lib/lvgl/lvgl.h"

#include "../drivers/tft_hx8352.h"
#include "../drivers/led.h"

#include "../gui/gui.h"
#include "../config/config.h"

#include "../lib/lvgl/src/lv_gpu/lv_gpu_stm32_dma2d.h"

#define GUI_BUTTON_CNT	5
uint8_t gui_button_index = 0;


/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_flush_ready()' has to be called when finished
 * This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
void gui_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t * color_p)
{
	tft_refresh_buffer(area->x1, area->y1, area->x2, area->y2);
	lv_disp_flush_ready(&gui.disp_drv);
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

	uint8_t key_map[] = {LV_KEY_RIGHT, LV_KEY_HOME, LV_KEY_ENTER, LV_KEY_ESC, LV_KEY_LEFT};

	if (last_key_state == LV_INDEV_STATE_REL)
	{
		for (uint8_t i = 0; i < 5; i++)
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

void task_GUI(void *argument)
{
	vTaskSuspend(NULL);

	INFO("Started");

	lv_log_register_print_cb(lv_debug_cb);
	lv_init();

	led_init();

	led_set_backlight(config_get_int(&config.settings.display.backlight));

	//display init
	tft_init();
	tft_init_display();

//	tft_test_pattern();
//	tft_refresh_buffer(0, 0, 240, 400);

	//Littlvgl init

	//Display driver glue
	static lv_disp_buf_t disp_buf;

	lv_disp_buf_init(&disp_buf, tft_buffer, NULL, TFT_BUFFER_SIZE);
	lv_disp_drv_init(&gui.disp_drv);

	gui.disp_drv.hor_res = TFT_WIDTH;
	gui.disp_drv.ver_res = TFT_HEIGHT;
	gui.disp_drv.flush_cb = gui_disp_flush;
	gui.disp_drv.buffer = &disp_buf;

	gui.disp_drv.clean_dcache_cb = gui_clean_dcache;


	lv_disp_drv_register(&gui.disp_drv);


	//Input group for navigation
	gui.input.nav = lv_group_create();

	lv_indev_drv_t indev_drv_enc;
	lv_indev_drv_init(&indev_drv_enc);
	indev_drv_enc.type = LV_INDEV_TYPE_ENCODER;
	indev_drv_enc.read_cb = gui_input_cb;

	lv_indev_t * nav_indev = lv_indev_drv_register(&indev_drv_enc);
    lv_indev_set_group(nav_indev, gui.input.nav);

    //Input group for keys
	gui.input.keypad = lv_group_create();

	lv_indev_drv_t indev_drv_keypad;
	lv_indev_drv_init(&indev_drv_keypad);
	indev_drv_keypad.type = LV_INDEV_TYPE_KEYPAD;
	indev_drv_keypad.read_cb = gui_input_cb;

	lv_indev_t * key_indev = lv_indev_drv_register(&indev_drv_keypad);
    lv_indev_set_group(key_indev, gui.input.keypad);

    gui_init();

    uint32_t delay;

    while (!system_power_off)
	{
		gui_loop();

		delay = lv_task_handler();
		osDelay(delay);
	}

    INFO("Done");
    vTaskSuspend(NULL);
}
