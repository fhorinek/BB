#include "sound.h"

#include "wav_decoder.h"
#include "../protocol.h"

void pipe_sound_request_next();

void pipe_sound_loop(void * arg)
{
	while(1)
	{
		pipe_sound_request_next();
		vTaskDelay(100/ portTICK_PERIOD_MS);
	}
}

void pipe_sound_init()
{
    INFO("pipe_sound_init");

    pipes.sound.lock = xSemaphoreCreateBinary();

    pipes.sound.total_len = 0;
	pipes.sound.request = false;

    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipes.sound.pipe = audio_pipeline_init(&pipeline_cfg);

    raw_stream_cfg_t raw_cfg = RAW_STREAM_CFG_DEFAULT();
    raw_cfg.type = AUDIO_STREAM_WRITER;
    raw_cfg.out_rb_size = 24 * 1024;
    pipes.sound.stream = raw_stream_init(&raw_cfg);

    wav_decoder_cfg_t wav_cfg = DEFAULT_WAV_DECODER_CONFIG();
    pipes.sound.decoder = wav_decoder_init(&wav_cfg);

    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
        rsp_cfg.src_rate = 16000;
        rsp_cfg.src_ch = 1;
        rsp_cfg.dest_rate = OUTPUT_SAMPLERATE;
        rsp_cfg.dest_ch = OUTPUT_CHANNELS;
	pipes.sound.filter = rsp_filter_init(&rsp_cfg);


    raw_cfg.type = AUDIO_STREAM_WRITER;
    pipes.sound.raw = raw_stream_init(&raw_cfg);

    audio_pipeline_register(pipes.sound.pipe, pipes.sound.stream, "stream");
    audio_pipeline_register(pipes.sound.pipe, pipes.sound.decoder, "decoder");
    audio_pipeline_register(pipes.sound.pipe, pipes.sound.filter, "filter");
    audio_pipeline_register(pipes.sound.pipe, pipes.sound.raw, "raw");

    const char *link[4] = {"stream", "decoder", "filter", "raw"};
    audio_pipeline_link(pipes.sound.pipe, &link[0], 4);

    downmix_set_input_rb(pipes.output.mix, audio_element_get_input_ringbuf(pipes.sound.raw), SOURCE_SOUND_INDEX);
    downmix_set_source_stream_info(pipes.output.mix, OUTPUT_SAMPLERATE, OUTPUT_CHANNELS, SOURCE_SOUND_INDEX);
    audio_pipeline_set_listener(pipes.sound.pipe, pipes.events);

    xSemaphoreGive(pipes.sound.lock);

    xTaskCreate(pipe_sound_loop, "pipe_sound_loop", 2 * 1024, NULL, 20, NULL);
}

void pipe_sound_event(audio_event_iface_msg_t * msg)
{
	audio_element_msg_cmd_t cmd = msg->cmd;
	INFO("pipe_sound_event %u", cmd);

	if (cmd == AEL_MSG_CMD_REPORT_POSITION)//11
	{
		audio_element_info_t music_info = {0};
		audio_element_getinfo(pipes.sound.decoder, &music_info);

		INFO("Position %u / %u", music_info.byte_pos, music_info.total_bytes);
	}

	if (cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO)//9
	{
		audio_element_info_t music_info = {0};
		audio_element_getinfo(pipes.sound.decoder, &music_info);

		INFO("Receive music info from Decoder, sample_rates=%d, bits=%d, ch=%d", music_info.sample_rates, music_info.bits, music_info.channels);

		audio_element_setinfo(pipes.sound.filter, &music_info);
	}

	if (cmd == AEL_MSG_CMD_REPORT_STATUS)//8
	{
		INFO("Status from decoder = %u", msg->data);

		uint8_t type = (uint32_t)msg->data;

		if (type == AEL_STATUS_STATE_FINISHED)
		{
			INFO("sound finished");

			xSemaphoreTake(pipes.sound.lock, WAIT_INF);
            pipe_sound_reset();
            xSemaphoreGive(pipes.sound.lock);
		}

		if (type == AEL_STATUS_STATE_STOPPED)
		{
			INFO("sound stopped");
//			audio_pipeline_terminate(pipes.sound.pipe);

		}

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

#define SOUND_PACKET_SIZE	(1024 * 4)

void pipe_sound_request_next()
{
	xSemaphoreTake(pipes.sound.lock, WAIT_INF);

	if (pipes.sound.id > 0	//sound started
			&& (pipes.sound.total_len - pipes.sound.pos) > 0 //not reached the end
			&& !pipes.sound.request) //request is not pending
	{
		ringbuf_handle_t rb = audio_element_get_output_ringbuf(pipes.sound.stream);
		uint16_t size = rb_bytes_available(rb);

		if (size >= SOUND_PACKET_SIZE || (pipes.sound.total_len - pipes.sound.pos) <= size)
		{
			INFO("sending request for new data id %u (space avalible %u)", pipes.sound.id, size);
			pipes.sound.request = true;
			protocol_send_sound_reg_more(pipes.sound.id, SOUND_PACKET_SIZE);
		}
	}

	xSemaphoreGive(pipes.sound.lock);
}

void pipe_sound_write(uint8_t id, uint8_t * buf, uint16_t len)
{
	xSemaphoreTake(pipes.sound.lock, WAIT_INF);

	if (id == pipes.sound.id)
	{
		INFO("pipe_sound_write %u %u + %u", id, pipes.sound.pos, len);

		if (len > 0)
		{
			pipes.sound.pos += len;
			raw_stream_write(pipes.sound.stream, (char *)buf, len);

			if (pipes.sound.pos >= pipes.sound.total_len)
			{
				//this was last data chunk
				audio_element_set_ringbuf_done(pipes.sound.stream);
			}
		}
	}
	else
	{
		WARN("Wrong id %u != %u", id, pipes.sound.id);
	}

	pipes.sound.request = false;

    xSemaphoreGive(pipes.sound.lock);
}

void pipe_sound_start(uint8_t id, uint8_t type, uint32_t len)
{
    INFO("pipe_sound_start %u %u", id, len);

    xSemaphoreTake(pipes.sound.lock, WAIT_INF);

	//old sound is playing, stop
	if (pipes.sound.id > 0)
	{
		pipe_sound_stop();
	}

    pipes.sound.id = id;

    audio_pipeline_run(pipes.sound.pipe);

    pipes.sound.total_len = len;
    pipes.sound.pos = 0;

    xSemaphoreGive(pipes.sound.lock);
}

//must be run from locked context
void pipe_sound_reset()
{
	INFO("pipe_sound_reset");

	//invalidate sound id
	pipes.sound.id = 0;
	audio_pipeline_reset_ringbuffer(pipes.sound.pipe);
	audio_pipeline_reset_elements(pipes.sound.pipe);
	audio_pipeline_change_state(pipes.sound.pipe, AEL_STATE_INIT);
}

//must be run from locked context
void pipe_sound_stop()
{
	INFO("pipe_sound_stop");
	audio_pipeline_stop(pipes.sound.pipe);
	audio_pipeline_wait_for_stop(pipes.sound.pipe);

	pipe_sound_reset();
}


