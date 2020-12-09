/*
 * i2s.c
 *
 *  Created on: 4. 12. 2020
 *      Author: horinek
 */

#include "i2s.h"

#include "driver/i2s.h"

#define SAMPLE_RATE     (48000)
#define SAMPLE_PER_CYCLE (SAMPLE_RATE/WAVE_FREQ_HZ)

#define I2S_NUM         I2S_NUM_0
#define WAVE_FREQ_HZ    (2000)
#define PI              (3.14159265)
#define I2S_BCK_IO      (GPIO_NUM_16) //BCLK
#define I2S_WS_IO       (GPIO_NUM_21) //LRCLK
#define I2S_DO_IO       (GPIO_NUM_4) //SDIN
#define I2S_DI_IO       (-1)

static void setup_triangle_sine_waves()
{
//    uint32_t * samples_data = malloc(4 * 2 * SAMPLE_PER_CYCLE);
    uint32_t * samples_data = malloc(4 * SAMPLE_PER_CYCLE);
    uint16_t i;
    double sin_float;
    size_t i2s_bytes_write = 0;

    for(i = 0; i < SAMPLE_PER_CYCLE; i++) {
        sin_float = sin(i * 2 * PI / SAMPLE_PER_CYCLE);
        sin_float *= 0x7FFFFF;
//        sin_float *= 0x7FFF;

//        printf("%0.1f\n", sin_float);

        samples_data[i + 0] = (int)sin_float << 8;
       // samples_data[i*2 + 1] = (int)sin_float << 8;
    }

    i2s_write(I2S_NUM, samples_data, 4 * SAMPLE_PER_CYCLE, &i2s_bytes_write, 100);

    free(samples_data);
}

void i2s_init()
{
    i2s_config_t i2s_config = {
           .mode = I2S_MODE_MASTER | I2S_MODE_TX,                                  // Only TX
           .sample_rate = SAMPLE_RATE,
           .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
           .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,                                   //2-channels
           .communication_format = I2S_COMM_FORMAT_STAND_I2S,
           .dma_buf_count = 6,
           .dma_buf_len = 60,
 //          .use_apll = true,
           .tx_desc_auto_clear = true,
           .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,                                //Interrupt level 1
       };
       i2s_pin_config_t pin_config = {
           .bck_io_num = I2S_BCK_IO,
           .ws_io_num = I2S_WS_IO,
           .data_out_num = I2S_DO_IO,
           .data_in_num = I2S_DI_IO                                               //Not used
       };
       i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
       i2s_set_pin(I2S_NUM, &pin_config);

       //setup_triangle_sine_waves();

}
