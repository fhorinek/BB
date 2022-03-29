/*
 * db.c
 *
 *  Created on: 30. 3. 2021
 *      Author: horinek
 */
#include "db.h"

#define DB_SEPARATOR            '\t'
#define DB_LINE_LEN             128
#define DB_WORK_BUFFER_SIZE     2048

int32_t db_locate(FIL * fp, char * key, char * buff, uint16_t buffer_size)
{
    uint32_t pos = 0;
    f_lseek(fp, 0);

    while (f_gets(buff, buffer_size, fp) != NULL)
    {
        if (strstr(buff, key) == buff && buff[strlen(key)] == DB_SEPARATOR)
        {
            return pos;
        }

        pos = f_tell(fp);
    }

    return -1;
}

bool db_exists(char * path, char * key)
{
    char buff[DB_LINE_LEN];
    FIL f;
    bool ret = false;

    if (f_open(&f, path, FA_READ) == FR_OK)
    {
        ret = db_locate(&f, key, buff, sizeof(buff)) >= 0;
        f_close(&f);
    }

    return ret;
}

bool db_query(char * path, char * key, char * value, uint16_t value_len)
{
    char buff[DB_LINE_LEN];
    FIL f;
    int32_t pos = -1;


    FRESULT res = f_open(&f, path, FA_READ);
    if (res == FR_OK)
    {
        pos = db_locate(&f, key, buff, sizeof(buff));
        f_close(&f);
    }
    else
    {
        ERR("Could not open file '%s', res = %u", path, res);
    }

    //line starting with key found!
    if (pos >= 0)
    {
        //remove /n
        buff[strlen(buff) - 1] = 0;
        strncpy(value, buff + strlen(key) + 1, value_len);
        return true;
    }
    else
    {
        WARN("Key '%s' not found in '%s'", key, path);
    }

    return false;
}

bool db_query_int(char * path, char * key, int16_t * value)
{
    char buff[8];

    bool ret = db_query(path, key, buff, sizeof(buff));
    if (ret)
    {
        *value = atoi(buff);
    }
    return ret;
}


void db_remove_line(FIL * fp, char * path, uint32_t start_pos, uint16_t lenght)
{
    FIL new;
    char * buff;
    char path_new[64];
    UINT br, bw;

    f_lseek(fp, 0);
    get_tmp_filename(path_new);

    ASSERT(f_open(&new, path_new, FA_WRITE | FA_CREATE_NEW) == FR_OK);

    buff = (char *) malloc(DB_WORK_BUFFER_SIZE);
    ASSERT(buff != NULL);

    uint32_t pos = 0;
    while (1)
    {
        int16_t to_read = DB_WORK_BUFFER_SIZE;
        if (pos + DB_WORK_BUFFER_SIZE > start_pos)
            to_read = start_pos - pos;

        if (to_read > 0)
        {
            f_read(fp, buff, to_read, &br);
            if (br == 0) //EOF
                break;

            f_write(&new, buff, br, &bw);

            pos += to_read;
        }

        if (pos == start_pos)
        {
            pos += lenght;
            start_pos = INT16_MAX;
            f_lseek(fp, pos);
        }
    }

    free(buff);

    f_close(fp);
    f_close(&new);
    f_unlink(path);
    f_rename(path_new, path);
}

void db_delete(char * path, char * key)
{
    char buff[DB_LINE_LEN];
    FIL f;

    if(f_open(&f, path, FA_READ) == FR_OK)
    {
        int32_t pos = db_locate(&f, key, buff, sizeof(buff));

        if (pos >= 0)
        {
            //remove line close the file for us
            db_remove_line(&f, path, pos, strlen(buff));
        }
        else
        {
            f_close(&f);
        }
    }
}

void db_insert(char * path, char * key, char * value)
{
    FIL f;
    char buff[DB_LINE_LEN];

    if (f_open(&f, path, FA_OPEN_ALWAYS | FA_WRITE | FA_READ) == FR_OK)
    {
        int32_t pos = db_locate(&f, key, buff, sizeof(buff));

        if (pos >= 0)
        {
            //remove '\n'
            buff[strlen(buff) - 1] = 0;

            if (strcmp(buff + strlen(key) + 1, value) == 0)
            {
                //value is the same (nothing to update)
                return;
            }
            else
            {
                //value differs, remove old record
                db_remove_line(&f, path, pos, strlen(buff) + 1);
                f_open(&f, path, FA_WRITE | FA_OPEN_APPEND);
            }
        }

        UINT bw;

        //move to end
        f_lseek(&f, f_size(&f));
        snprintf(buff, DB_LINE_LEN, "%s%c%s\n", key, DB_SEPARATOR, value);
        f_write(&f, buff, strlen(buff), &bw);
        f_close(&f);
    }
}

void db_dump(char * path)
{
    FIL f;
    char buff[DB_LINE_LEN];

    INFO("Dumping file: %s", path);

    if (f_open(&f, path, FA_READ) == FR_OK)
    {
        uint16_t i = 0;
        while (f_gets(buff, sizeof(buff), &f) != NULL)
        {
            buff[strlen(buff) - 1] = 0;
            INFO("\t%03u: %s", i, buff);
            i++;
        }

        INFO("End of file");
        f_close(&f);
    }
    else
    {
        INFO("Unable to open");
    }
}
