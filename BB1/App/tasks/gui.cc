/*
 * gui.cc
 *
 *  Created on: Apr 23, 2020
 *      Author: horinek
 */

#include "../common.h"
#include "../lib/lvgl/lvgl.h"

#include "../drivers/tft_hx8352.h"

#include "../gui/gui.h"

extern "C" void task_GUI(void *argument);


static lv_disp_drv_t disp_drv;

#define GUI_BUTTON_CNT	5
uint8_t gui_button_index = 0;


/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_flush_ready()' has to be called when finished
 * This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
void gui_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t * color_p)
{
	tft_refresh_buffer(area->x1, area->y1, area->x2, area->y2);
	lv_disp_flush_ready(&disp_drv);
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

void task_GUI(void *argument)
{
	vTaskSuspend(NULL);

	INFO("Started");

	lv_init();

	HAL_TIM_Base_Start(&tim_rgb_and_bck);
	HAL_TIM_PWM_Start(&tim_rgb_and_bck, tim_bckl_channel);

	//display init
	tft_init();
	tft_init_display();

//	tft_test_pattern();
//	tft_refresh_buffer(0, 0, 240, 400);

	//Littlvgl init

	//Display driver glue
	static lv_disp_buf_t disp_buf;

	lv_disp_buf_init(&disp_buf, tft_buffer, NULL, TFT_BUFFER_SIZE);
	lv_disp_drv_init(&disp_drv);

	disp_drv.hor_res = TFT_WIDTH;
	disp_drv.ver_res = TFT_HEIGHT;
	disp_drv.flush_cb = gui_disp_flush;
	disp_drv.buffer = &disp_buf;

	lv_disp_drv_register(&disp_drv);


	//Keyboard input glue
	gui_group = lv_group_create();

	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_ENCODER;
	indev_drv.read_cb = gui_input_cb;

	lv_indev_t * key_indev = lv_indev_drv_register(&indev_drv);
    lv_indev_set_group(key_indev, gui_group);

    gui_init();

    uint32_t delay;

	for(;;)
	{
		gui_loop();

		delay = lv_task_handler();
		osDelay(delay);
	}
}
