idf_component_register(SRCS "audio_board.c" "board.c" "board_pins_config.c"
					REQUIRES "esp_peripherals" "audio_sal" "audio_hal" "esp_dispatcher" "display_service"
                    INCLUDE_DIRS "include")
