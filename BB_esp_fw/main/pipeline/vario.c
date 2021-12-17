#define DEBUG_LEVEL DBG_DEBUG
#include "vario.h"

#include "../protocol.h"

uint32_t * vario_buffer;
//100 ms buffer
#define VARIO_BUFFER_SIZE	((OUTPUT_SAMPLERATE / 10) * 2 * sizeof(int16_t))

static tone_part_t * to_remove = NULL;

static tone_part_t ** tones = NULL;
static uint8_t tone_index = 0;
static uint8_t tone_cnt = 0;

tone_part_t * tone_active = NULL;

static uint16_t tone_position = 0;
static uint16_t tone_repeates = 0;


static int16_t * one_ms_silent;

void vario_tone_free(tone_part_t * tone)
{
	if (tone->buffer != one_ms_silent)
		free(tone->buffer);
	free(tone);
}

void pipe_vario_step()
{
	xSemaphoreTake(pipes.vario.lock, WAIT_INF);

	ringbuf_handle_t rb = audio_element_get_output_ringbuf(pipes.sound.stream);
	uint16_t size = rb_bytes_available(rb) / sizeof(uint16_t);

	if (tone_active != NULL)
	{
		while (size > 0)
		{
			//active tone is marked for removal
			if (tone_active == to_remove)
			{
				//actual tone is silent or wave is finished
				if (tone_active->buffer == one_ms_silent || tone_position >= tone_active->size)
				{
					vario_tone_free(to_remove);
					to_remove = NULL;

					if (tone_cnt > 0)
					{
						tone_active = tones[tone_index];
						tone_position = 0;
					}
					else
					{
						tone_active = NULL;
						break;
					}
				}
			}

			//one repetition of tone is done
			if (tone_position >= tone_active->size)
			{
				tone_position = 0;
				tone_repeates++;

				//all repetitions are done
				if (tone_repeates >= tone_active->repeat)
				{
					//select new tone
					if (tone_cnt > 0)
					{
						tone_index = (tone_index + 1) % tone_cnt;
						tone_active = tones[tone_index];
						tone_repeates = 0;
					}
					else
					{
						tone_active = NULL;
						break;
					}
				}
			}

			uint16_t to_write = min(tone_active->size - tone_position, size);
			raw_stream_write(pipes.vario.stream, (char *)(tone_active->buffer + tone_position), to_write * sizeof(uint16_t));

			size -= to_write;
			tone_position = (tone_position + to_write);
		}
	}

	xSemaphoreGive(pipes.vario.lock);
}

#define HALF_AMP (32767/2)

uint16_t vario_tone_lenght(uint16_t freq)
{
	return (OUTPUT_SAMPLERATE / freq) * 2;
}

tone_part_t * vario_create_part(uint16_t freq, uint16_t duration)
{
	tone_part_t * tone = ps_malloc(sizeof(tone_part_t));

	if (freq == 0)
	{
		tone->buffer = one_ms_silent;
		tone->repeat = duration;
		tone->size = ONE_MS;
	}
	else
	{
		tone->size = vario_tone_lenght(freq);
		tone->buffer = ps_malloc(tone->size * sizeof(int16_t));
		uint16_t one_ch_len = tone->size / 2;

		for (uint16_t i = 0; i < one_ch_len; i++)
		{
			tone->buffer[i*2] = (int16_t)(sin((M_TWOPI * i) / one_ch_len) * HALF_AMP);
		}

		tone->repeat = max(1, (duration * ONE_MS) / one_ch_len);
	}

	return tone;
}

tone_part_t * vario_create_part_fade(uint16_t freq, uint16_t duration, bool down)
{
	tone_part_t * tone = ps_malloc(sizeof(tone_part_t));

	if (freq == 0)
	{
		tone->buffer = one_ms_silent;
		tone->repeat = duration;
		tone->size = ONE_MS;
	}
	else
	{
		uint16_t wave_len = vario_tone_lenght(freq);
		uint16_t wave_cnt = (duration * ONE_MS) / wave_len;
		tone->size = wave_len * wave_cnt;

		tone->buffer = ps_malloc(tone->size * sizeof(int16_t));
		uint16_t one_ch_len = tone->size / 2;
		for (uint16_t i = 0; i < one_ch_len; i++)
		{
			int16_t amp = HALF_AMP;

			if (down)
			{
				amp = (amp * (one_ch_len - i - 1)) / one_ch_len;
			}
			else
			{
				amp = (amp * i) / one_ch_len;
			}

			tone->buffer[i*2] = (int16_t)(sin((M_TWOPI * (i % wave_len)) / wave_len) * amp);
		}

		tone->repeat = 1;
	}

	return tone;
}

void pipe_vario_replace(tone_part_t ** new_tones, uint8_t cnt)
{
	//if tone replacement is pending wait
	while(to_remove != NULL)
	{
		taskYIELD();
	}

	xSemaphoreTake(pipes.vario.lock, WAIT_INF);

	//get last position in tone
	uint32_t actual_pos = 0;
	for (uint8_t i = 0; i < tone_cnt; i++)
	{
		if (tones[i] != tone_active)
		{
			if (to_remove == NULL)
				actual_pos += tones[i]->size * tones[i]->repeat;

			vario_tone_free(tones[i]);
		}
		else
		{
			//one wave is in progress, replacement will occur after wave is done
			actual_pos += tones[i]->size * (tone_repeates + 1);

			//gracefully remove tone in next buffer write
			to_remove = tones[i];
		}
	}

	//free tones
	if (tones != NULL)
	{
		free(tones);
	}
	
	//assign new tones
	tones = new_tones;
	tone_cnt = cnt;

	//fallback if new tone is shorter then last one
	//start new tone from the beginning
	tone_index = 0;
	tone_repeates = 0;

	//set same position in new tone
	uint32_t total_duration = 0;
	for (uint8_t i = 0; i < cnt; i++)
	{
		uint32_t duration = tones[i]->size * tones[i]->repeat;

		if (total_duration + duration > actual_pos)
		{
			tone_index = i;
			tone_repeates = (actual_pos - total_duration) / tones[i]->size;
			break;
		}
		else
		{
			total_duration += duration;
		}
	}

	if (tone_active == NULL)
	{
		tone_active = tones[tone_index];
		tone_position = 0;
	}

	xSemaphoreGive(pipes.vario.lock);

	protocol_send(PROTO_TONE_ACK, NULL, 0);
}

#define FADE 10 //in ms

void vario_create_sequence(tone_pair_t * pairs, uint8_t cnt)
{
	if (cnt == 0)
	{
		pipe_vario_replace(NULL, 0);
		return;
	}

	tone_part_t ** new_tones = ps_malloc(sizeof(tone_part_t *) * cnt * 2);
	uint8_t index = 0;
	for (uint8_t i = 0; i < cnt; i++)
	{
		uint16_t next_tone = pairs[(i + 1) % cnt].tone;
		uint16_t tone = pairs[i].tone;
		uint16_t dura = pairs[i].dura;

		bool fade = false;
		bool down;

		if ((tone > 0 && next_tone == 0) || (tone == 0 && next_tone > 0))
		{
			fade = true;
			down = tone > 0;
			dura = max(0, dura - FADE);
		}

		new_tones[index++] = vario_create_part(tone, dura);

		if (fade)
		{
			new_tones[index++] = vario_create_part_fade((down) ? tone : next_tone, FADE, down);
		}
	}

	pipe_vario_replace(new_tones, index);
}

void pipe_vario_loop()
{
	//ps malloc zero out the allocated memeory
	one_ms_silent = ps_malloc(ONE_MS * sizeof(int16_t));

	tones = malloc(sizeof(tone_part_t *) * 1);

//	tones[0] = vario_create_part(0, 1);

	tone_active = NULL;//tones[0];
	tone_index = 0;
	tone_cnt = 0;

	tone_position = 0;
	tone_repeates = 0;

	while (1)
	{
		pipe_vario_step();
		task_sleep(50);
	}
}

void pipe_vario_init()
{
    INFO("pipe_vario_init");

    pipes.vario.lock = xSemaphoreCreateBinary();

    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipes.vario.pipe = audio_pipeline_init(&pipeline_cfg);

    raw_stream_cfg_t raw_cfg = RAW_STREAM_CFG_DEFAULT();
    raw_cfg.type = AUDIO_STREAM_WRITER;
    raw_cfg.out_rb_size = VARIO_BUFFER_SIZE;
    pipes.vario.stream = raw_stream_init(&raw_cfg);

    raw_cfg.type = AUDIO_STREAM_WRITER;
    pipes.vario.raw = raw_stream_init(&raw_cfg);

    audio_pipeline_register(pipes.vario.pipe, pipes.vario.stream, "stream");
    audio_pipeline_register(pipes.vario.pipe, pipes.vario.raw, "raw");

    const char *link[2] = {"stream", "raw"};
    audio_pipeline_link(pipes.vario.pipe, &link[0], 2);

    downmix_set_input_rb(pipes.output.mix, audio_element_get_input_ringbuf(pipes.vario.raw), SOURCE_VARIO_INDEX);
    downmix_set_source_stream_info(pipes.output.mix, OUTPUT_SAMPLERATE, OUTPUT_CHANNELS, SOURCE_VARIO_INDEX);
    audio_pipeline_set_listener(pipes.vario.pipe, pipes.events);

    audio_pipeline_run(pipes.vario.pipe);

    xTaskCreate(pipe_vario_loop, "pipe_vario_loop", 512 * 3, NULL, 20, NULL);

    xSemaphoreGive(pipes.vario.lock);
}

void pipe_vario_event(audio_event_iface_msg_t * msg)
{
	audio_element_msg_cmd_t cmd = msg->cmd;
//	INFO("pipe_vario_event %u", cmd);

	if (cmd == AEL_MSG_CMD_REPORT_STATUS)//8
	{
//	    AEL_STATUS_NONE                     = 0,
//	    AEL_STATUS_ERROR_OPEN               = 1,
//	    AEL_STATUS_ERROR_INPUT              = 2,
//	    AEL_STATUS_ERROR_PROCESS            = 3,
//	    AEL_STATUS_ERROR_OUTPUT             = 4,
//	    AEL_STATUS_ERROR_CLOSE              = 5,
//	    AEL_STATUS_ERROR_TIMEOUT            = 6,
//	    AEL_STATUS_ERROR_UNKNOWN            = 7,
//	    AEL_STATUS_INPUT_DONE               = 8,
//	    AEL_STATUS_INPUT_BUFFERING          = 9,
//	    AEL_STATUS_OUTPUT_DONE              = 10,
//	    AEL_STATUS_OUTPUT_BUFFERING         = 11,
//	    AEL_STATUS_STATE_RUNNING            = 12,
//	    AEL_STATUS_STATE_PAUSED             = 13,
//	    AEL_STATUS_STATE_STOPPED            = 14,
//	    AEL_STATUS_STATE_FINISHED           = 15,
//	    AEL_STATUS_MOUNTED                  = 16,
//	    AEL_STATUS_UNMOUNTED                = 17,
	}
}

void vario_proces_packet(proto_tone_play_t * packet)
{
	//DBG("Packet id = %u", packet->id);

	tone_pair_t * pairs = NULL;

	if (packet->size != 0)
	{
		pairs = ps_malloc(sizeof(tone_pair_t *) * packet->size);

		for (uint8_t i = 0; i < packet->size; i++)
		{
			pairs[i].dura = packet->dura[i];
			pairs[i].tone = packet->freq[i];
		}
	}

	vario_create_sequence(pairs, packet->size);

	if (pairs != NULL)
	{
		free(pairs);
	}

	free(packet);
	vTaskDelete(NULL);
}


