/*
 * crash.c
 *
 *  Created on: Mar 9, 2022
 *      Author: horinek
 */

#include "crash.h"
#include "bsod.h"
#include "rtc.h"

#include "drivers/rtc.h"
#include "drivers/rev.h"
#include "drivers/sd.h"

static int32_t file;
static bool file_open = false;

#define WRITE(A)    red_write(file, A, strlen(A))

void info_get_firmware_version(char * buff, size_t buff_size)
{
    char tmp[20];
    rev_get_sw_string(tmp);
    snprintf(buff, buff_size, "firmware_version: '%s'\n", tmp);
}

// Time according to ISO8601 (e.g 2022-03-13T18:05:42Z)
void info_get_timestamp(char * buff, size_t buff_size)
{
    uint16_t year;
    uint8_t month, weekday, day, hour, minute, second;

    rtc_get_date(&day, &weekday, &month, &year);
    rtc_get_time(&hour, &minute, &second);

    snprintf(buff, buff_size, "timestamp: '%04u-%02u-%02uT%02u:%02u:%02uZ'\n", year, month, day, hour, minute, second);
}

// Records relevant meta data in a yaml format
void crash_store_info(const Crash_Object * info)
{
    file = red_open(PATH_CRASH_INFO, RED_O_WRONLY | RED_O_CREAT | RED_O_TRUNC);
    if (file > 0)
    {
        char buff[64];

        snprintf(buff, sizeof(buff), "serial_number: '%08lX'\n", rev_get_short_id());
        WRITE(buff);

        info_get_timestamp(buff, sizeof(buff));
        WRITE(buff);

        info_get_firmware_version(buff, sizeof(buff));
        WRITE(buff);

        snprintf(buff, sizeof(buff), "hardware_revision: '%02X'\n", rev_get_hw());
        WRITE(buff);

        if (bsod_msg_ptr != NULL)
        {
            // Replace >'< with >"< (>'< is used as delimiter in the yaml)
            char * result;
            while((result = strchr(bsod_msg_ptr, '\'')) != NULL)
                *result = '"';

            WRITE("message: '");
            WRITE(bsod_msg_ptr);
            WRITE("'\n");
        }

        uint32_t CFSR = SCB->CFSR;

        WRITE("fault:\n");
        snprintf(buff, sizeof(buff), "  status: '%08lX'\n", CFSR);
        WRITE(buff);

        WRITE("  reasons:\n");
        if (CFSR & SCB_CFSR_USGFAULTSR_Msk)
            WRITE("    - 'USAGE_FAULT'\n");
        if (CFSR & SCB_CFSR_DIVBYZERO_Msk)
            WRITE("    - 'DIVBYZERO'\n");
        if (CFSR & SCB_CFSR_UNALIGNED_Msk)
            WRITE("    - 'UNALIGNED'\n");
        if (CFSR & SCB_CFSR_NOCP_Msk)
            WRITE("    - 'NOCP'\n");
        if (CFSR & SCB_CFSR_INVPC_Msk)
            WRITE("    - 'INVPC'\n");
        if (CFSR & SCB_CFSR_INVSTATE_Msk)
            WRITE("    - 'INVSTATE'\n");
        if (CFSR & SCB_CFSR_UNDEFINSTR_Msk)
            WRITE("    - 'UNDEFINSTR'\n");

        if (CFSR & SCB_CFSR_BUSFAULTSR_Msk)
            WRITE("    - 'BUS_FAULT'\n");
        if (CFSR & SCB_CFSR_BFARVALID_Msk)
        	WRITE("    - 'BFARVALID'\n");
        if (CFSR & SCB_CFSR_LSPERR_Msk)
            WRITE("    - 'LSPERR'\n");
        if (CFSR & SCB_CFSR_STKERR_Msk)
            WRITE("    - 'STKERR'\n");
        if (CFSR & SCB_CFSR_UNSTKERR_Msk)
            WRITE("    - 'UNSTKERR'\n");
        if (CFSR & SCB_CFSR_IMPRECISERR_Msk)
            WRITE("    - 'IMPRECISERR'\n");
        if (CFSR & SCB_CFSR_PRECISERR_Msk)
            WRITE("    - 'PRECISERR'\n");
        if (CFSR & SCB_CFSR_IBUSERR_Msk)
            WRITE("    - 'IBUSERR'\n");

        if (CFSR & SCB_CFSR_MEMFAULTSR_Msk)
            WRITE("    - 'MEM_FAULT'\n");
        if (CFSR & SCB_CFSR_MMARVALID_Msk)
        	WRITE("    - 'MMARVALID'\n");
        if (CFSR & SCB_CFSR_IACCVIOL_Msk)
            WRITE("    - 'IACCVIOL'\n");
        if (CFSR & SCB_CFSR_DACCVIOL_Msk)
            WRITE("    - 'DACCVIOL'\n");
        if (CFSR & SCB_CFSR_MUNSTKERR_Msk)
            WRITE("    - 'MUNSTKERR'\n");
        if (CFSR & SCB_CFSR_MSTKERR_Msk)
            WRITE("    - 'MSTKERR'\n");
        if (CFSR & SCB_CFSR_MLSPERR_Msk)
            WRITE("    - 'MLSPERR'\n");

        if (CFSR & SCB_CFSR_BFARVALID_Msk)
        {
            snprintf(buff, sizeof(buff), "  bus_status: '%08lX'\n", SCB->BFAR);
            WRITE(buff);
        }

        if (CFSR & SCB_CFSR_MMARVALID_Msk)
        {
            snprintf(buff, sizeof(buff), "  memory_management_status: '%08lX'\n", SCB->MMFAR);
            WRITE(buff);
        }

        WRITE("mcu_registers:\n");
        snprintf(buff, sizeof(buff), "  R0:  '%08lX'\n", info->pSP->r0);
        WRITE(buff);
        snprintf(buff, sizeof(buff), "  R1:  '%08lX'\n", info->pSP->r1);
        WRITE(buff);
        snprintf(buff, sizeof(buff), "  R2:  '%08lX'\n", info->pSP->r2);
        WRITE(buff);
        snprintf(buff, sizeof(buff), "  R3:  '%08lX'\n", info->pSP->r3);
        WRITE(buff);
        snprintf(buff, sizeof(buff), "  R12: '%08lX'\n", info->pSP->r12);
        WRITE(buff);
        snprintf(buff, sizeof(buff), "  LR:  '%08lX'\n", info->pSP->lr);
        WRITE(buff);
        snprintf(buff, sizeof(buff), "  PC:  '%08lX'\n", info->pSP->pc);
        WRITE(buff);
        snprintf(buff, sizeof(buff), "  PSR: '%08lX'\n", info->pSP->psr);
        WRITE(buff);

        red_close(file);
    }
}

void CrashCatcher_DumpStart(const Crash_Object * info)
{
    bsod_crash_start(info);

    sd_init_failsafe();

    red_mkdir(PATH_CRASH_DIR);

    crash_store_info(info);

    file = red_open(PATH_CRASH_DUMP, RED_O_WRONLY | RED_O_CREAT | RED_O_TRUNC);
    if (file > 0)
    {
        file_open = true;
        FAULT("Crash file opened!");
    }
    else
    {
        FAULT("Unable to create crash dump, res = %d", file);
    }
}

static uint32_t total_size = 0;

void CrashCatcher_DumpMemory(const void* pvMemory, CrashCatcherElementSizes elementSize, size_t elementCount)
{
    if (!file_open)
        return;

    int32_t len = elementCount * elementSize;
    total_size += len;

    int32_t res;
    static uint16_t dbg_cnt = 0;

    dbg_cnt++;
    res = red_write(file, pvMemory, len);
    if (res != len)
        FAULT("f_write, res = %d (%u)", res, dbg_cnt);

    res = red_sync();
    if (res != 0)
        FAULT("f_sync, res = %d (%u)", res, dbg_cnt);
}

CrashCatcherReturnCodes CrashCatcher_DumpEnd(void)
{
    if (!file_open)
        bsod_crash_fail();

    int32_t res = red_close(file);
    if (res != 0)
        FAULT("f_close, res = %u", res);

    FAULT("Closed, size: %u", total_size);

    bsod_crash_dumped();
    return CRASH_CATCHER_TRY_AGAIN;
}

const CrashCatcherMemoryRegion * CrashCatcher_GetMemoryRegions(void)
{
    static const CrashCatcherMemoryRegion regions[] =
    {
        //DTCM
        {0x20000000, 0x20020000, CRASH_CATCHER_BYTE},
        //RAM
        {0x24000000, 0x24100000, CRASH_CATCHER_BYTE},
        //PSRAM
        // Excluded because it adds ~8MB to the dump file
        //{0x90000000, 0x90800000, CRASH_CATCHER_BYTE},
        //end of records
        {0xFFFFFFFF, 0xFFFFFFFF, CRASH_CATCHER_BYTE}
    };

    return regions;
}
