#include "bluetooth.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

void pipe_bluetooth_init()
{
    INFO("pipe_bluetooth_init");
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BTDM));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    esp_bt_dev_set_device_name("Strato");
    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipes.bluetooth.pipe = audio_pipeline_init(&pipeline_cfg);

    a2dp_stream_config_t a2dp_config = {
        .type = AUDIO_STREAM_READER,
        .user_callback = {0},
    };
    pipes.bluetooth.a2dp = a2dp_stream_init(&a2dp_config);


    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
	rsp_cfg.src_rate = 44100;
	rsp_cfg.src_ch = 2;
	rsp_cfg.dest_rate = OUTPUT_SAMPLERATE;
	rsp_cfg.dest_ch = OUTPUT_CHANNELS;
	rsp_cfg.task_prio = 23;
	rsp_cfg.out_rb_size = 16*1024;

	pipes.bluetooth.filter = rsp_filter_init(&rsp_cfg);


	raw_stream_cfg_t raw_cfg = RAW_STREAM_CFG_DEFAULT();
    raw_cfg.type = AUDIO_STREAM_WRITER;
    pipes.bluetooth.raw = raw_stream_init(&raw_cfg);

    audio_pipeline_register(pipes.bluetooth.pipe, pipes.bluetooth.a2dp, "bt");
    audio_pipeline_register(pipes.bluetooth.pipe, pipes.bluetooth.filter, "filter");
    audio_pipeline_register(pipes.bluetooth.pipe, pipes.bluetooth.raw, "raw");

    const char *link[3] = {"bt", "filter", "raw"};
    audio_pipeline_link(pipes.bluetooth.pipe, &link[0], 3);

    downmix_set_input_rb(pipes.output.mix, audio_element_get_input_ringbuf(pipes.bluetooth.raw), SOURCE_BT_INDEX);
    downmix_set_source_stream_info(pipes.output.mix, OUTPUT_SAMPLERATE, OUTPUT_CHANNELS, SOURCE_BT_INDEX);
    audio_pipeline_set_listener(pipes.bluetooth.pipe, pipes.events);

    audio_pipeline_run(pipes.bluetooth.pipe);
}

void pipe_bluetooth_event(audio_event_iface_msg_t * msg)
{
	if (msg->cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO)
	{
		audio_element_info_t music_info = {0};
		audio_element_getinfo(pipes.bluetooth.a2dp, &music_info);

		INFO("Receive music info from Bluetooth, sample_rates=%d, bits=%d, ch=%d", music_info.sample_rates, music_info.bits, music_info.channels);

		audio_element_setinfo(pipes.bluetooth.filter, &music_info);
	}
}
