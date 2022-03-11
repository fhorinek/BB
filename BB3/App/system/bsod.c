/*
 * bsod.c
 *
 *  Created on: 7. 6. 2021
 *      Author: horinek
 */

#include "drivers/tft/tft.h"
#include "drivers/power/led.h"
#include "drivers/nvm.h"
#include "lib/mcufont/mcufont.h"
#include "drivers/rev.h"
#include "lib/miniz/miniz.h"

const struct mf_font_s * bsod_font;

#define GFX_NONE	0
#define GFX_RED		1
#define GFX_GREEN	2
#define GFX_WHITE	3
#define GFX_BLACK   4
#define GFX_INVERT  5

uint8_t gfx_color = GFX_BLACK;
static uint16_t gfx_last_x = 0;

static void pixel_callback(int16_t x, int16_t y, uint8_t count, uint8_t alpha, void *state)
{
    uint32_t pos;
    uint16_t value;

    if (y < 0 || y >= TFT_HEIGHT) return;
    if (x < 0 || x + count >= TFT_WIDTH) return;

    while (count--)
    {
        pos = TFT_WIDTH * y + x;
        value = tft_buffer[pos];

        if (gfx_color == GFX_INVERT)
        {
            value = ~value;
        }
        else
        {
            //565 to 888
            uint8_t r = (0b1111100000000000 & value) >> 8;
            uint8_t g = (0b0000011111100000 & value) >> 3;
            uint8_t b = (0b0000000000011111 & value) << 3;

            if (gfx_color != GFX_RED)
                r = r > alpha ? r - alpha : 0;

            if (gfx_color != GFX_GREEN)
                g = g > alpha ? g - alpha : 0;

            b = b > alpha ? b - alpha : 0;

            //convert 24bit to 16bit (565)
            r >>= 3;
            g >>= 2;
            b >>= 3;

            value = (r << 11) | (g << 5) | b;
        }
 		tft_buffer[pos] = value;

        x++;
    }
    gfx_last_x = x;
}

static uint8_t character_callback(int16_t x, int16_t y, mf_char character, void *state)
{
    return mf_render_character(bsod_font, x, y, character, pixel_callback, state);
}

void bsod_draw_text(uint16_t x, uint16_t y, char * text, enum mf_align_t align)
{
	mf_render_aligned(bsod_font, x, y, align, text, strlen(text), character_callback, NULL);
}

void bsod_init()
{
	bsod_font = mf_find_font("roboto");

    HAL_TIM_Base_Start_IT(disp_timer);
    HAL_TIM_PWM_Start(disp_timer, led_bclk);
	__HAL_TIM_SET_COMPARE(disp_timer, led_bclk, 10);

	tft_init_bsod();
	tft_color_fill(0xFFFF);
}

#define LINE_SIZE   24
#define LEFT_PAD    8

void bsod_end()
{
    bsod_draw_text(TFT_WIDTH / 2, TFT_HEIGHT - LINE_SIZE, "Reset", MF_ALIGN_CENTER);

	tft_refresh_buffer(0, 0, 239, 399, tft_buffer);

	uint32_t d = 0;
	while(1)
	{
		if (HAL_GPIO_ReadPin(BT3) == LOW)
		{
			d++;
			if (d > 500000)
			{
				NVIC_SystemReset();
			}
		}
		else
		{
			d = 0;
		}
	}
}

void bsod_show(context_frame_t * frame)
{
	bsod_init();

	uint16_t line = 0;

	char buff[64];
	uint32_t CFSR = SCB->CFSR;

    if (CFSR & SCB_CFSR_USGFAULTSR_Msk)
    	bsod_draw_text(TFT_WIDTH / 2, (line++) * LINE_SIZE, "** Usage Fault **", MF_ALIGN_CENTER);
    if (CFSR & SCB_CFSR_DIVBYZERO_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "DIVBYZERO", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_UNALIGNED_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "UNALIGNED", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_NOCP_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "NOCP", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_INVPC_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "INVPC", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_INVSTATE_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "INVSTATE", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_UNDEFINSTR_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "UNDEFINSTR", MF_ALIGN_LEFT);

    if (CFSR & SCB_CFSR_BUSFAULTSR_Msk)
    	bsod_draw_text(TFT_WIDTH / 2, (line++) * LINE_SIZE, "** Bus Fault **", MF_ALIGN_CENTER);
    if (CFSR & SCB_CFSR_BFARVALID_Msk)
    {
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "BFARVALID", MF_ALIGN_LEFT);
    	snprintf(buff, sizeof(buff), "BFAR: %08lX", SCB->BFAR);
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);
    }
    if (CFSR & SCB_CFSR_LSPERR_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "LSPERR", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_STKERR_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "STKERR", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_UNSTKERR_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "UNSTKERR", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_IMPRECISERR_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "IMPRECISERR", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_PRECISERR_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "PRECISERR", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_IBUSERR_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "IBUSERR", MF_ALIGN_LEFT);

    if (CFSR & SCB_CFSR_MEMFAULTSR_Msk)
    	bsod_draw_text(TFT_WIDTH / 2, (line++) * LINE_SIZE, "** Mem Fault **", MF_ALIGN_CENTER);
    if (CFSR & SCB_CFSR_MMARVALID_Msk)
    {
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "MMARVALID", MF_ALIGN_LEFT);
    	snprintf(buff, sizeof(buff), "MMFAR: %08lX", SCB->BFAR);
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);
    }
    if (CFSR & SCB_CFSR_IACCVIOL_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "IACCVIOL", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_DACCVIOL_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "DACCVIOL", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_MUNSTKERR_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "MUNSTKERR", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_MSTKERR_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "MSTKERR", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_MLSPERR_Msk)
    	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, "MLSPERR", MF_ALIGN_LEFT);

    line++;

    snprintf(buff, sizeof(buff), "PC: 0x%08lX", frame->pc);
    bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);
    snprintf(buff, sizeof(buff), "R1: 0x%08lX", frame->r1);
    bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);
    snprintf(buff, sizeof(buff), "R2: 0x%08lX", frame->r2);
    bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);
    snprintf(buff, sizeof(buff), "R3: 0x%08lX", frame->r3);
    bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);
    snprintf(buff, sizeof(buff), "R12: 0x%08lX", frame->r12);
    bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);
    snprintf(buff, sizeof(buff), "LR: 0x%08lX", frame->lr);
    bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);
    snprintf(buff, sizeof(buff), "XPSR: 0x%08lX", frame->xpsr);
    bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);

    line++;

	snprintf(buff, sizeof(buff), "CFSR: %08lX", CFSR);
	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);

    char tmp[10];
    rev_get_sw_string(tmp);
	snprintf(buff, sizeof(buff), "FW: %s", tmp);
	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);

	snprintf(buff, sizeof(buff), "HW: %02X", rev_get_hw());
	bsod_draw_text(LEFT_PAD, (line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);

    bsod_end();
}

static bool bsod_line_callback(const char *line, uint16_t count, void *state)
{
	int16_t * y = (int16_t *)state;

    mf_render_aligned(bsod_font, LEFT_PAD, *y, MF_ALIGN_LEFT, line, count, character_callback, NULL);
    *y += LINE_SIZE;

    return true;
}

void bsod_msg(const char *format, ...)
{
	bsod_init();

	vTaskSuspendAll();

	va_list arp;

	char msg[256];
    va_start(arp, format);
    vsnprintf(msg, sizeof(msg), format, arp);
    va_end(arp);

    int16_t y = LEFT_PAD;

    bsod_draw_text(TFT_WIDTH / 2, y, "ERROR", MF_ALIGN_CENTER);
    y += LINE_SIZE;

	mf_wordwrap(bsod_font, TFT_WIDTH - 2 * LEFT_PAD, msg, bsod_line_callback, &y);

	FAULT("%s", msg);

	bsod_end();
}

static uint16_t bsod_line = 0;

void bsod_crash_reason(const Crash_Object * info)
{
    char buff[64];
    char tmp[10];

    rev_get_sw_string(tmp);
    snprintf(buff, sizeof(buff), "FW: %s", tmp);
    bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);

//    snprintf(buff, sizeof(buff), "HW: %02X", rev_get_hw());
//    bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);

    bsod_line++;

    uint32_t CFSR = SCB->CFSR;

    if (CFSR & SCB_CFSR_USGFAULTSR_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "Usage Fault", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_DIVBYZERO_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "DIVBYZERO", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_UNALIGNED_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "UNALIGNED", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_NOCP_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "NOCP", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_INVPC_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "INVPC", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_INVSTATE_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "INVSTATE", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_UNDEFINSTR_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "UNDEFINSTR", MF_ALIGN_LEFT);

    if (CFSR & SCB_CFSR_BUSFAULTSR_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "Bus Fault", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_BFARVALID_Msk)
    {
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "BFARVALID", MF_ALIGN_LEFT);
        snprintf(buff, sizeof(buff), "BFAR: %08lX", SCB->BFAR);
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);
    }
    if (CFSR & SCB_CFSR_LSPERR_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "LSPERR", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_STKERR_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "STKERR", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_UNSTKERR_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "UNSTKERR", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_IMPRECISERR_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "IMPRECISERR", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_PRECISERR_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "PRECISERR", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_IBUSERR_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "IBUSERR", MF_ALIGN_LEFT);

    if (CFSR & SCB_CFSR_MEMFAULTSR_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "Mem Fault", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_MMARVALID_Msk)
    {
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "MMARVALID", MF_ALIGN_LEFT);
        snprintf(buff, sizeof(buff), "MMFAR: %08lX", SCB->BFAR);
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);
    }
    if (CFSR & SCB_CFSR_IACCVIOL_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "IACCVIOL", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_DACCVIOL_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "DACCVIOL", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_MUNSTKERR_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "MUNSTKERR", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_MSTKERR_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "MSTKERR", MF_ALIGN_LEFT);
    if (CFSR & SCB_CFSR_MLSPERR_Msk)
        bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, "MLSPERR", MF_ALIGN_LEFT);

    bsod_line++;

    snprintf(buff, sizeof(buff), "PC: 0x%08lX", info->pSP->pc);
    bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);
    snprintf(buff, sizeof(buff), "LR: 0x%08lX", info->pSP->lr);
    bsod_draw_text(LEFT_PAD, (bsod_line++) * LINE_SIZE, buff, MF_ALIGN_LEFT);

    bsod_line++;


}

void bsod_redraw()
{
    tft_refresh_buffer(0, 0, 239, 399, tft_buffer);
}



void bsod_crash_start(const Crash_Object * info)
{
    bsod_init();

    bsod_line = 0;

    bsod_draw_text(TFT_WIDTH / 2, (bsod_line++) * LINE_SIZE, "** Strato Crashed **", MF_ALIGN_CENTER);

    bsod_crash_reason(info);

    if (config_get_bool(&config.debug.crash_dump))
    {
        bsod_draw_text(TFT_WIDTH / 2, (bsod_line++) * LINE_SIZE, "Creating report", MF_ALIGN_CENTER);
        bsod_draw_text(LEFT_PAD, (bsod_line) * LINE_SIZE, "Dumping RAM...", MF_ALIGN_LEFT);

        bsod_redraw();
    }
    else
    {
        bsod_draw_text(TFT_WIDTH / 2, (bsod_line++) * LINE_SIZE, "Crash report disabled", MF_ALIGN_CENTER);

        bsod_end();
    }
}

void bsod_list_readdir(FIL * file, char * path, uint8_t level)
{
    DIR dir;
    static FILINFO fno;
    static char buff[PATH_LEN];
    UINT bw;

    for (uint8_t i = 0; i < level; i++)
        buff[i] = '\t';

    if (level > 0)
    {
        snprintf(buff + level, sizeof(buff) - level, "[%s]\n", path);
    }
    else
    {
        snprintf(buff, sizeof(buff), "[Strato SD card]\n");
    }
    f_write(file, buff, strlen(buff), &bw);


    FRESULT res = f_opendir(&dir, path);
    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno);

            if (res != FR_OK || fno.fname[0] == 0)
                break;

            if (fno.fattrib & AM_DIR)
            {
                uint16_t i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                bsod_list_readdir(file, path, level + 1);
                path[i] = 0;
            }
            else
            {
                snprintf(buff + level, sizeof(buff) - level, "\t%-30s %lu\n", fno.fname, fno.fsize);
                f_write(file, buff, strlen(buff), &bw);
            }
        }
        f_closedir(&dir);
    }
}

void bsod_list_flies()
{
    bsod_draw_text(LEFT_PAD, (bsod_line) * LINE_SIZE, "Listing files...", MF_ALIGN_LEFT);

    FIL file;
    FRESULT res = f_open(&file, PATH_CRASH_FILES, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK)
    {
        FAULT("f_open, res = %u", res);
        bsod_draw_text(gfx_last_x, (bsod_line++) * LINE_SIZE, "fail", MF_ALIGN_LEFT);
    }

    bsod_redraw();

    if (res != FR_OK)
        return;

    char path[PATH_LEN] = "";
    bsod_list_readdir(&file, path, 0);
    f_close(&file);

    bsod_draw_text(gfx_last_x, (bsod_line++) * LINE_SIZE, "done", MF_ALIGN_LEFT);

    bsod_redraw();
}

void bsod_zip_path(mz_zip_archive * zip, char * path)
{
    DIR dir;
    static FILINFO fno;
    static char buff[PATH_LEN];

    FRESULT res = f_opendir(&dir, path);
    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno);

            if (res != FR_OK || fno.fname[0] == 0)
                break;

            if (fno.fattrib & AM_DIR)
            {
                uint16_t i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                bsod_zip_path(zip, path);
                path[i] = 0;
            }
            else
            {
                snprintf(buff, sizeof(buff), "%s/%s", path, fno.fname);
                FAULT("Compressing %s", buff);

                //do not include wifi passwords!
                if (strcmp(buff, PATH_NETWORK_DB) != 0)
                    mz_zip_writer_add_file(zip, buff, buff, "", 0, 0);
            }
        }
        f_closedir(&dir);
    }
}

void bsod_bundle_report()
{
    bsod_draw_text(LEFT_PAD, (bsod_line) * LINE_SIZE, "Compressing...", MF_ALIGN_LEFT);
    bsod_redraw();

    char path[PATH_LEN];
    mz_zip_archive * zip = malloc(sizeof(mz_zip_archive));
    mz_zip_zero_struct(zip);

    snprintf(path, sizeof(path), "crash_report.zip");
    uint16_t cnt = 0;
    while (file_exists(path))
    {
        cnt++;
        snprintf(path, sizeof(path), "crash_report_%u.zip", cnt);
    }


    bool res = mz_zip_writer_init_file(zip, path, 0);

    if (res)
    {
        strcpy(path, PATH_CONFIG_DIR);
        bsod_zip_path(zip, path);

        f_rename(DEBUG_FILE, PATH_CRASH_LOG);
        strcpy(path, PATH_CRASH_DIR);
        bsod_zip_path(zip, path);

        mz_zip_writer_finalize_archive(zip);
        free(zip);

        remove_dir(PATH_CRASH_DIR);

        bsod_draw_text(gfx_last_x, (bsod_line++) * LINE_SIZE, "done", MF_ALIGN_LEFT);
    }
    else
    {
        bsod_draw_text(gfx_last_x, (bsod_line++) * LINE_SIZE, "fail", MF_ALIGN_LEFT);
    }

    bsod_redraw();
}

void bsod_crash_dumped()
{
    bsod_draw_text(gfx_last_x, (bsod_line++) * LINE_SIZE, "done", MF_ALIGN_LEFT);

    bsod_list_flies();
    bsod_bundle_report();

    bsod_end();
}

void bsod_crash_fail()
{
    bsod_draw_text(gfx_last_x, (bsod_line++) * LINE_SIZE, "fail", MF_ALIGN_LEFT);

    bsod_end();
}
