#include "vario.h"

#include "../protocol.h"

uint32_t * vario_buffer;
//100 ms buffer
#define VARIO_BUFFER_SIZE	((OUTPUT_SAMPLERATE / 10) * sizeof(int16_t))

static tone_part_t * to_remove = NULL;

static tone_part_t ** tones = NULL;
static uint8_t tone_index = 0;
static uint8_t tone_cnt = 0;

tone_part_t * tone_active = NULL;

static uint16_t tone_position = 0;
static uint16_t tone_repeates = 0;


static uint16_t * one_ms_silent;

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

	while (size > 0)
	{
		if (tone_position >= tone_active->size || tone_active->size == 0)
		{
			tone_position = 0;
			tone_repeates++;

			if (tone_repeates >= tone_active->repeat)
			{
				tone_index = (tone_index + 1) % tone_cnt;
				if (tone_active == to_remove)
				{
					vario_tone_free(to_remove);
					to_remove = NULL;
					tone_index = 0;
				}

				tone_active = tones[tone_index];
				tone_repeates = 0;
			}
		}

		uint16_t to_write = min(tone_active->size - tone_position, size);
		raw_stream_write(pipes.vario.stream, (char *)(tone_active->buffer + tone_position), to_write * sizeof(uint16_t));

		size -= to_write;
		tone_position = (tone_position + to_write);
	}

	xSemaphoreGive(pipes.vario.lock);
}

//#define HALF_AMP 32767
#define HALF_AMP 16384

uint16_t * vario_generate_tone(uint16_t freq, uint16_t * len)
{
	*len = OUTPUT_SAMPLERATE / freq;
	int16_t * buff = ps_malloc(*len * sizeof(uint16_t));

	for (uint16_t i = 0; i < *len; i++)
	{
		buff[i] = (sin((M_TWOPI * i) / *len) * HALF_AMP);
	}

	return (uint16_t *)buff;
}

#define ONE_MS	(OUTPUT_SAMPLERATE / 1000)

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
		tone->buffer = vario_generate_tone(freq, &tone->size);
		tone->repeat = (duration * ONE_MS) / tone->size;
		ASSERT(tone->repeat > 0);
	}

	return tone;
}

void pipe_vario_replace(tone_part_t ** new_tones, uint8_t cnt)
{
	while(to_remove != NULL);

	xSemaphoreTake(pipes.vario.lock, WAIT_INF);

	for (uint8_t i = 0; i < tone_cnt; i++)
	{
		if (tones[i] != tone_active)
		{
			vario_tone_free(tones[i]);
		}
		else
		{
			//gracefully remove tone
			to_remove = tones[i];

			if (tone_active->buffer == one_ms_silent)
			{
				//silent tone, skip
				tone_position = tone_active->size;
			}
			else
			{
				//finish wave
				tone_repeates = tone_active->repeat;
			}
		}
	}

	free(tones);
	
	tones = new_tones;
	tone_cnt = cnt;

	xSemaphoreGive(pipes.vario.lock);
}

void pipe_vario_loop()
{
	//ps malloc zero out the allocated memeory
	one_ms_silent = ps_malloc(ONE_MS * sizeof(int16_t));

	tones = malloc(sizeof(tone_part_t *) * 1);

	tones[0] = vario_create_part(0, 100);

	tone_active = tones[0];
	tone_index = 0;
	tone_cnt = 1;

	tone_position = 0;
	tone_repeates = 0;

	while (1)
	{
		pipe_vario_step();
		vTaskDelay(50 / portTICK_PERIOD_MS);
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

    xTaskCreate(pipe_vario_loop, "pipe_vario_loop", 1024 * 2, NULL, 12, NULL);

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



