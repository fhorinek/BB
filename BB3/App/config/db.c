/*
 * db.c
 *
 *  Created on: 30. 3. 2021
 *      Author: horinek
 */
#include "db.h"

#define DB_SEPARATOR            '\t'
#define DB_OLD_SEPARATOR            '='
#define DB_LINE_LEN             128
#define DB_WORK_BUFFER_SIZE     2048

int32_t db_locate(int32_t fp, char * key, char * buff, uint16_t buffer_size)
{
    uint32_t pos = 0;
    red_lseek(fp, 0, RED_SEEK_SET);

    while (red_gets(buff, buffer_size, fp) != NULL)
    {
        if (strstr(buff, key) == buff
                && (buff[strlen(key)] == DB_SEPARATOR || buff[strlen(key)] == DB_OLD_SEPARATOR))
        {
            return pos;
        }

        pos = red_lseek(fp, 0, RED_SEEK_CUR);
    }

    return -1;
}

bool db_exists(char * path, char * key)
{
    char buff[DB_LINE_LEN];
    int32_t f = red_open(path, RED_O_RDONLY);

    bool ret = false;

    if (f > 0)
    {
        ret = db_locate(f, key, buff, sizeof(buff)) >= 0;
        red_close(f);
    }

    return ret;
}

bool db_query(char * path, char * key, char * value, uint16_t value_len)
{
    char buff[DB_LINE_LEN];

    int32_t pos = -1;


    int32_t f = red_open(path, RED_O_RDONLY);
    if (f > 0)
    {
        pos = db_locate(f, key, buff, sizeof(buff));
        red_close(f);
    }
    else
    {
        ERR("Could not open file '%s', res = %d", path, f);
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

bool db_query_def(char * path, char * key, char * value, uint16_t value_len, char * def)
{
    bool found = db_query(path, key, value, value_len);
    if (!found)
        value = def;
    return found;
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

bool db_query_int_def(char * path, char * key, int16_t * value, int16_t def)
{
    bool found = db_query_int(path, key, value);
    if (!found)
        *value = def;
    return found;
}

void db_remove_line(int32_t fp, char * path, uint32_t start_pos, uint16_t lenght)
{
    char * buff;
    char path_new[PATH_LEN];
    int32_t br, bw;

    red_lseek(fp, 0, RED_SEEK_SET);
    get_tmp_filename(path_new);

    int32_t new = red_open(path_new, RED_O_WRONLY | RED_O_CREAT);
    ASSERT(new > 0);

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
            br = red_read(fp, buff, to_read);
            if (br == 0) //EOF
                break;

            bw = red_write(new, buff, br);

            ASSERT(bw == br);

            pos += to_read;
        }

        if (pos == start_pos)
        {
            pos += lenght;
            start_pos = INT16_MAX;
            red_lseek(fp, pos, RED_SEEK_SET);
        }
    }

    free(buff);

    red_close(fp);
    red_close(new);
    red_unlink(path);
    red_rename(path_new, path);
}

void db_delete(char * path, char * key)
{
    char buff[DB_LINE_LEN];
    int32_t f = red_open(path, RED_O_RDONLY);

    if (f > 0)
    {
        int32_t pos = db_locate(f, key, buff, sizeof(buff));

        if (pos >= 0)
        {
            //remove line close the file for us
            db_remove_line(f, path, pos, strlen(buff));
        }
        else
        {
            red_close(f);
        }
    }
}

void db_insert(char * path, char * key, char * value)
{
    char buff[DB_LINE_LEN];

    int32_t f = red_open(path, RED_O_RDWR | RED_O_CREAT);

    if (f > 0)
    {
        int32_t pos = db_locate(f, key, buff, sizeof(buff));

        if (pos >= 0)
        {
            //remove '\n'
            buff[strlen(buff) - 1] = 0;

            if (strcmp(buff + strlen(key) + 1, value) == 0)
            {
                //value is the same (nothing to update)
                red_close(f);
                return;
            }
            else
            {
                //value differs, remove old record
                //remove line also close the file
                db_remove_line(f, path, pos, strlen(buff) + 1);
                f = red_open(path, RED_O_WRONLY | RED_O_APPEND);
            }
        }

        //move to end
        red_lseek(f, 0, RED_SEEK_END);
        snprintf(buff, DB_LINE_LEN, "%s%c%s\n", key, DB_SEPARATOR, value);
        red_write(f, buff, strlen(buff));
        red_close(f);
    }
}

void db_insert_int(char * path, char * key, int16_t value)
{
    char buff[8];
    snprintf(buff, sizeof(buff), "%d", value);
    db_insert(path, key, buff);
}

void db_dump(char * path)
{
    char buff[DB_LINE_LEN];

    INFO("Dumping file: %s", path);

    int32_t f = f_open(&f, path, RED_O_RDONLY);

    if (f > 0)
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
