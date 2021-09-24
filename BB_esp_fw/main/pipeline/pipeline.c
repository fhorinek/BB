/*
 * pipeline.c
 *
 *  Created on: 19. 1. 2021
 *      Author: horinek
 */

#include "pipeline.h"

#include "bluetooth.h"
#include "sound.h"
#include "output.h"
#include "vario.h"

pipelines_t pipes;

void pipeline_loop(void *pvParameters)
{
	while (1)
	{
		audio_event_iface_msg_t msg;
		esp_err_t ret = audio_event_iface_listen(pipes.events, &msg, WAIT_INF);

		if (ret != ESP_OK)
		{
			INFO("[ * ] Event interface error : %d", ret);
			continue;
		}

//		if (msg.cmd == AEL_MSG_CMD_ERROR)
//		{
//			INFO("[ * ] Action command: src_type:%d, source:%p cmd:%d, data:%p, data_len:%d",
//					 msg.source_type, msg.source, msg.cmd, msg.data, msg.data_len);
//
//			continue;
//		}

		if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT)
		{
			if (msg.source == (void *) pipes.bluetooth.bt)
			{
				pipe_bluetooth_event(&msg);
				continue;
			}
			if (msg.source == (void *) pipes.sound.decoder)
			{
				pipe_sound_event(&msg);
				continue;
			}
			if (msg.source == (void *) pipes.vario.stream)
			{
				pipe_vario_event(&msg);
				continue;
			}
		}
	}

	vTaskDelete(NULL);
}


void pipeline_monitor()
{
	uint16_t a2dp = rb_bytes_available(audio_element_get_output_ringbuf(pipes.bluetooth.bt));
	uint16_t filter = rb_bytes_available(audio_element_get_output_ringbuf(pipes.bluetooth.filter));

	uint16_t vario = rb_bytes_available(audio_element_get_output_ringbuf(pipes.vario.stream));

	uint16_t sound = rb_bytes_available(audio_element_get_output_ringbuf(pipes.sound.stream));

	uint16_t mix = rb_bytes_available(audio_element_get_output_ringbuf(pipes.output.mix));

	INFO("BT: %u->%u VA %u SN %u OUT %u", a2dp, filter, vario, sound, mix);
}

void pipeline_init()
{
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    pipes.events = audio_event_iface_init(&evt_cfg);

	pipe_output_init();
	print_free_memory("pipe_output_init");

	pipe_sound_init();
	print_free_memory("pipe_sound_init");

//	pipe_bluetooth_init();
//	print_free_memory("pipe_bluetooth_init");

	pipe_vario_init();
	print_free_memory("pipe_vario_init");

	xTaskCreate(pipeline_loop, "pipeline_loop", 512 * 3, NULL, 15, NULL);
}

